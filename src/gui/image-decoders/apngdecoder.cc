#include <core/context.h>
#include <cairomm/context.h>
#include <cairomm/surface.h>
#include <image-decoders/imagedecoder.h>
#include <gui/gui_features.h>
#include <core/systemclock.h>
#include <cdlog.h>
#include "png.h"     /* original (unpatched) libpng is ok */
//REF:https://github.com/xxyyboy/img_apng2webp/blob/main/apng2png/apng2webp.c
namespace cdroid {

#define APNG_DISPOSE_OP_NONE 0
#define APNG_DISPOSE_OP_BACKGROUND 1
#define APNG_DISPOSE_OP_PREVIOUS 2

#define APNG_BLEND_OP_SOURCE 0
#define APNG_BLEND_OP_OVER 1

APNGDecoder::APNGDecoder() {
}

APNGDecoder::~APNGDecoder() {
}

int APNGDecoder::readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int frameIndex) {
    return 0;
}


static void istream_png_reader(png_structp png_ptr, png_bytep png_data, png_size_t data_size){
    std::istream* is = (std::istream*)(png_get_io_ptr(png_ptr));
    is->read(reinterpret_cast<char*>(png_data), data_size);
};

int APNGDecoder::load(std::istream&istream) {
    return 0;
}
}/*namesapce*/
