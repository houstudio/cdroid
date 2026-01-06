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
#include <string.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <core/color.h>
#include <png.h>
#include <fstream>
#ifdef PNG_APNG_SUPPORTED
#include <image-decoders/pngframesequence.h>

namespace cdroid {
//REF: https://gitee.com/suyimin1/APNG4Android
//https://gitee.com/z411500976/upng-js/blob/master/UPNG.js
//https://github.com/guoweilkd/lv_lib_apng/blob/master/lv_apng.c
////////////////////////////////////////////////////////////////////////////////
// Frame sequence
////////////////////////////////////////////////////////////////////////////////
static void pngmem_reader(png_structp png_ptr, png_bytep png_data, png_size_t data_size) {
    char** ppd = (char**)(png_get_io_ptr(png_ptr));
    memcpy(png_data,*ppd, data_size);
    *ppd += data_size;
}

PngFrameSequence::PngFrameSequence(std::istream&stream)
    :FrameSequence(stream), mLoopCount(1), mBgColor(COLOR_TRANSPARENT) {
    png_structp png_ptr;
    png_infop png_info;
    png_color_16p bg = nullptr;
    png_bytep trans_alpha = nullptr;
    Color ccBG(mBgColor);

    mStream.seekg(0,std::ios::end);
    mDataSize = mStream.tellg();
    mDataBytes= new uint8_t[mDataSize];
    mStream.seekg(0,std::ios::beg);
    mStream.read((char*)mDataBytes,mDataSize);

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    png_info= png_create_info_struct(png_ptr);
    char*pd = (char*)mDataBytes;
    png_set_read_fn(png_ptr,(void*)&pd,pngmem_reader);
    png_read_info(png_ptr, png_info);

    LOGD_IF(!png_get_valid(png_ptr, png_info, PNG_INFO_acTL),"Not a APNG");

    png_get_IHDR(png_ptr, png_info, (uint32_t*)&mImageWidth, (uint32_t*)&mImageHeight,nullptr,nullptr,nullptr,nullptr,nullptr);
    //&bit_depth, &color_type, &interlace_type, NULL, NULL);
    png_get_acTL(png_ptr, png_info, (uint32_t*)&mFrameCount, (uint32_t*)&mLoopCount);

    png_get_tRNS(png_ptr, png_info, &trans_alpha, nullptr,nullptr);
    if(trans_alpha)ccBG.setAlpha(float(*trans_alpha)/65535.f);
    LOGD("trans_alpha=%p %d",trans_alpha,(trans_alpha?*trans_alpha:0));
    png_get_bKGD(png_ptr, png_info, &bg);

    if (bg) {
        switch (png_get_color_type(png_ptr, png_info)) {
        case PNG_COLOR_TYPE_RGB:
            ccBG.setRed(float(bg->red)/65535.f);
            ccBG.setGreen(float(bg->red)/65535.f);
            ccBG.setBlue(float(bg->red)/65535.f);
            break;
        case PNG_COLOR_TYPE_PALETTE:
            // Handle palette-based background color if needed
            break;
        case PNG_COLOR_TYPE_RGB_ALPHA:
            ccBG.setRed(float(bg->red)/65535.f);
            ccBG.setGreen(float(bg->red)/65535.f);
            ccBG.setBlue(float(bg->red)/65535.f);
            if(trans_alpha==nullptr)
                ccBG.setAlpha(0.f);
            LOGD("Background RGB(%d, %d, %d) with alpha %d", bg->red, bg->green, bg->blue,0);
            break;
        default: LOGD("Background color type not supported");  break;
        }
        mBgColor = ccBG.toArgb();
        LOGD("Background RGB(%d, %d, %d)", bg->red, bg->green, bg->blue);
    }
    LOGD("mDataSize=%d image %dx%dx%d background=%x",mDataSize,mImageWidth,mImageHeight,mFrameCount,mBgColor);

    png_destroy_read_struct(&png_ptr, &png_info, NULL);
}

PngFrameSequence::~PngFrameSequence() {
    delete[] mDataBytes;
}

int PngFrameSequence::getWidth() const {
    return mImageWidth;
}

int PngFrameSequence::getHeight() const {
    return mImageHeight;
}

bool PngFrameSequence::isOpaque() const {
    return 0;//(mBgColor & COLOR_8888_ALPHA_MASK) == COLOR_8888_ALPHA_MASK;
}

int PngFrameSequence::getFrameCount() const {
    return mFrameCount;
}

FrameSequenceState* PngFrameSequence::createState() const {
    return new PngFrameSequenceState(*this);
}


////////////////////////////////////////////////////////////////////////////////
// Frame sequence state
////////////////////////////////////////////////////////////////////////////////

PngFrameSequence::PngFrameSequenceState::PngFrameSequenceState(const PngFrameSequence& frameSequence) :
    mFrameSequence(frameSequence),mFrameIndex(0){
    const int width = mFrameSequence.getWidth();
    const int height= mFrameSequence.getHeight();

    LOGD("size=%dx%dx%d",width,height,mFrameSequence.getFrameCount());
    png_ptr = nullptr;
    png_info= nullptr;
    mFrame = new uint8_t[width*height*4];
    mBuffer = new uint8_t[width*height*4];
    mPrevFrame = new uint8_t[width*height*4];
}

void PngFrameSequence::PngFrameSequenceState::resetPngIO(){
    if((png_ptr==nullptr)||(png_info==nullptr)){
        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        png_info= png_create_info_struct(png_ptr);
    }
    mFrameIndex = 0;
    mBytesAt = mFrameSequence.mDataBytes;
    png_set_read_fn(png_ptr,(void*)&mBytesAt,pngmem_reader);

    png_set_sig_bytes(png_ptr, 0);
    png_read_info(png_ptr, png_info);

    png_set_expand(png_ptr);
    png_set_strip_16(png_ptr);
    png_set_gray_to_rgb(png_ptr);
    png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
    png_set_bgr(png_ptr);
    png_set_interlace_handling(png_ptr);

    //PNGDecoder::setGamma(nullptr,png_ptr, png_info);

    png_read_update_info(png_ptr, png_info);
}

PngFrameSequence::PngFrameSequenceState::~PngFrameSequenceState() {
    png_destroy_read_struct(&png_ptr, &png_info, nullptr);
    delete [] mFrame;
    delete [] mBuffer;
    delete [] mPrevFrame;
}

static void composeFrame(uint8_t*dst,uint8_t*src,uint32_t stride, unsigned char bop, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    uint32_t  i, j;
    int u, v, al;
    for (j=0; j<h; j++) {
        uint8_t * sp = src+j*stride;//rows_src[j];
        uint8_t * dp = dst+(j+y)*stride+x*4;//rows_dst[j+y] + x*4;

        if (bop == 0){memcpy(dp, sp, w*4);continue;}
        for (i=0; i<w; i++, sp+=4, dp+=4) {
            if (sp[3] == 255)
                memcpy(dp, sp, 4);
            else if (sp[3] != 0) {
                if (dp[3] != 0) {
                    u = sp[3]*255;
                    v = (255-sp[3])*dp[3];
                    al = u + v;
                    dp[0] = (sp[0]*u + dp[0]*v)/al;
                    dp[1] = (sp[1]*u + dp[1]*v)/al;
                    dp[2] = (sp[2]*u + dp[2]*v)/al;
                    dp[3] = al/255;
                } else
                    memcpy(dp, sp, 4);
            }
        }
    }
}

static void fillFrame(uint8_t*dst,uint32_t stride, uint32_t x, uint32_t y, uint32_t w, uint32_t h,uint32_t color) {
    for (uint32_t j=0; j<h; j++) {
        uint8_t * dp = dst+(j+y)*stride+x*4;//rows_dst[j+y] + x*4;

        for (uint32_t i=0; i<w; i++,dp+=4) {
           *((uint32_t*)dp) = color;
        }
    }
}

long PngFrameSequence::PngFrameSequenceState::drawFrame(int frameNr,
        uint32_t* outputPtr, int outputPixelStride, int previousFrameNr) {
    const int width = mFrameSequence.getWidth();
    const int height = mFrameSequence.getHeight();
    uint32_t frmX,frmY,frmWidth,frmHeight,frmDelay;
    uint16_t delay_num,delay_den;
    uint8_t frmDop,frmBop;

    uint32_t first = png_get_first_frame_is_hidden(png_ptr, png_info) ? 1 : 0;

    std::vector<png_bytep> rows(height);

    for(int i=0;i<height;i++){
        rows[i]= mBuffer + i*width*4;
    }

    if(frameNr==0)
       resetPngIO();
    LOGE_IF(frameNr!=mFrameIndex,"Error FrameSequence %d should be %d",frameNr,mFrameIndex);

    png_read_frame_head(png_ptr, png_info);
    png_get_next_frame_fcTL(png_ptr,png_info,&frmWidth,&frmHeight,&frmX,&frmY,
                &delay_num,&delay_den,&frmDop,&frmBop);
    if(delay_den==0)delay_den = 100;
    frmDelay = (delay_num*1000)/delay_den;
    if(frameNr==first){/*The first frame doesn't have an fcTL so it's expected to be hidden, but we'll extract it anyway*/
        frmBop = PNG_BLEND_OP_SOURCE;
        if(frmDop==PNG_DISPOSE_OP_PREVIOUS)
            frmDop = PNG_DISPOSE_OP_NONE;
    }

    LOGV("frame %d at(%d,%d,%d,%d) op=%d/%d readPos=%d delay=%d",mFrameIndex,frmX,frmY,frmWidth,frmHeight,frmDop,frmBop,mBytesAt-mFrameSequence.mDataBytes,frmDelay);

    png_read_image(png_ptr, rows.data());
    if(frmDop==PNG_DISPOSE_OP_PREVIOUS)
        memcpy(mPrevFrame,mFrame,width*height*4);

    composeFrame(mFrame,mBuffer,width*4,frmBop,frmX,frmY,frmWidth,frmHeight);
    memcpy(outputPtr,mFrame,width*height*4);
    switch(frmDop){
    case PNG_DISPOSE_OP_NONE:/*Nothing* TODO*/ break;
    case PNG_DISPOSE_OP_BACKGROUND:
        fillFrame(mFrame,width*4,frmX,frmY,frmWidth,frmHeight,mFrameSequence.mBgColor);
        break;
    case PNG_DISPOSE_OP_PREVIOUS:
        memcpy(mFrame,mPrevFrame,width*height*4);
        break;
    }
    mFrameIndex++;
    return frmDelay;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Registry
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PngFrameSequence::isPNG(const uint8_t* header,uint32_t head_size) {
    static constexpr const char*PNG_STAMP="\x89\x50\x4E\x47\x0D\x0A\x1A\x0A";
    return !memcmp(PNG_STAMP, header, PNG_HEADER_SIZE);
}

}/*endof namespace*/

#endif /*PNG_APNG_SUPPORTED*/

