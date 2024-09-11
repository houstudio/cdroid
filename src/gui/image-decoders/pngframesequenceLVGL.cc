/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <core/color.h>
#include <png.h>
#ifdef PNG_APNG_SUPPORTED
#include <image-decoders/pngframesequenceLVGL.h>

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

PngFrameSequence::PngFrameSequence(std::istream* stream) :
    mLoopCount(1), mBgColor(COLOR_TRANSPARENT) {
    png_structp png_ptr;
    png_infop png_info;
    png_color_16p bg = nullptr;
    png_bytep trans_alpha = nullptr;
    Color ccBG(mBgColor);

    stream->seekg(0,std::ios::end);
    mDataSize = stream->tellg();
    mDataBytes= new uint8_t[mDataSize];
    stream->seekg(0,std::ios::beg);
    stream->read((char*)mDataBytes,mDataSize);

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
    mBaseBuffer = new uint8_t[width*height*4];
    mRenderBuffer = nullptr;
    mCurrBase = nullptr;
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
    png_read_update_info(png_ptr, png_info);
}

PngFrameSequence::PngFrameSequenceState::~PngFrameSequenceState() {
    png_destroy_read_struct(&png_ptr, &png_info, nullptr);
    delete [] mBaseBuffer;
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

void  PngFrameSequence::PngFrameSequenceState::blend2Render(uint32_t x, uint32_t y, uint32_t w, uint32_t h){
    uint8_t *prender = mRenderBuffer;
    uint8_t *pbase = mBaseBuffer;
    const int width  = mFrameSequence.getWidth();
    const int height = mFrameSequence.getHeight();
    const int row_size = width * 4;
    int opa_i = 3;

    if(mCurrBase == pbase){
        memcpy(prender,pbase, y * row_size);

        int offset = (y + h) * row_size;
        memcpy(prender + offset,pbase + offset, (height - y - h) * row_size);

        int end = y + h;
        for(int i = y; i < end; i ++){
            memcpy(&prender[i * row_size],&pbase[i * row_size], x*4);
            memcpy(&prender[i * row_size + (x + w)*4],&pbase[i * row_size + (x + w)*4], (width - x - w)*4);
        }
    }

    int u, v, al;
    int i_end = y + h;
    int k_end = (x + w) * 4;
    for(int i = y; i < i_end; i ++){
        int row_offset = i * row_size;
        for(int k = x*4; k < k_end; k += 4){
            if(prender[row_offset + k + opa_i] == 255){//什么都不用干
            }
            else if(prender[row_offset + k + opa_i] == 0){//用base填充
                memcpy(&prender[row_offset + k],&pbase[row_offset + k],4);
            }
            else if(1){
                int index = row_offset + k;
                if(pbase[index + opa_i] != 0){
                    u = prender[index + opa_i] * 255;
                    v = (255 - prender[index + opa_i]) * pbase[index + opa_i];
                    al = u + v;
                    prender[index + 0] = (prender[index + 0] * u + pbase[index + 0] * v) / al;
                    prender[index + 1] = (prender[index + 1] * u + pbase[index + 1] * v) / al;
                    prender[index + 2] = (prender[index + 2] * u + pbase[index + 2] * v) / al;
                    prender[index + 3] = al / 255;
                }
            }
        }
    }
}

void  PngFrameSequence::PngFrameSequenceState::fill2Render(uint32_t x, uint32_t y, uint32_t w, uint32_t h){
    uint8_t *prender =mRenderBuffer;
    uint8_t *pbase = mBaseBuffer;
    const int width = mFrameSequence.getWidth();
    const int height= mFrameSequence.getHeight();
    const int row_size = width * 4;

    if(mCurrBase == pbase){
        memcpy(prender,pbase, y * row_size);

        int offset = (y + h) * row_size;
        memcpy(prender + offset,pbase + offset, (height - y - h) * row_size);

        int end = y + h;
        for(int i = y; i < end; i ++){
            memcpy(&prender[i * row_size],&pbase[i * row_size], x*4);
            memcpy(&prender[i * row_size + (x + w)*4],&pbase[i * row_size + (x + w)*4], (width - x - w)*4);
        }
    }
}

void PngFrameSequence::PngFrameSequenceState::copyArea2Base(uint32_t x, uint32_t y, uint32_t w, uint32_t h){
    uint8_t *prender = mRenderBuffer;
    uint8_t *pbase = mBaseBuffer;
    const uint32_t width  = mFrameSequence.getWidth();
    const int row_size = width * 4;

    int end = y + h;
    for(int i = y; i < end; i ++){
        memcpy(&pbase[i * row_size + x*4],&prender[i * row_size + x*4], w*4);
    }
}

void PngFrameSequence::PngFrameSequenceState::clearBaseArea(uint32_t x, uint32_t y, uint32_t w, uint32_t h){
    uint8_t *prender = mRenderBuffer;
    uint8_t *pbase = mBaseBuffer;
    const int width  = mFrameSequence.getWidth();
    const int height = mFrameSequence.getHeight();
    const int row_size = width * 4;

    memcpy(pbase, prender, height * width * 4);

    int end = y + h;
    for(int i = y; i < end; i ++){
        memset(&pbase[i * row_size + x*4], 0x00, w*4);
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

static uint32_t frmX1, frmY1, frmWidth1 ,frmHeight1;
long PngFrameSequence::PngFrameSequenceState::drawFrame(int frameNr,
        uint32_t* outputPtr, int outputPixelStride, int previousFrameNr) {
    const int width = mFrameSequence.getWidth();
    const int height = mFrameSequence.getHeight();
    uint32_t frmX, frmY, frmWidth ,frmHeight,frmDelay;
    uint16_t delay_num, delay_den,delay;
    uint8_t frmDop,frmBop;
    uint8_t *pbase = mBaseBuffer;
    uint8_t *prender = (uint8_t*)outputPtr;


    uint32_t first = png_get_first_frame_is_hidden(png_ptr, png_info) ? 1 : 0;

    std::vector<png_bytep> rows(height);

    if(frameNr==0){
        mRenderBuffer =(uint8_t*)outputPtr;
        mCurrBase = (uint8_t*)outputPtr;
        resetPngIO();
    }

    LOGE_IF(frameNr!=mFrameIndex,"Error FrameSequence %d should be %d",frameNr,mFrameIndex);

    png_read_frame_head(png_ptr, png_info);
    png_get_next_frame_fcTL(png_ptr,png_info,&frmWidth,&frmHeight,&frmX,&frmY,
                &delay_num,&delay_den,&frmDop,&frmBop);
    if(delay_den==0)delay_den = 100;
    frmDelay = (delay_num*1000)/delay_den;
    if(frameNr==0){/*The first frame doesn't have an fcTL so it's expected to be hidden, but we'll extract it anyway*/
        frmBop = PNG_BLEND_OP_SOURCE;
        if(frmDop==PNG_DISPOSE_OP_PREVIOUS)
            frmDop = PNG_DISPOSE_OP_BACKGROUND;
    }

    LOGD("frame %d at(%d,%d,%d,%d) op=%d/%d",mFrameIndex,frmX,frmY,frmWidth,frmHeight,frmDop,frmBop,mBytesAt-mFrameSequence.mDataBytes);
    
    for(int i=0;i<height;i++){
        rows[i]= prender + (i+frmY)*width*4+frmX*4;
    }

    if(mFrameIndex==0)mCurrBase = prender;
    if(mCurrBase ==prender){
        if(frmDop==PNG_DISPOSE_OP_PREVIOUS){
            memcpy(pbase,prender,width*height*4);
            mCurrBase = pbase;
        }else if(frmBop==PNG_BLEND_OP_OVER){
            copyArea2Base(frmX,frmY,frmWidth,frmHeight);
        }
    }
    png_read_image(png_ptr, rows.data());

    if(frmBop==PNG_BLEND_OP_OVER){
        blend2Render(frmX,frmY,frmWidth,frmHeight);
    }else{
        fill2Render(frmX,frmY,frmWidth,frmHeight);
    }

    if(frmDop==PNG_DISPOSE_OP_BACKGROUND){
        clearBaseArea(frmX,frmY,frmWidth,frmHeight);
        mCurrBase = pbase;
    }else{
        mCurrBase = prender;
    }
    mFrameIndex++;

    return frmDelay;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Registry
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool acceptsBuffers() {
    return false;
}

static bool isPng(void* header, int header_size) {
    static constexpr const char*PNG_STAMP="\x89\x50\x4E\x47\x0D\x0A\x1A\x0A";
    constexpr int PNG_STAMP_LEN = strlen(PNG_STAMP);
    return !memcmp(PNG_STAMP, header, PNG_STAMP_LEN)
           || !memcmp(PNG_STAMP, header, PNG_STAMP_LEN)
           || !memcmp(PNG_STAMP, header, PNG_STAMP_LEN);
}

static FrameSequence* createFramesequence(std::istream* stream) {
    return new cdroid::PngFrameSequence(stream);
}

static FrameSequence::RegistryEntry gEntry = {
    8/*PNG_STAMP_LEN*/,
    isPng,
    createFramesequence,
    //NULL,
    acceptsBuffers,
};
static FrameSequence::Registry gRegister(gEntry);

}/*endof namespace*/

#endif /*PNG_APNG_SUPPORTED*/

