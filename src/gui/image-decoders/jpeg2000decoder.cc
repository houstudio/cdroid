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
/*********************************************************************************
 * JPEG2000 decoder using OpenJPEG (optional, guarded by USE(OPENJPEG))
 *********************************************************************************/
#include <gui_features.h>
#include <core/context.h>
#include <cairomm/surface.h>
#include <image-decoders/imagedecoder.h>
#include <porting/cdlog.h>
#include <vector>
#include <cstring>
#include <memory>
#if ENABLE(OPENJPEG)
#include <openjpeg.h>
#if ENABLE(LCMS)
#include <lcms2.h>
#endif

namespace cdroid {

struct JPEG2000Decoder::PRIVATE {
    // no persistent copy; we rely on mStream from ImageDecoder
};

JPEG2000Decoder::JPEG2000Decoder(std::istream& stream): ImageDecoder(stream) {
    mPrivate = new PRIVATE();
}

// Helper: create an OpenJPEG memory stream backed by a std::istream (no full-file buffering)
static opj_stream_t* create_opj_memory_stream(std::istream& stream) {
    opj_stream_t* opjstream = opj_stream_create(64 * 1024, OPJ_TRUE);
    if (!opjstream) return nullptr;

    // Determine stream length (if possible) while preserving position
    std::istream::pos_type orig = stream.tellg();
    std::streamoff length = -1;
    if (orig != (std::istream::pos_type)-1) {
        stream.seekg(0, std::ios::end);
        std::istream::pos_type endpos = stream.tellg();
        if (endpos != (std::istream::pos_type)-1) length = static_cast<std::streamoff>(endpos);
        stream.seekg(orig);
    }

    // read callback
    auto read_fn = [](void* p_buffer, OPJ_SIZE_T p_nb_bytes, void* p_user_data) -> OPJ_SIZE_T {
        std::istream* is = static_cast<std::istream*>(p_user_data);
        if (!is || !is->good()) return (OPJ_SIZE_T)-1;
        is->read(reinterpret_cast<char*>(p_buffer), (std::streamsize)p_nb_bytes);
        std::streamsize got = is->gcount();
        if (got <= 0) return (OPJ_SIZE_T)-1;
        return (OPJ_SIZE_T)got;
    };

    // skip callback: use seek relative
    auto skip_fn = [](OPJ_OFF_T p_nb_bytes, void* p_user_data) -> OPJ_OFF_T {
        std::istream* is = static_cast<std::istream*>(p_user_data);
        if (!is) return (OPJ_OFF_T)-1;
        std::istream::pos_type cur = is->tellg();
        if (cur == (std::istream::pos_type)-1) return (OPJ_OFF_T)-1;
        is->seekg((std::streamoff)p_nb_bytes, std::ios::cur);
        if (!is->good()) { is->clear(); is->seekg(cur); return (OPJ_OFF_T)-1; }
        std::istream::pos_type npos = is->tellg();
        if (npos == (std::istream::pos_type)-1) return (OPJ_OFF_T)-1;
        return static_cast<OPJ_OFF_T>(npos - cur);
    };

    // seek callback: absolute
    auto seek_fn = [](OPJ_OFF_T p_nb_bytes, void* p_user_data) -> OPJ_BOOL {
        std::istream* is = static_cast<std::istream*>(p_user_data);
        if (!is) return OPJ_FALSE;
        is->clear();
        is->seekg((std::streamoff)p_nb_bytes, std::ios::beg);
        return is->good() ? OPJ_TRUE : OPJ_FALSE;
    };

    // free user data: do nothing (caller manages stream lifetime)
    auto free_ud = [](void* /*p_user_data*/) {};

    opj_stream_set_user_data(opjstream, &stream, (opj_stream_free_user_data_fn)free_ud);
    if (length >= 0) opj_stream_set_user_data_length(opjstream, (OPJ_UINT64)length);
    opj_stream_set_read_function(opjstream, (opj_stream_read_fn)read_fn);
    opj_stream_set_skip_function(opjstream, (opj_stream_skip_fn)skip_fn);
    opj_stream_set_seek_function(opjstream, (opj_stream_seek_fn)seek_fn);
    return opjstream;
}

JPEG2000Decoder::~JPEG2000Decoder() {
    delete mPrivate;
}

bool JPEG2000Decoder::isJP2(const uint8_t* contents, uint32_t header_size) {
    static const uint8_t sig[12] = {0x00,0x00,0x00,0x0C, 0x6A,0x50,0x20,0x20, 0x0D,0x0A,0x87,0x0A};
    if (header_size < 12) return false;
    return std::memcmp(contents, sig, 12) == 0;
}

bool JPEG2000Decoder::isJ2K(const uint8_t* contents, uint32_t header_size) {
    if (header_size < 4) return false;
    return (contents[0] == 0xFF && contents[1] == 0x4F && contents[2] == 0xFF && contents[3] == 0x51);
}

bool JPEG2000Decoder::decodeSize() {
    // Read header bytes from stream to detect format
    uint8_t header[12] = {0};
    std::istream::pos_type origPos = mStream.tellg();
    mStream.clear();
    mStream.seekg(0, std::ios::beg);
    mStream.read(reinterpret_cast<char*>(header), sizeof(header));
    std::streamsize headerGot = mStream.gcount();
    if (origPos != (std::istream::pos_type)-1) mStream.seekg(origPos);

    opj_dparameters_t parameters;
    opj_set_default_decoder_parameters(&parameters);

    opj_codec_t* codec = nullptr;
    if (JPEG2000Decoder::isJP2(header, (uint32_t)headerGot)) codec = opj_create_decompress(OPJ_CODEC_JP2);
    else if (JPEG2000Decoder::isJ2K(header, (uint32_t)headerGot)) codec = opj_create_decompress(OPJ_CODEC_J2K);
    else return false;

    // Ensure stream is at beginning for OpenJPEG callbacks
    mStream.clear();
    mStream.seekg(0, std::ios::beg);

    opj_stream_t* stream = create_opj_memory_stream(mStream);
    if (!stream) { opj_destroy_codec(codec); return false; }

    if (!opj_setup_decoder(codec, &parameters)) { opj_stream_destroy(stream); opj_destroy_codec(codec); return false; }

    opj_image_t* image = nullptr;
    if (!opj_read_header(stream, codec, &image)) { opj_stream_destroy(stream); opj_destroy_codec(codec); return false; }

    if (!image) { opj_stream_destroy(stream); opj_destroy_codec(codec); return false; }

    // Use component 0 for dimensions
    mImageWidth = (int)image->comps[0].w;
    mImageHeight = (int)image->comps[0].h;
    mFrameCount = 1;

    opj_image_destroy(image);
    opj_stream_destroy(stream);
    opj_destroy_codec(codec);
    return true;
}

Cairo::RefPtr<Cairo::ImageSurface> JPEG2000Decoder::decode(float scale, void* targetProfile) {
    // Read header bytes to detect format
    uint8_t header[12] = {0};
    std::istream::pos_type origPos = mStream.tellg();
    mStream.clear();
    mStream.seekg(0, std::ios::beg);
    mStream.read(reinterpret_cast<char*>(header), sizeof(header));
    std::streamsize headerGot = mStream.gcount();
    if (origPos != (std::istream::pos_type)-1) mStream.seekg(origPos);

    opj_dparameters_t parameters;
    opj_set_default_decoder_parameters(&parameters);

    opj_codec_t* codec = nullptr;
    if (JPEG2000Decoder::isJP2(header, (uint32_t)headerGot)) codec = opj_create_decompress(OPJ_CODEC_JP2);
    else if (JPEG2000Decoder::isJ2K(header, (uint32_t)headerGot)) codec = opj_create_decompress(OPJ_CODEC_J2K);
    else return nullptr;

    // Ensure stream at start for OpenJPEG
    mStream.clear();
    mStream.seekg(0, std::ios::beg);

    opj_stream_t* stream = create_opj_memory_stream(mStream);
    if (!stream) { opj_destroy_codec(codec); return nullptr; }

    if (!opj_setup_decoder(codec, &parameters)) { opj_stream_destroy(stream); opj_destroy_codec(codec); return nullptr; }

    opj_image_t* image = nullptr;
    if (!opj_read_header(stream, codec, &image)) { opj_stream_destroy(stream); opj_destroy_codec(codec); return nullptr; }

    if (!image) { opj_stream_destroy(stream); opj_destroy_codec(codec); return nullptr; }

    if (!opj_decode(codec, stream, image)) { opj_image_destroy(image); opj_stream_destroy(stream); opj_destroy_codec(codec); return nullptr; }

    if (!opj_end_decompress(codec, stream)) { opj_image_destroy(image); opj_stream_destroy(stream); opj_destroy_codec(codec); return nullptr; }

    int width = image->comps[0].w;
    int height = image->comps[0].h;
    int numcomps = image->numcomps;

    Cairo::RefPtr<Cairo::ImageSurface> surf = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, width, height);
    uint8_t* dst = surf->get_data();
    int stride = surf->get_stride();

    // Prepare per-component scaling
    std::vector<double> scaleFactor(numcomps, 1.0);
    for (int c = 0; c < numcomps; ++c) {
        int prec = image->comps[c].prec;
        double maxv = (prec >= 31) ? (double)0x7fffffff : (double)((1u << prec) - 1u);
        scaleFactor[c] = 255.0 / std::max(1.0, maxv);
    }

#if ENABLE(LCMS)
    cmsHTRANSFORM transform = nullptr;
    if (targetProfile != nullptr) {
        cmsHPROFILE src_profile = nullptr;
        if (image->icc_profile_buf && image->icc_profile_len > 0) {
            src_profile = cmsOpenProfileFromMem(image->icc_profile_buf, image->icc_profile_len);
        } else {
            src_profile = cmsCreate_sRGBProfile();
        }
        if (src_profile) {
            transform = cmsCreateTransform(src_profile, TYPE_RGBA_8, (cmsHPROFILE)targetProfile, TYPE_RGBA_8,
                                           cmsGetHeaderRenderingIntent(src_profile), 0);
            cmsCloseProfile(src_profile);
        }
    }
#endif

    // Convert samples to ARGB (optionally via LCMS per-row transform)
    std::vector<uint8_t> srcRow(width * 4);
    std::vector<uint8_t> dstRow(width * 4);
    for (int y = 0; y < height; ++y) {
        uint32_t* row = reinterpret_cast<uint32_t*>(dst + y * stride);
        for (int x = 0; x < width; ++x) {
            int r = 0, g = 0, b = 0, a = 255;
            if (numcomps >= 3) {
                r = static_cast<int>(image->comps[0].data[y * width + x] * scaleFactor[0] + 0.5);
                g = static_cast<int>(image->comps[1].data[y * width + x] * scaleFactor[1] + 0.5);
                b = static_cast<int>(image->comps[2].data[y * width + x] * scaleFactor[2] + 0.5);
            } else if (numcomps == 1) {
                r = g = b = static_cast<int>(image->comps[0].data[y * width + x] * scaleFactor[0] + 0.5);
            }
            if (numcomps >= 4) {
                a = static_cast<int>(image->comps[3].data[y * width + x] * scaleFactor[3] + 0.5);
            }
            r = std::min(255, std::max(0, r));
            g = std::min(255, std::max(0, g));
            b = std::min(255, std::max(0, b));
            a = std::min(255, std::max(0, a));
            srcRow[4 * x + 0] = static_cast<uint8_t>(r);
            srcRow[4 * x + 1] = static_cast<uint8_t>(g);
            srcRow[4 * x + 2] = static_cast<uint8_t>(b);
            srcRow[4 * x + 3] = static_cast<uint8_t>(a);
        }

#if ENABLE(LCMS)
        if (transform) {
            cmsDoTransform(transform, srcRow.data(), dstRow.data(), width);
            for (int x = 0; x < width; ++x) {
                uint8_t a = dstRow[4 * x + 3];
                uint8_t r = dstRow[4 * x + 0];
                uint8_t g = dstRow[4 * x + 1];
                uint8_t b = dstRow[4 * x + 2];
                uint32_t p;
                if (a == 0) p = 0;
                else {
                    if (a != 255) {
                        r = (uint8_t)((a * r + 0x80 + ((a * r + 0x80) >> 8)) >> 8);
                        g = (uint8_t)((a * g + 0x80 + ((a * g + 0x80) >> 8)) >> 8);
                        b = (uint8_t)((a * b + 0x80 + ((a * b + 0x80) >> 8)) >> 8);
                    }
                    p = (uint32_t(a) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
                }
                row[x] = p;
            }
            continue;
        }
#endif

        // No transform path: write premultiplied values directly
        for (int x = 0; x < width; ++x) {
            uint8_t r = srcRow[4 * x + 0];
            uint8_t g = srcRow[4 * x + 1];
            uint8_t b = srcRow[4 * x + 2];
            uint8_t a = srcRow[4 * x + 3];
            uint32_t p;
            if (a == 0) p = 0;
            else {
                if (a != 255) {
                    r = (uint8_t)((a * r + 0x80 + ((a * r + 0x80) >> 8)) >> 8);
                    g = (uint8_t)((a * g + 0x80 + ((a * g + 0x80) >> 8)) >> 8);
                    b = (uint8_t)((a * b + 0x80 + ((a * b + 0x80) >> 8)) >> 8);
                }
                p = (uint32_t(a) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
            }
            row[x] = p;
        }
    }

    opj_image_destroy(image);
    opj_stream_destroy(stream);
    opj_destroy_codec(codec);
#if ENABLE(LCMS)
    if(transform){
        cmsDeleteTransform(transform);
    }
#endif
    const int transparency = ImageDecoder::computeTransparency(surf);
    ImageDecoder::setTransparency(surf, transparency);
    return surf;
}

} // namespace cdroid
#endif // USE(OPENJPEG)
