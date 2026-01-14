/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <gui_features.h>
#if ENABLE(WEBP)
#include <core/context.h>
#include <cairomm/surface.h>
#include <image-decoders/imagedecoder.h>
#include <webp/decode.h>
#include <webp/demux.h>
#include <fstream>
#include <cstring>
#include <vector>
#include <porting/cdlog.h>
#if ENABLE(LCMS)
#include <lcms2.h>
#endif

namespace cdroid {

struct PRIVATE {
    uint8_t* data;
    size_t size;
    struct WebPData webp_data;
    struct WebPDemuxer* demux;
};

static inline int multiply_alpha (int alpha, int color) {
    int temp = (alpha * color) + 0x80;
    return ((temp + (temp >> 8)) >> 8);
}

// Scan RIFF WebP chunks to extract an embedded ICC profile (ICCP chunk) if any.
static std::vector<uint8_t> extractICCP(const uint8_t* data, size_t size) {
    std::vector<uint8_t> empty;
    if (!data || size < 12) return empty;
    size_t pos = 12; // skip RIFF header (12 bytes)
    while (pos + 8 <= size) {
        const uint8_t* tagPtr = data + pos;
        uint32_t chunkSize = (uint32_t)tagPtr[4] | ((uint32_t)tagPtr[5] << 8) | ((uint32_t)tagPtr[6] << 16) | ((uint32_t)tagPtr[7] << 24);
        size_t dataStart = pos + 8;
        if (dataStart + chunkSize > size) break;
        if (!std::memcmp(tagPtr, "ICCP", 4) || !std::memcmp(tagPtr, "iccp", 4)) {
            return std::vector<uint8_t>(data + dataStart, data + dataStart + chunkSize);
        }
        size_t advance = 8 + chunkSize;
        if (chunkSize & 1) advance++;
        pos += advance;
    }
    return empty;
}

WEBPDecoder::WEBPDecoder(std::istream& stream): ImageDecoder(stream) {
    mPrivate = new PRIVATE();
    mPrivate->data = nullptr;
    mPrivate->size = 0;
    mPrivate->demux = nullptr;

    // Read entire stream into memory
    std::vector<char> buf((std::istreambuf_iterator<char>(mStream)), std::istreambuf_iterator<char>());
    mPrivate->size = buf.size();
    if (mPrivate->size > 0) {
        mPrivate->data = new uint8_t[mPrivate->size];
        std::memcpy(mPrivate->data, buf.data(), mPrivate->size);
        mPrivate->webp_data.bytes = mPrivate->data;
        mPrivate->webp_data.size = mPrivate->size;
    } else {
        mPrivate->data = nullptr;
    }
}

WEBPDecoder::~WEBPDecoder() {
    if (mPrivate) {
        if (mPrivate->demux) WebPDemuxDelete(mPrivate->demux);
        delete[] mPrivate->data;
        delete mPrivate;
    }
}

bool WEBPDecoder::decodeSize() {
    if (!mPrivate || !mPrivate->data || mPrivate->size == 0) return false;

    if (!WebPGetInfo(mPrivate->data, mPrivate->size, (int*)&mImageWidth, (int*)&mImageHeight)) {
        LOGE("WebP get info failed");
        return false;
    }

    // Try to determine frame count for animated WebP.
    mPrivate->demux = WebPDemux(&mPrivate->webp_data);
    if (mPrivate->demux) {
        mFrameCount = WebPDemuxGetI(mPrivate->demux, WEBP_FF_FRAME_COUNT);
        WebPDemuxDelete(mPrivate->demux);
        mPrivate->demux = nullptr;
    } else {
        mFrameCount = 1;
    }
    return true;
}

Cairo::RefPtr<Cairo::ImageSurface> WEBPDecoder::decode(float scale, void* targetProfile) {
    if ((mImageWidth == -1) || (mImageHeight == -1)) {
        if (!decodeSize()) return nullptr;
    }

    const int width = mImageWidth;
    const int height = mImageHeight;

    Cairo::RefPtr<Cairo::ImageSurface> image = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, width, height);
    uint8_t* pixels = image->get_data();
    const int stride = image->get_stride();

    const size_t out_size = static_cast<size_t>(stride) * height;

#if ENABLE(LCMS)
    cmsHTRANSFORM transform = nullptr;
    if (targetProfile != nullptr) {
        std::vector<uint8_t> icc = extractICCP(mPrivate->data, mPrivate->size);
        cmsHPROFILE src_profile = nullptr;
        if (!icc.empty()) {
            src_profile = cmsOpenProfileFromMem(icc.data(), icc.size());
        } else {
            src_profile = cmsCreate_sRGBProfile();
        }
        if (src_profile) {
            transform = cmsCreateTransform(src_profile, TYPE_RGBA_8,
                                           (cmsHPROFILE)targetProfile, TYPE_RGBA_8,
                                           cmsGetHeaderRenderingIntent(src_profile), 0);
            cmsCloseProfile(src_profile);
            if (transform) {
                // store transform in base class for cleanup
                mTransform = transform;
            }
        }
    }
#endif

    // Decode into temporary buffer in BGRA order (byte order: B,G,R,A)
    uint8_t* decoded = new uint8_t[out_size];
    if (WebPDecodeBGRAInto(mPrivate->data, mPrivate->size, decoded, out_size, stride) == 0) {
        LOGE("WebP decode failed");
        delete[] decoded;
        return nullptr;
    }

#if ENABLE(LCMS)
    if (mTransform) {
        // We have an LCMS transform: use it to convert rows from RGBA -> RGBA color space
        uint8_t* srcRow = new uint8_t[width * 4];
        uint8_t* dstRow = new uint8_t[width * 4];
        for (int y = 0; y < height; y++) {
            uint8_t* srcDecodedRow = decoded + y * stride;
            uint32_t* dst32 = reinterpret_cast<uint32_t*>(pixels + y * stride);
            // build src RGBA row from decoded BGRA
            for (int x = 0; x < width; x++) {
                srcRow[4 * x + 0] = srcDecodedRow[4 * x + 2]; // R
                srcRow[4 * x + 1] = srcDecodedRow[4 * x + 1]; // G
                srcRow[4 * x + 2] = srcDecodedRow[4 * x + 0]; // B
                srcRow[4 * x + 3] = srcDecodedRow[4 * x + 3]; // A
            }
            // color transform
            cmsDoTransform((cmsHTRANSFORM)mTransform, srcRow, dstRow, width);
            for (int x = 0; x < width; x++) {
                uint8_t a = srcRow[4 * x + 3];
                uint8_t r = dstRow[4 * x + 0];
                uint8_t g = dstRow[4 * x + 1];
                uint8_t b = dstRow[4 * x + 2];
                uint32_t p;
                if (a == 0) {
                    p = 0;
                } else {
                    if (a != 255) {
                        r = (uint8_t)multiply_alpha(a, r);
                        g = (uint8_t)multiply_alpha(a, g);
                        b = (uint8_t)multiply_alpha(a, b);
                    }
                    p = (uint32_t(a) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
                }
                dst32[x] = p;
            }
        }
        delete[] srcRow;
        delete[] dstRow;
        delete[] decoded;
        const int transparency = ImageDecoder::computeTransparency(image);
        ImageDecoder::setTransparency(image, transparency);
        return image;
    }
#endif

    // No color transform: premultiply and convert BGRA bytes to native ARGB32 (0xAARRGGBB)
    for (int y = 0; y < height; y++) {
        uint8_t* row = decoded + y * stride;
        uint32_t* dst = reinterpret_cast<uint32_t*>(pixels + y * stride);
        for (int x = 0; x < width; x++) {
            uint8_t b = row[x * 4 + 0];
            uint8_t g = row[x * 4 + 1];
            uint8_t r = row[x * 4 + 2];
            uint8_t a = row[x * 4 + 3];
            uint32_t p;
            if (a == 0) {
                p = 0;
            } else {
                if (a != 255) {
                    r = (uint8_t)multiply_alpha(a, r);
                    g = (uint8_t)multiply_alpha(a, g);
                    b = (uint8_t)multiply_alpha(a, b);
                }
                p = (uint32_t(a) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
            }
            dst[x] = p;
        }
    }

    delete[] decoded;
    const int transparency = ImageDecoder::computeTransparency(image);
    ImageDecoder::setTransparency(image, transparency);
    return image;
}

bool WEBPDecoder::isWEBP(const uint8_t* contents, uint32_t header_size) {
    if (header_size < 12) return false;
    return !std::memcmp("RIFF", contents, 4) && !std::memcmp("WEBP", contents + 8, 4);
}

} // namespace cdroid

#endif /* ENABLE(WEBP) */