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
#include <core/context.h>
#include <cairomm/context.h>
#include <cairomm/surface.h>
#include <drawable/drawable.h>
#include <image-decoders/imagedecoder.h>
#include <core/systemclock.h>
#include <cdlog.h>
#if ENABLE(LCMS)
#include <lcms2.h>
#endif
#include <png.h>     /* original (unpatched) libpng is ok */
//REF:https://github.com/xxyyboy/img_apng2webp/blob/main/apng2png/apng2webp.c
//https://gitee.com/mirrors_line/apng-drawable.git
namespace cdroid {

#if defined(PNG_LIBPNG_VER_MAJOR) && defined(PNG_LIBPNG_VER_MINOR) && (PNG_LIBPNG_VER_MAJOR > 1 || (PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 4))
#define JMPBUF(png_ptr) png_jmpbuf(png_ptr)
#else
#define JMPBUF(png_ptr) png_ptr->jmpbuf
#endif

struct PRIVATE {
    png_structp png_ptr;
    png_infop info_ptr;
    int transparency;
    std::istream*istream;
};

static void istream_png_reader(png_structp png_ptr, png_bytep png_data, png_size_t data_size) {
    PRIVATE*priv = (PRIVATE*)(png_get_io_ptr(png_ptr));
    priv->istream->read(reinterpret_cast<char*>(png_data), data_size);
}
static void png_warning_handler(png_structp png_ptr, png_const_charp warning_msg) {
    LOGW("Warning: %s", warning_msg);
}
static void png_error_handler(png_structp png_ptr, png_const_charp error_msg) {
    LOGE("png decoder error:%s\r\n",error_msg);
    longjmp(JMPBUF(png_ptr), 1);
}
static void PNGAPI headerAvailable(png_structp png, png_infop){
    LOGD("%p header available",png);
    //static_cast<PNGDecoder*>(png_get_progressive_ptr(png))->headerAvailable();
}
static void PNGAPI rowAvailable(png_structp png, png_bytep rowBuffer, png_uint_32 rowIndex, int interlacePass){
    LOGD("%p rowAvailable",png);
    //static_cast<PNGDecoder*>(png_get_progressive_ptr(png))->rowAvailable(rowBuffer, rowIndex, interlacePass);
}
static void PNGAPI pngComplete(png_structp png, png_infop){
    LOGD("%p pngComplete ",png);
    //static_cast<PNGDecoder*>(png_get_progressive_ptr(png))->pngComplete();
}

PNGDecoder::PNGDecoder(std::istream&stream):ImageDecoder(stream) {
    mPrivate = new PRIVATE();
    mPrivate->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, png_error_handler, png_warning_handler);
    mPrivate->info_ptr= png_create_info_struct(mPrivate->png_ptr);
    png_set_progressive_read_fn(mPrivate->png_ptr, this, headerAvailable, rowAvailable, pngComplete);
    mPrivate->transparency = PixelFormat::UNKNOWN;
    mPrivate->istream = &mStream;
    png_set_read_fn(mPrivate->png_ptr,mPrivate,istream_png_reader);
}

PNGDecoder::~PNGDecoder() {
    if(mPrivate->png_ptr)
        png_destroy_read_struct(&mPrivate->png_ptr, &mPrivate->info_ptr, nullptr);
    delete mPrivate;
}

bool PNGDecoder::isPNG(const uint8_t* contents,uint32_t header_size){
    return !memcmp((const char*)contents, "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8);
}

void*PNGDecoder::getColorProfile(PRIVATE*priv,uint8_t colorType) {
#if ENABLE(LCMS)
    if (png_get_valid(priv->png_ptr, priv->info_ptr, PNG_INFO_iCCP)) {
        png_charp name;
        png_bytep icc_data;
        png_uint_32 icc_size;
        int comp_type;
        png_get_iCCP(priv->png_ptr, priv->info_ptr, &name, &comp_type, &icc_data, &icc_size);

        cmsHPROFILE src_profile = cmsOpenProfileFromMem(icc_data, icc_size);
        cmsColorSpaceSignature profileSpace = cmsGetColorSpace(src_profile);

        if ((profileSpace != cmsSigRgbData) && ((colorType & PNG_COLOR_MASK_COLOR) || (profileSpace != cmsSigGrayData))) {
            cmsCloseProfile(src_profile);
            return nullptr;
        }
        return src_profile;
    } else {
        return cmsCreate_sRGBProfile();
    }
#else
    return nullptr;
#endif
}

Cairo::RefPtr<Cairo::ImageSurface> PNGDecoder::decode(float scale,void*targetProfile) {
    png_structp png_ptr = mPrivate->png_ptr;
    png_infop info_ptr  = mPrivate->info_ptr;

    if( (mImageWidth==-1) || (mImageHeight==-1) )
        decodeSize();
    std::vector<png_bytep> row_pointers(mImageHeight);
    Cairo::RefPtr<Cairo::ImageSurface> image = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,mImageWidth,mImageHeight);
    uint8_t*frame_pixels = image->get_data();

    for (png_uint_32 y = 0; y < mImageHeight; ++y) {
        row_pointers[y] = (frame_pixels + y* mImageWidth * 4);
        memset(row_pointers[y],0,mImageWidth * 4);
    }
#if ENABLE(LCMS)
    cmsHTRANSFORM transform = nullptr;
    if(targetProfile) {
        const uint32_t inType = TYPE_RGBA_8;
        const uint8_t colorType = png_get_color_type(png_ptr,info_ptr);
        cmsHPROFILE src_profile = getColorProfile(mPrivate, colorType);
        const uint32_t profileSpace = cmsGetColorSpace(src_profile);
        transform = cmsCreateTransform(src_profile, inType, targetProfile, inType,
                                        cmsGetHeaderRenderingIntent(src_profile), 0);
        cmsCloseProfile(src_profile);
    }
#else
    png_read_image(png_ptr, row_pointers.data());
#endif
#if ENABLE(LCMS)
    if(transform==nullptr){
        png_read_image(png_ptr, row_pointers.data());
    }else{
        uint8_t *srcLine = new uint8_t[mImageWidth*4];
        for(uint32_t i = 0;i < mImageHeight;i++){
            png_read_row(png_ptr,srcLine,nullptr);
            cmsDoTransform(transform,srcLine,row_pointers[i],mImageWidth);
            uint8_t *pd = row_pointers[i] , *ps = srcLine;
            for(int j = 0;j < mImageWidth;j++,ps+=4,pd+=4)pd[3]=ps[3];
        }
        delete []srcLine;
        cmsDeleteTransform(transform);
    }
#endif
    png_read_end (png_ptr, info_ptr);
    cairo_surface_set_mime_data(image->cobj(), CAIRO_MIME_TYPE_PNG, nullptr, 0, nullptr,nullptr);
    const int transparency = mPrivate->transparency!=PixelFormat::UNKNOWN ? mPrivate->transparency:ImageDecoder::computeTransparency(image);
    ImageDecoder::setTransparency(image,transparency);
    return image;
}

static inline int multiply_alpha (int alpha, int color) {
    int temp = (alpha * color) + 0x80;
    return ((temp + (temp >> 8)) >> 8);
}

/* Premultiplies data and converts RGBA bytes => native endian */
static void premultiply_data (png_structp png, png_row_infop row_info, png_bytep data) {
    unsigned int i;

    for (i = 0; i < row_info->rowbytes; i += 4) {
        uint32_t p;
        uint8_t *base = &data[i];
        uint8_t alpha = base[3];

        if (alpha == 0) {
            p = 0;
        } else {
            uint8_t red   = base[0];
            uint8_t green = base[1];
            uint8_t blue  = base[2];

            if (alpha != 0xff) {
                red   = multiply_alpha (alpha, red);
                green = multiply_alpha (alpha, green);
                blue  = multiply_alpha (alpha, blue);
            }
            p = ((uint32_t)alpha << 24) | (red << 16) | (green << 8) | (blue << 0);
        }
        memcpy (base, &p, sizeof (uint32_t));
    }
}

static uint16_t f_to_u16(float val){
    if (val < 0) return 0;
    else if (val > 1) return 65535;
    else return (uint16_t)(val * 65535.f);
}

static void unpremultiply_float (float *f, uint16_t *d16, unsigned width){
    unsigned int i;

    for (i = 0; i < width; i++) {
        float r, g, b, a;
        r = *f++;  g = *f++;
        b = *f++;  a = *f++;

        if (a > 0) {
            *d16++ = f_to_u16(r / a);
            *d16++ = f_to_u16(g / a);
            *d16++ = f_to_u16(b / a);
            *d16++ = f_to_u16(a);
        } else {
            *d16++ = 0;  *d16++ = 0;
            *d16++ = 0;  *d16++ = 0;
        }
    }
}

double PNGDecoder::setGamma(Context*ctx,png_structp png_ptr,png_infop info_ptr){
    constexpr double cMaxGamma = 21474.83;
    constexpr double cDefaultGamma = 2.2;/*default screen gamma*/
    constexpr double cInverseGamma = 0.45455;
    int intent;
    double gamma = cInverseGamma;

    if(png_get_sRGB(png_ptr, info_ptr, &intent)){
        png_set_gamma(png_ptr, cDefaultGamma, PNG_DEFAULT_sRGB);
        gamma = PNG_DEFAULT_sRGB;
    }else if(png_get_gAMA(png_ptr, info_ptr,&gamma)){
        if( (gamma<=0.0)||(gamma>cMaxGamma)){
            gamma = cInverseGamma;
            png_set_gAMA(png_ptr, info_ptr,gamma);
        }
        png_set_gamma(png_ptr, cDefaultGamma, gamma);
    }else{
        gamma = cInverseGamma;
        png_set_gamma(png_ptr,cDefaultGamma,cInverseGamma);
    }
    return gamma;
}

bool PNGDecoder::decodeSize() {
    cairo_format_t format;
    int bit_depth,interlace,color_type;

    png_structp png_ptr = mPrivate->png_ptr;
    png_infop info_ptr = mPrivate->info_ptr;

    if (setjmp(JMPBUF(png_ptr))) {
        //png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return false;
    }
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr,(uint32_t*)&mImageWidth, (uint32_t*)&mImageHeight, &bit_depth, &color_type,&interlace, NULL, NULL);

    /* convert palette/gray image to rgb */
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
        color_type = PNG_COLOR_TYPE_RGB;
    }

    /* expand gray bit depth if needed */
    if ( (color_type == PNG_COLOR_TYPE_GRAY) && (bit_depth < 8) ) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }

    /* transform transparency to alpha */
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        int num_trans = 0;
        png_bytep trans_alpha = nullptr;
        png_color_16p trans_color = nullptr;
        png_get_tRNS(png_ptr, info_ptr, &trans_alpha, &num_trans, &trans_color);
        if(num_trans==0)mPrivate->transparency=PixelFormat::OPAQUE;
        png_set_tRNS_to_alpha(png_ptr);
    }

    if (bit_depth < 8)
        png_set_packing (png_ptr);

    /* convert grayscale to RGB */
    if ( (color_type==PNG_COLOR_TYPE_GRAY) || (color_type==PNG_COLOR_TYPE_GRAY_ALPHA) ) {
        png_set_gray_to_rgb(png_ptr);
    }
    if(color_type==PNG_COLOR_TYPE_RGB){
        png_set_expand(png_ptr);
        png_set_add_alpha(png_ptr,0xFFU,PNG_FILLER_AFTER);
    }
    if (interlace != PNG_INTERLACE_NONE)
        png_set_interlace_handling (png_ptr);
    png_set_filler (png_ptr, 0xff, PNG_FILLER_AFTER);

    /* recheck header after setting EXPAND options */
    png_read_update_info (png_ptr, info_ptr);
    png_get_IHDR (png_ptr, info_ptr,(uint32_t*)&mImageWidth, (uint32_t*)&mImageHeight,
            &bit_depth, &color_type, &interlace, nullptr, nullptr);

    //setGamma(nullptr,png_ptr,info_ptr);

    LOGE_IF((bit_depth != 8 && bit_depth != 16) || ! (color_type == PNG_COLOR_TYPE_RGB ||  color_type == PNG_COLOR_TYPE_RGB_ALPHA),
            "Decoder Error");

#ifdef PNG_APNG_SUPPORTED
    png_get_acTL(png_ptr, info_ptr, (uint32_t*)&mFrameCount,nullptr);
#endif

    if(color_type==PNG_COLOR_TYPE_RGB_ALPHA) {
        if(bit_depth == 8){
            png_set_read_user_transform_fn (png_ptr, premultiply_data);
        }else {
            //png_set_read_user_transform_fn (png_ptr, premultiply_floatdata);
            LOGD("TODO:CAIRO_FORMAT_RGBA128F bit_depth=%d",bit_depth);
        }
    }
    return true;
}
}/*namesapce*/
