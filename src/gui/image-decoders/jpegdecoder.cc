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
#include <stdio.h>
#include <stddef.h>
#include <setjmp.h>
#include <jpeglib.h>
#include <gui_features.h>
#include <image-decoders/imagedecoder.h>
#include <cdlog.h>
#if ENABLE(LCMS)
#include <lcms2.h>
#endif

#if defined(ENABLE_TURBOJPEG) && ENABLE_TURBOJPEG
#include <turbojpeg.h>
#endif

#if defined(ENABLE_JPEG) && ENABLE_JPEG
#include <jpeglib.h>
#endif

namespace cdroid{

#if defined(ENABLE_JPEG) && ENABLE_JPEG
struct decoder_error_mgr {
    struct jpeg_error_mgr pub; // "public" fields for IJG library
    jmp_buf setjmp_buffer;     // For handling catastropic errors
};

struct JpegStream {
    jpeg_source_mgr pub;
    std::istream* stream;
    unsigned char buffer [4096];
};

static void init_source (j_decompress_ptr cinfo) {
    auto src = (JpegStream*)(cinfo->src);
}

static void handle_jpeg_error(j_common_ptr cinfo) {
    struct decoder_error_mgr *err = (struct decoder_error_mgr*)(cinfo->err);
    char jpegLastErrorMsg[JMSG_LENGTH_MAX];
    (*(cinfo->err->format_message))(cinfo, jpegLastErrorMsg);
    jpeg_destroy_decompress((j_decompress_ptr)cinfo);
    LOGE("JPEG read/write error:%s",jpegLastErrorMsg);
    longjmp(err->setjmp_buffer, 1);
}

static boolean fill_buffer (j_decompress_ptr cinfo) {
    // Read to buffer
    JpegStream* src = (JpegStream*)(cinfo->src);
    src->stream->read((char*)src->buffer,sizeof(src->buffer));
    src->pub.next_input_byte = src->buffer;
    src->pub.bytes_in_buffer = src->stream->gcount();// How many yo could read

    return (!src->stream->eof())||src->pub.bytes_in_buffer;
}

static void skip (j_decompress_ptr cinfo, long num_bytes) {
    JpegStream*src = (JpegStream*)cinfo->src;
    if (num_bytes > 0) {
        while (num_bytes > (long)src->pub.bytes_in_buffer) {
            num_bytes -= (long)src->pub.bytes_in_buffer;
            (void)(*src->pub.fill_input_buffer) (cinfo);
        }
        src->pub.next_input_byte += (size_t)num_bytes;
        src->pub.bytes_in_buffer -= (size_t)num_bytes;
    }
}

static void term (j_decompress_ptr cinfo) {
    // Close the stream, can be nop
}

static void make_jpeg_stream (j_decompress_ptr cinfo, std::istream* in) {
    JpegStream * src;
    if (cinfo->src == NULL) {/* first time for this JPEG object? */
        cinfo->src =(struct jpeg_source_mgr*)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo,0/*POOL_PERMANENT*/,sizeof(JpegStream));
        src = reinterpret_cast<JpegStream*> (cinfo->src);
    }
    src = reinterpret_cast<JpegStream*> (cinfo->src);
    src->pub.init_source = init_source;
    src->pub.fill_input_buffer = fill_buffer;
    src->pub.skip_input_data = skip;
    src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
    src->pub.term_source = term;
    src->stream = in;
    src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
    src->pub.next_input_byte = NULL; /* until buffer loaded */
}

#ifndef LIBJPEG_TURBO_VERSION
static void pix_conv(unsigned char *dst, int dw, const unsigned char *src, int sw, int num) {
    int si, di;
    // safety check
    if (dw < 3 || sw < 3 || dst == NULL || src == NULL)
        return;
    num--;
    for (si = num * sw, di = num * dw; si >= 0; si -= sw, di -= dw) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        dst[di + 2] = src[si    ];
        dst[di + 1] = src[si + 1];
        dst[di + 0] = src[si + 2];
#else
        // FIXME: This is untested, it may be wrong.
        dst[di - 3] = src[si - 3];
        dst[di - 2] = src[si - 2];
        dst[di - 1] = src[si - 1];
#endif
    }
}
#endif

static int gcd(int a, int b) {
    if (b == 0) return a;
    return gcd(b, a % b);
}

static void decimalToFraction(double decimal,int& scale_num,int& scale_denom) {
    const int precision = 1000000;
    const int numerator = decimal * precision;
    const int denominator = precision;
    const int commonDivisor = gcd(numerator, denominator);
    scale_num   = numerator / commonDivisor;
    scale_denom = denominator / commonDivisor;
}

struct PRIVATE{
    struct jpeg_decompress_struct cinfo;
    struct decoder_error_mgr jerr;
};

JPEGDecoder::JPEGDecoder(std::istream&stream):ImageDecoder(stream){
    mPrivate = new  PRIVATE;
    struct jpeg_decompress_struct* cinfo = &mPrivate->cinfo;
    struct decoder_error_mgr* jerr = &mPrivate->jerr;

    cinfo->err = jpeg_std_error(&jerr->pub);
    jerr->pub.error_exit = handle_jpeg_error;
    jpeg_create_decompress(cinfo);
    make_jpeg_stream(cinfo,&mStream);
}

JPEGDecoder::~JPEGDecoder(){
   delete mPrivate;
}

bool JPEGDecoder::isJPEG(const uint8_t* contents,uint32_t header_size){
    return !memcmp((const char*)contents, "\xFF\xD8\xFF", 3);
}

Cairo::RefPtr<Cairo::ImageSurface> JPEGDecoder::decode(float scale,void*targetProfile){
    Cairo::RefPtr<Cairo::ImageSurface>image;
    struct jpeg_decompress_struct* cinfo = &mPrivate->cinfo;
    struct decoder_error_mgr* jerr = &mPrivate->jerr;
    int scale_num,scale_denom;
    JSAMPROW row_pointer[1];
    // initialize jpeg decompression structures
    if (setjmp(jerr->setjmp_buffer)){
        return image;
    }
    if((mImageWidth==-1)||(mImageHeight==-1)){
        decodeSize();
    }

    decimalToFraction(scale,scale_num,scale_denom);
    if(scale_num){
        cinfo->scale_num  = scale_num;
        cinfo->scale_denom= scale_denom;
    }
#if ENABLE(LCMS)
    cmsHTRANSFORM transform = nullptr;
    if(targetProfile){
        cmsHPROFILE src_profile = getColorProfile(mPrivate);
        if (src_profile) {
            cmsUInt32Number inType=TYPE_RGBA_8;
            cmsColorSpaceSignature profileSpace = cmsGetColorSpace(src_profile);

            transform = cmsCreateTransform(src_profile, inType, targetProfile, TYPE_RGBA_8,
                             cmsGetHeaderRenderingIntent(src_profile), 0);

            cmsCloseProfile(src_profile);
        }
    }
#endif
#ifdef LIBJPEG_TURBO_VERSION
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    cinfo->out_color_space = JCS_EXT_BGRA;
#else
    cinfo->out_color_space = JCS_EXT_ARGB;
#endif
#else
    cinfo->out_color_space = JCS_RGB;
#endif
    // start decompressor
    (void) jpeg_start_decompress(cinfo);
    // create Cairo image surface
    image = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, cinfo->output_width, cinfo->output_height);
    if (image ==nullptr) {
        jpeg_destroy_decompress(cinfo);
        return nullptr;
    }

    // loop over all scanlines and fill Cairo image surface
    uint8_t*srcLine = new uint8_t[image->get_stride()];
    while (cinfo->output_scanline < cinfo->output_height) {
        unsigned char *row_address = image->get_data() +cinfo->output_scanline * image->get_stride();
#if ENABLE(LCMS)
        if(transform==nullptr)
            row_pointer[0] = row_address;
        else
            row_pointer[0] = srcLine;
#else
        row_pointer[0] = row_address;
#endif
        if(jpeg_read_scanlines(cinfo, row_pointer, 1)!=1){
            LOGW("jpeg data corrupt at scanline=%d/%d",cinfo->output_scanline,cinfo->output_height);
            break;
        }
#ifndef LIBJPEG_TURBO_VERSION
        pix_conv(row_address, 4, row_address, 3, cinfo->output_width);
#endif
#if ENABLE(LCMS)
        if(transform)cmsDoTransform(transform, srcLine, row_address, mImageWidth);
#endif
    }
    delete []srcLine;
#if ENABLE(LCMS)
    if(transform){
        cmsDeleteTransform(transform);
    }
#endif
    // finish and close everything
    (void) jpeg_finish_decompress(cinfo);
    jpeg_destroy_decompress(cinfo);

    // set jpeg mime data
    cairo_surface_set_mime_data(image->cobj(),CAIRO_MIME_TYPE_JPEG,nullptr,0,nullptr,nullptr);
    return image;
}

bool JPEGDecoder::decodeSize(){
    struct jpeg_decompress_struct* cinfo = &mPrivate->cinfo;
    jpeg_read_header(cinfo, TRUE);
    mImageWidth = cinfo->image_width;
    mImageHeight= cinfo->image_height;
    return true;
}

void* JPEGDecoder::getColorProfile(PRIVATE*priv) {
#if ENABLE(LCMS)
    JOCTET* icc_data;
    unsigned int icc_size;
    if (jpeg_read_icc_profile(&priv->cinfo, &icc_data, &icc_size)) {
        cmsHPROFILE src_profile = cmsOpenProfileFromMem(icc_data, icc_size);
        free(icc_data);

        cmsColorSpaceSignature profileSpace = cmsGetColorSpace(src_profile);

        if (profileSpace != cmsSigRgbData &&
            (mPrivate->cinfo.jpeg_color_space != JCS_GRAYSCALE ||
            profileSpace != cmsSigGrayData)) {
            cmsCloseProfile(src_profile);
            return nullptr;
        }
        LOGD("profileSpace=%x",profileSpace);
        return src_profile;
    } else {
        LOGD("profileSpace sRGB");
        return cmsCreate_sRGBProfile();
    }
#else
    return nullptr;
#endif
}
#endif/*ENABLE_JPEG*/
}/*endof namespacee*/
