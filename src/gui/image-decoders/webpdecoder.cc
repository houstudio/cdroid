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
}

WEBPDecoder::~WEBPDecoder() {
    if (mPrivate) {
        if (mPrivate->demux) WebPDemuxDelete(mPrivate->demux);
        delete[] mPrivate->data;
        delete mPrivate;
    }
}

bool WEBPDecoder::decodeSize() {
    if (!mPrivate) return false;

    // Incrementally read small chunks and call WebPGetFeatures until we can
    // determine width/height. For seekable streams we restore position; for
    // non-seekable streams we keep the prefetched bytes in mPrivate for later use.
    constexpr size_t kChunk = 8 * 1024;
    constexpr size_t kMaxProbe = 256 * 1024; // limit probing to avoid long reads
    std::vector<uint8_t> probe;
    probe.reserve(kChunk);

    std::streampos origPos = mStream.tellg();
    bool seekable = (origPos != std::streampos(-1));
    mStream.clear();
    if (seekable) mStream.seekg(0, std::ios::beg);

    while (probe.size() < kMaxProbe && mStream) {
        size_t before = probe.size();
        probe.resize(before + kChunk);
        mStream.read(reinterpret_cast<char*>(probe.data() + before), (std::streamsize)kChunk);
        std::streamsize got = mStream.gcount();
        if (got <= 0) {
            probe.resize(before);
            break;
        }
        probe.resize(before + (size_t)got);

        WebPBitstreamFeatures features;
        VP8StatusCode st = WebPGetFeatures(probe.data(), probe.size(), &features);
        if (st == VP8_STATUS_OK) {
            mImageWidth = features.width;
            mImageHeight = features.height;
            mFrameCount = (features.has_animation ? 1 : 1); // accurate frame count requires full demux; keep 1
            // If stream is not seekable, stash the probe for later decode
            if (!seekable) {
                mPrivate->size = probe.size();
                mPrivate->data = new uint8_t[mPrivate->size];
                std::memcpy(mPrivate->data, probe.data(), mPrivate->size);
                mPrivate->webp_data.bytes = mPrivate->data;
                mPrivate->webp_data.size = mPrivate->size;
            }
            if (seekable && origPos != std::streampos(-1)) mStream.seekg(origPos);
            return true;
        } else if (st == VP8_STATUS_NOT_ENOUGH_DATA) {
            continue; // read more
        } else {
            LOGE("WebPGetFeatures failed: %d", st);
            if (seekable && origPos != std::streampos(-1)) mStream.seekg(origPos);
            return false;
        }
    }

    // If we exit loop without success, restore position (if possible) and fail.
    if (seekable && origPos != std::streampos(-1)) mStream.seekg(origPos);
    return false;
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
        }
    }
#endif

    // Prefer incremental decode: feed input stream to a WebPIDecoder and decode
    // directly into the Cairo surface to avoid allocating the whole image.
    if (targetProfile == nullptr) {
        WebPDecBuffer outBuf;
        if (!WebPInitDecBuffer(&outBuf)) {
            LOGE("WebPInitDecBuffer failed");
            return nullptr;
        }
        outBuf.colorspace = MODE_bgrA; // bytes order B,G,R,A which maps to native ARGB32 on little-endian
        outBuf.width = width;
        outBuf.height = height;
        outBuf.is_external_memory = 1;
        outBuf.u.RGBA.rgba = pixels;
        outBuf.u.RGBA.stride = stride;
        outBuf.u.RGBA.size = out_size;

        WebPIDecoder* idec = WebPINewDecoder(&outBuf);
        if (!idec) {
            LOGE("WebPINewDecoder failed");
            WebPFreeDecBuffer(&outBuf);
            return nullptr;
        }

        const size_t kChunk = 16 * 1024;
        std::vector<char> buf(kChunk);
        VP8StatusCode status = VP8_STATUS_SUSPENDED;

        // If decodeSize prefetched bytes into mPrivate for non-seekable streams,
        // submit them first to the incremental decoder.
        if (mPrivate->data && mPrivate->size > 0) {
            status = WebPIAppend(idec, mPrivate->data, mPrivate->size);
            if (status != VP8_STATUS_OK && status != VP8_STATUS_SUSPENDED) {
                LOGE("WebPIAppend(prefetch) failed: %d", status);
                WebPIDelete(idec);
                WebPFreeDecBuffer(&outBuf);
                return nullptr;
            }
        }

        while (mStream && status != VP8_STATUS_OK) {
            mStream.read(buf.data(), (std::streamsize)buf.size());
            std::streamsize r = mStream.gcount();
            if (r <= 0) break;
            status = WebPIAppend(idec, reinterpret_cast<const uint8_t*>(buf.data()), (size_t)r);
            if (status != VP8_STATUS_OK && status != VP8_STATUS_SUSPENDED) {
                LOGE("WebPIAppend failed: %d", status);
                WebPIDelete(idec);
                WebPFreeDecBuffer(&outBuf);
                return nullptr;
            }
            // partial rows are already written into `pixels` by the decoder
        }

        if (status != VP8_STATUS_OK) {
            LOGE("WebP incremental decode incomplete: %d", status);
            WebPIDelete(idec);
            WebPFreeDecBuffer(&outBuf);
            return nullptr;
        }

        WebPIDelete(idec);
        WebPFreeDecBuffer(&outBuf);

        const int transparency = ImageDecoder::computeTransparency(image);
        ImageDecoder::setTransparency(image, transparency);
        return image;
    }

    // If a targetProfile is requested (color management), perform incremental decode
    // and apply LCMS transform per-row as decoded, avoiding full-file buffering.
#if ENABLE(LCMS)
    if (targetProfile != nullptr) {
        // We'll accumulate a small prefix to search for ICCP chunk.
        std::vector<uint8_t> prefetched;
        if (mPrivate->data && mPrivate->size > 0) {
            prefetched.insert(prefetched.end(), mPrivate->data, mPrivate->data + mPrivate->size);
        } else {
            // If stream is seekable, rewind to start and read small prefix for ICCP
            std::streampos orig = mStream.tellg();
            if (orig != std::streampos(-1)) {
                mStream.clear();
                mStream.seekg(0, std::ios::beg);
                const size_t preSize = 64 * 1024;
                prefetched.resize(preSize);
                mStream.read(reinterpret_cast<char*>(prefetched.data()), (std::streamsize)preSize);
                std::streamsize got = mStream.gcount();
                prefetched.resize((size_t)got);
                mStream.clear();
                mStream.seekg(orig);
            }
        }

        cmsHTRANSFORM transform = nullptr;
        cmsHPROFILE src_profile = nullptr;

        WebPIDecoder* idec = WebPINewRGB(MODE_BGRA, nullptr, 0, 0);
        if (!idec) {
            LOGE("WebPINewRGB failed");
            return nullptr;
        }

        const size_t kChunk2 = 16 * 1024;
        std::vector<char> buf(kChunk2);
        int processed_last_y = 0;
        VP8StatusCode status = VP8_STATUS_SUSPENDED;

        auto tryCreateTransform = [&]() {
            if (transform) return;
            std::vector<uint8_t> icc = extractICCP(prefetched.data(), prefetched.size());
            if (icc.empty()) return;
            src_profile = cmsOpenProfileFromMem(icc.data(), icc.size());
            if (!src_profile) return;
            cmsHTRANSFORM t = cmsCreateTransform(src_profile, TYPE_RGBA_8,
                                                 (cmsHPROFILE)targetProfile, TYPE_RGBA_8,
                                                 cmsGetHeaderRenderingIntent(src_profile), 0);
            cmsCloseProfile(src_profile);
            if (t) transform = t;
        };

        // If we prefetched some bytes, submit them first
        if (!prefetched.empty()) {
            status = WebPIAppend(idec, prefetched.data(), prefetched.size());
            tryCreateTransform();
            if (status != VP8_STATUS_OK && status != VP8_STATUS_SUSPENDED) {
                LOGE("WebPIAppend(prefetch) failed: %d", status);
                WebPIDelete(idec);
                if (transform) cmsDeleteTransform(transform);
                return nullptr;
            }
            // process any available rows
            int last_y = -1, w = 0, h = 0, decStride = 0;
            uint8_t* internal = WebPIDecGetRGB(idec, &last_y, &w, &h, &decStride);
            if (internal && last_y >= 0) {
                for (int y = processed_last_y; y <= last_y; y++) {
                    uint8_t* srcDecodedRow = internal + y * decStride;
                    // build src RGBA row
                    std::vector<uint8_t> srcRow(width * 4);
                    std::vector<uint8_t> dstRow(width * 4);
                    for (int x = 0; x < width; x++) {
                        srcRow[4 * x + 0] = srcDecodedRow[4 * x + 2]; // R
                        srcRow[4 * x + 1] = srcDecodedRow[4 * x + 1]; // G
                        srcRow[4 * x + 2] = srcDecodedRow[4 * x + 0]; // B
                        srcRow[4 * x + 3] = srcDecodedRow[4 * x + 3]; // A
                    }
                    if (transform) cmsDoTransform(transform, srcRow.data(), dstRow.data(), width);
                    uint32_t* dst32 = reinterpret_cast<uint32_t*>(pixels + y * stride);
                    for (int x = 0; x < width; x++) {
                        uint8_t a = srcRow[4 * x + 3];
                        uint8_t r = transform ? dstRow[4 * x + 0] : srcRow[4 * x + 0];
                        uint8_t g = transform ? dstRow[4 * x + 1] : srcRow[4 * x + 1];
                        uint8_t b = transform ? dstRow[4 * x + 2] : srcRow[4 * x + 2];
                        uint32_t p;
                        if (a == 0) p = 0;
                        else {
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
                processed_last_y = last_y + 1;
            }
        }

        // Continue reading and appending
        while (mStream && status != VP8_STATUS_OK) {
            mStream.read(buf.data(), (std::streamsize)buf.size());
            std::streamsize r = mStream.gcount();
            if (r <= 0) break;
            // keep small prefix for ICC search
            if (prefetched.size() < 128 * 1024) prefetched.insert(prefetched.end(), buf.data(), buf.data() + r);
            status = WebPIAppend(idec, reinterpret_cast<const uint8_t*>(buf.data()), (size_t)r);
            tryCreateTransform();
            if (status != VP8_STATUS_OK && status != VP8_STATUS_SUSPENDED) {
                LOGE("WebPIAppend failed: %d", status);
                WebPIDelete(idec);
                if (transform) cmsDeleteTransform(transform);
                return nullptr;
            }

            int last_y = -1, w = 0, h = 0, decStride = 0;
            uint8_t* internal = WebPIDecGetRGB(idec, &last_y, &w, &h, &decStride);
            if (internal && last_y >= processed_last_y) {
                for (int y = processed_last_y; y <= last_y; y++) {
                    uint8_t* srcDecodedRow = internal + y * decStride;
                    std::vector<uint8_t> srcRow(width * 4);
                    std::vector<uint8_t> dstRow(width * 4);
                    for (int x = 0; x < width; x++) {
                        srcRow[4 * x + 0] = srcDecodedRow[4 * x + 2];
                        srcRow[4 * x + 1] = srcDecodedRow[4 * x + 1];
                        srcRow[4 * x + 2] = srcDecodedRow[4 * x + 0];
                        srcRow[4 * x + 3] = srcDecodedRow[4 * x + 3];
                    }
                    if (transform) cmsDoTransform(transform, srcRow.data(), dstRow.data(), width);
                    uint32_t* dst32 = reinterpret_cast<uint32_t*>(pixels + y * stride);
                    for (int x = 0; x < width; x++) {
                        uint8_t a = srcRow[4 * x + 3];
                        uint8_t r = transform ? dstRow[4 * x + 0] : srcRow[4 * x + 0];
                        uint8_t g = transform ? dstRow[4 * x + 1] : srcRow[4 * x + 1];
                        uint8_t b = transform ? dstRow[4 * x + 2] : srcRow[4 * x + 2];
                        uint32_t p;
                        if (a == 0) p = 0;
                        else {
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
                processed_last_y = last_y + 1;
            }
        }

        if (status != VP8_STATUS_OK) {
            LOGE("WebP incremental decode incomplete: %d", status);
            WebPIDelete(idec);
            if (transform) cmsDeleteTransform(transform);
            return nullptr;
        }

        WebPIDelete(idec);
        if (transform) cmsDeleteTransform(transform);

        const int transparency = ImageDecoder::computeTransparency(image);
        ImageDecoder::setTransparency(image, transparency);
        return image;
    }
#endif
    return image;
}

bool WEBPDecoder::isWEBP(const uint8_t* contents, uint32_t header_size) {
    if (header_size < 12) return false;
    return !std::memcmp("RIFF", contents, 4) && !std::memcmp("WEBP", contents + 8, 4);
}

} // namespace cdroid

#endif /* ENABLE(WEBP) */
