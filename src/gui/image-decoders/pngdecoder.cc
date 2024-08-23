#include <core/context.h>
#include <cairomm/context.h>
#include <cairomm/surface.h>
#include <image-decoders/imagedecoder.h>
#include <core/systemclock.h>
#include <cdlog.h>
#include <lcms2.h>
#include "png.h"     /* original (unpatched) libpng is ok */
//REF:https://github.com/xxyyboy/img_apng2webp/blob/main/apng2png/apng2webp.c
//https://gitee.com/mirrors_line/apng-drawable.git
namespace cdroid {

#define PNG_JMPBUF(x) png_jmpbuf((png_structp) x)
struct PRIVATE{
    png_structp png_ptr;
    png_infop info_ptr;
};

static void istream_png_reader(png_structp png_ptr, png_bytep png_data, png_size_t data_size){
    std::istream* is = (std::istream*)(png_get_io_ptr(png_ptr));
    is->read(reinterpret_cast<char*>(png_data), data_size);
}

PNGDecoder::PNGDecoder(std::istream&stm):ImageDecoder(stm) {
    mPrivate = new PRIVATE();
    mPrivate->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    mPrivate->info_ptr= png_create_info_struct(mPrivate->png_ptr);

    png_set_read_fn(mPrivate->png_ptr,(void*)istream,istream_png_reader);
}

PNGDecoder::~PNGDecoder() {
    png_destroy_read_struct(&mPrivate->png_ptr, &mPrivate->info_ptr, nullptr);
    delete mPrivate;
}

static void png_error_fn(png_structp png_ptr, png_const_charp msg) {
    LOGE("------ png error %s", msg);
    longjmp(PNG_JMPBUF(png_ptr), -1);
}

void*PNGDecoder::getColorProfile(PRIVATE*priv,uint8_t colorType){
#if ENABLE(LCMS)
    if (png_get_valid(priv->png_ptr, priv->info_ptr, PNG_INFO_iCCP)) {
        png_charp name;
        png_bytep icc_data;
        png_uint_32 icc_size;
        int comp_type;
        png_get_iCCP(priv->png_ptr, priv->info_ptr, &name, &comp_type, &icc_data, &icc_size);

        cmsHPROFILE src_profile = cmsOpenProfileFromMem(icc_data, icc_size);
        cmsColorSpaceSignature profileSpace = cmsGetColorSpace(src_profile);

        if (profileSpace != cmsSigRgbData &&
            (colorType & PNG_COLOR_MASK_COLOR || profileSpace != cmsSigGrayData)) {
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

Cairo::RefPtr<Cairo::ImageSurface> PNGDecoder::decode(float scale,void*targetProfile){
    png_structp png_ptr =mPrivate->png_ptr;
    png_infop info_ptr=mPrivate->info_ptr;

    if( (mImageWidth==-1) || (mImageHeight==-1) )
        decodeSize();
    std::vector<png_bytep> row_pointers(mImageHeight);
    Cairo::RefPtr<Cairo::ImageSurface> image = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,mImageWidth,mImageHeight);
    unsigned char*frame_pixels=image->get_data();

    for (png_uint_32 y = 0; y < mImageHeight; ++y) {
        row_pointers[y] = frame_pixels + y* mImageWidth * 4;
    }
#if ENABLE(LCMS)
    if(targetProfile){
        const int inType = TYPE_RGBA_8;
        const uint8_t colorType = png_get_color_type(png_ptr,info_ptr);
        cmsHPROFILE src_profile = getColorProfile(mPrivate, colorType);
        const uint32_t profileSpace = cmsGetColorSpace(src_profile);
        mTransform = cmsCreateTransform(src_profile, inType, targetProfile, TYPE_RGBA_8,
                             cmsGetHeaderRenderingIntent(src_profile), 0);
        cmsCloseProfile(src_profile);
    }
#endif
    png_read_image(png_ptr, row_pointers.data());
    image->set_mime_data(CAIRO_MIME_TYPE_PNG, nullptr, 0, nullptr);
    return image;
}

bool PNGDecoder::decodeSize() {
    png_structp png_ptr = mPrivate->png_ptr;
    png_infop info_ptr = mPrivate->info_ptr;
    if (setjmp(png_jmpbuf(png_ptr))) {
        LOGE("Error during libpng init_io");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return false;
    }
    png_set_bgr(png_ptr);
    png_read_info(png_ptr, info_ptr);

    int bit_depth, color_type;
    png_get_IHDR(png_ptr, info_ptr,(uint32_t*)&mImageWidth, (uint32_t*)&mImageHeight, &bit_depth, &color_type, NULL, NULL, NULL);

    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
        color_type = PNG_COLOR_TYPE_RGB;
        bit_depth = 8;
    }

    if ( (color_type == PNG_COLOR_TYPE_GRAY) && (bit_depth < 8) ) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
        bit_depth = 8;
    }

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr);
        color_type |= PNG_COLOR_MASK_ALPHA;
    }

    if ( (color_type==PNG_COLOR_TYPE_GRAY) || (color_type==PNG_COLOR_TYPE_GRAY_ALPHA) ) {
        png_set_gray_to_rgb(png_ptr);
        color_type |= PNG_COLOR_MASK_COLOR;
        //is_gray = true;
    }
    if (color_type==PNG_COLOR_TYPE_RGB)
        png_set_filler(png_ptr,0xffffU,PNG_FILLER_AFTER);

    png_read_update_info(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);

    return true;
}
}/*namesapce*/
