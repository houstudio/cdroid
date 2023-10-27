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

int APNGDecoder::readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int& frameIndex) {
    if(!mFrames.empty()){
        ImageFrame*frm = mFrames[frameIndex];
#if 1
        const int dststride = image->get_stride();
        const int srcstride = frm->width*4;
        int u, v, al;
        const int bop= frm->disposalMethod&0xFF;
        uint8_t* dst = (image->get_data() + frm->y*dststride + frm->x*4);
        uint8_t* src = frm->pixels;
        for(int y=0;y<frm->height;y++){
            uint8_t*dp = dst;
            uint8_t*sp = src;
            for(int x=0;x<frm->width;x++,dp+=4,sp+=4){
               if((sp[3]==255)||(dp[3]==0)||(bop==APNG_BLEND_OP_SOURCE)||(frameIndex==0))*(uint32_t*)dp= *(uint32_t*)sp;
               else if(sp[3]!=0){
	               u = sp[3]*255;
	               v = (255-sp[3])*dp[3];
	               al = 255*255-(255-sp[3])*(255-dp[3]);
	               dp[0] = (sp[0]*u + dp[0]*v)/al;
	               dp[1] = (sp[1]*u + dp[1]*v)/al;
	               dp[2] = (sp[2]*u + dp[2]*v)/al;
	               dp[3] = al/255;
	           }
	        }
            dst += dststride;
            src += srcstride;
        }
#else
        Cairo::RefPtr<Cairo::Context>ctx = Cairo::Context::create(image);
        Cairo::RefPtr<Cairo::ImageSurface>frame=Cairo::ImageSurface::create(frm->pixels,Cairo::Surface::Format::ARGB32,int(frm->width),int(frm->height),int(frm->width*4));
        ctx->set_operator(((frm->disposalMethod&0xFF)&&(frameIndex))?Cairo::Context::Operator::OVER:Cairo::Context::Operator::SOURCE);
        ctx->set_source(frame,frm->x,frm->y);
        ctx->rectangle(frm->x,frm->y,frm->width,frm->height);
        ctx->fill();
#endif
        LOGV("FRAME[%d](%d,%d,%d,%d)disposalMethod=%x delay=%d",frameIndex,frm->x,frm->y,frm->width,frm->height,frm->disposalMethod,frm->duration);
        frameIndex = (frameIndex+1)%mFrameCount;
        return frm->duration;
    }
    return 0;
}


static void istream_png_reader(png_structp png_ptr, png_bytep png_data, png_size_t data_size){
    std::istream* is = (std::istream*)(png_get_io_ptr(png_ptr));
    is->read(reinterpret_cast<char*>(png_data), data_size);
};

int APNGDecoder::load(std::istream&istream) {
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

    if (setjmp(png_jmpbuf(png_ptr))) {
        std::cerr << "Error during libpng init_io" << std::endl;
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return false;
    }

    png_set_read_fn(png_ptr,(void*)&istream,istream_png_reader);
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
    mFrames.reserve(mFrameCount);
    uint32_t imgBytes = 0;
    ImageFrame*snap = new ImageFrame;
    snap->x = snap->y = 0;snap->disposalMethod = 0;
    snap->width = mImageWidth ; snap->height = mImageHeight;
    snap->pixels= new uint8_t[mImageWidth*mImageHeight*4];
    auto tm1=SystemClock::currentTimeMillis();
    for (png_uint_32 ifrm = 0; ifrm < mFrameCount; ++ifrm) {
        png_uint_16 delay_num , delay_den;
        png_byte dispose_op , blend_op;
        ImageFrame* frame = new ImageFrame;

        png_read_frame_head(png_ptr, info_ptr);
        png_get_next_frame_fcTL(png_ptr, info_ptr, &frame->width, &frame->height, &frame->x, &frame->y,
                        &delay_num, &delay_den, &dispose_op, &blend_op);
        frame->disposalMethod = (dispose_op<<8)|blend_op;
        frame->duration = delay_num * 1000 / (delay_den ? delay_den : 1000);// Convert to milliseconds
        frame->pixels = new uint8_t[frame->width * frame->height * 4];

        std::vector<png_bytep> row_pointers(frame->height);
        imgBytes += frame->width * frame->height * 4;
        if(ifrm==0){
            memset(frame->pixels,0,frame->width * frame->height * 4);
        }else{
            switch(snap->disposalMethod>>8){
            case APNG_DISPOSE_OP_PREVIOUS:memcpy(frame->pixels,snap->pixels,frame->width*frame->height*4);break;
            case APNG_DISPOSE_OP_BACKGROUND:memset(frame->pixels,0,frame->width * frame->height * 4);break;
            case APNG_DISPOSE_OP_NONE:break;
            }
        }
        for (png_uint_32 y = 0; y < frame->height; ++y) {
            row_pointers[y] = frame->pixels + y* frame->width * 4;
        }
        png_read_image(png_ptr, row_pointers.data());
        if((dispose_op==APNG_DISPOSE_OP_PREVIOUS)&&((snap->disposalMethod>>8)!=APNG_DISPOSE_OP_PREVIOUS)){
            memcpy(snap->pixels,frame->pixels,frame->width * frame->height * 4);
        }
        LOGV("FRAME[%d](%d,%d,%d,%d)op=%d/%d",ifrm,frame->x,frame->y,frame->width,frame->height,dispose_op,blend_op);
        snap->disposalMethod=(dispose_op<<8)|blend_op;
        mFrames.push_back(frame);
    }
    auto tm2=SystemClock::currentTimeMillis();
    LOGD("image(%dx%d)%d frames %d bytes color_type=%d/%d/%d time=%dms",mImageWidth,mImageHeight,mFrameCount,imgBytes,
		    color_type,PNG_COLOR_TYPE_RGB,PNG_COLOR_TYPE_RGB_ALPHA,tm2-tm1);
    delete []snap->pixels;
    delete snap;
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);	
    return mFrameCount;
}
}/*namesapce*/
