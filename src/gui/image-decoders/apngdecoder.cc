#include <core/context.h>
#include <cairomm/context.h>
#include <cairomm/surface.h>
#include <image-decoders/imagedecoder.h>
#include <gui/gui_features.h>
#include <core/systemclock.h>
#include <cdlog.h>
#include "png.h"     /* original (unpatched) libpng is ok */
//REF:https://github.com/xxyyboy/img_apng2webp/blob/main/apng2png/apng2webp.c
//https://gitee.com/mirrors_line/apng-drawable.git
namespace cdroid {

#define APNG_DISPOSE_OP_NONE 0
#define APNG_DISPOSE_OP_BACKGROUND 1
#define APNG_DISPOSE_OP_PREVIOUS 2

#define APNG_BLEND_OP_SOURCE 0
#define APNG_BLEND_OP_OVER 1

struct PRIVATE{
    png_structp png_ptr;
    png_infop info_ptr;
};
APNGDecoder::APNGDecoder(std::unique_ptr<std::istream>stm):ImageDecoder(std::move(stm)) {
    mPrivate = new PRIVATE();
}

APNGDecoder::~APNGDecoder() {
}
int APNGDecoder::readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int frameIndex) {
    png_uint_16 delay_num , delay_den;
    png_byte dispose_op , blend_op;
    //ImageFrame* frame = new ImageFrame;
    png_structp png_ptr =mPrivate->png_ptr;
    png_infop info_ptr=mPrivate->info_ptr;

    png_read_frame_head(png_ptr, info_ptr);
    uint8_t*frame_pixels;
    uint32_t frame_width,frame_height,frame_x,frame_y;
    png_get_next_frame_fcTL(png_ptr, info_ptr, &frame_width, &frame_height, &frame_x, &frame_y,
                    &delay_num, &delay_den, &dispose_op, &blend_op);
    //frame->disposalMethod = (dispose_op<<8)|blend_op;
    //frame->duration = delay_num * 1000 / (delay_den ? delay_den : 1000);// Convert to milliseconds
    frame_pixels = new uint8_t[frame_width * frame_height * 4];

    std::vector<png_bytep> row_pointers(frame_height);
    /*if(ifrm==0){
        memset(frame_pixels,0,frame_width * frame_height * 4);
    }else{
        switch(snap->disposalMethod>>8){
        case APNG_DISPOSE_OP_PREVIOUS:memcpy(frame_pixels,snap->pixels,frame->width*frame->height*4);break;
        case APNG_DISPOSE_OP_BACKGROUND:memset(frame_pixels,0,frame->width * frame->height * 4);break;
        case APNG_DISPOSE_OP_NONE:break;
        }
    }*/
    for (png_uint_32 y = 0; y < frame_height; ++y) {
        row_pointers[y] = frame_pixels + y* frame_width * 4;
    }
    png_read_image(png_ptr, row_pointers.data());
    /*if((dispose_op==APNG_DISPOSE_OP_PREVIOUS)&&((snap->disposalMethod>>8)!=APNG_DISPOSE_OP_PREVIOUS)){
        memcpy(snap->pixels,frame->pixels,frame->width * frame->height * 4);
    }*/
    LOGV("FRAME[%d](%d,%d,%d,%d)op=%d/%d",frameIndex,frame_x,frame_y,frame_width,frame_height,dispose_op,blend_op);
    //snap->disposalMethod=(dispose_op<<8)|blend_op;
    //mFrames.push_back(frame);
    return 0;
}

static void istream_png_reader(png_structp png_ptr, png_bytep png_data, png_size_t data_size){
    std::istream* is = (std::istream*)(png_get_io_ptr(png_ptr));
    is->read(reinterpret_cast<char*>(png_data), data_size);
}

int APNGDecoder::load() {
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        std::cerr << "Error creating read struct" << std::endl;
        return false;
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        std::cerr << "Error creating info struct" << std::endl;
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return false;
    }
    mPrivate->png_ptr = png_ptr;
    mPrivate->info_ptr= info_ptr;
    if (setjmp(png_jmpbuf(png_ptr))) {
        std::cerr << "Error during libpng init_io" << std::endl;
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return false;
    }

    png_set_read_fn(png_ptr,(void*)istream.get(),istream_png_reader);
    png_read_info(png_ptr, info_ptr);

    int bit_depth, color_type;
    png_get_IHDR(png_ptr, info_ptr,(uint32_t*)&mImageWidth, (uint32_t*)&mImageHeight, &bit_depth, &color_type, NULL, NULL, NULL);

    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr);
    }

    if (bit_depth == 16) {
        png_set_strip_16(png_ptr);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png_ptr);
    }

    png_set_bgr(png_ptr);
    png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
    png_read_update_info(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);

    mFrameCount = png_get_num_frames(png_ptr, info_ptr);
    return mFrameCount;
}
}/*namesapce*/
