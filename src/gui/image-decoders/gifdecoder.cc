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
#include <cairomm/surface.h>
#include <image-decoders/imagedecoder.h>
#include <gui_features.h>
#include <cdlog.h>

#if defined(ENABLE_GIF)&&ENABLE_GIF
#include <gif_lib.h>

namespace cdroid{

struct PRIVATE{
    GifFileType*gif;
}GIFPrivate;

GIFDecoder::GIFDecoder(std::istream&stream):ImageDecoder(stream){
    mPrivate = new PRIVATE;
    mPrivate->gif=nullptr;
}

GIFDecoder::~GIFDecoder(){
    DGifCloseFile(mPrivate->gif,nullptr);
    delete mPrivate;
}

#define  ARGB(a, r, g, b) ( uint32_t((a) & 0xff) << 24 ) | ( uint32_t((r) & 0xff) << 16 ) | ( uint32_t((g) & 0xff) << 8 ) | ((b) & 0xff)
#define  DISPOSE(ext) (((ext)->Bytes[0] & 0x1c) >> 2)
#define  TRANS_INDEX(ext) ((ext)->Bytes[3])
#define  TRANSPARENCY(ext) ((ext)->Bytes[0] & 1)
#define  DELAY(ext) (10*((ext)->Bytes[2] << 8 | (ext)->Bytes[1]))

static int GIFRead(GifFileType *gifFile, GifByteType *buff, int rdlen){
    std::istream*is=(std::istream*)gifFile->UserData;
    is->read((char*)buff,rdlen);
    return (int)is->gcount();
}

static int gifDrawFrame(GifFileType*gif,int current_frame,size_t pxstride,uint8_t *pixels,bool force_DISPOSE_1);

bool GIFDecoder::decodeSize(){
    int err;
    GifFileType*gifFileType = DGifOpen(&mStream,GIFRead,&err);
    LOGE_IF(gifFileType==nullptr,"git load failed");
    if(gifFileType==nullptr)return false;

    mPrivate->gif = gifFileType;
    mImageWidth = gifFileType->SWidth;
    mImageHeight= gifFileType->SHeight;
    mFrameCount = gifFileType->ImageCount;
    LOGD("GIF %d frames loaded size(%dx%d)",gifFileType->ImageCount,mImageWidth,mImageHeight);
    return true;
}

Cairo::RefPtr<Cairo::ImageSurface>  GIFDecoder::decode(float scale,void*targetProfile){
    decodeSize();
    DGifSlurp(mPrivate->gif);
    Cairo::RefPtr<Cairo::ImageSurface>img;
    img = Cairo::ImageSurface::create(Cairo::ImageSurface::Format::ARGB32,mImageWidth,mImageHeight);
    gifDrawFrame(mPrivate->gif,0,img->get_stride(),img->get_data(),false);
    return img;
}

bool GIFDecoder::isGIF(const uint8_t* contents,uint32_t header_size){
    return !memcmp((const char*)contents, "GIF87a", 6) || !memcmp((const char*)contents, "GIF89a", 6);
}

static int gifDrawFrame(GifFileType*gif,int current_frame,size_t pxstride,uint8_t *pixels,bool force_DISPOSE_1) {
    GifColorType *bg;
    GifColorType *color;
    SavedImage *frame;
    ExtensionBlock *ext = nullptr;
    GifImageDesc *frameInfo;
    ColorMapObject *colorMap;
    uint32_t *line;
    int width, height, x, y, j;
    uint8_t *px = (uint8_t*)pixels;
    width = gif->SWidth;
    height = gif->SHeight;
    frame = &(gif->SavedImages[current_frame]);
    frameInfo = &(frame->ImageDesc);
    colorMap = frameInfo->ColorMap?frameInfo->ColorMap:gif->SColorMap;
	
    bg = &colorMap->Colors[gif->SBackGroundColor];
    for (j = 0; j < frame->ExtensionBlockCount; j++) {
        if (frame->ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {
            ext = &(frame->ExtensionBlocks[j]);
            break;
        }
    }
    // For DISPOSE = 1, we assume its been drawn
    if (ext && DISPOSE(ext) == 1 && force_DISPOSE_1 && current_frame > 0) {
        current_frame = current_frame - 1;
        gifDrawFrame(gif,current_frame,pxstride, pixels, true);
    } else if (ext && DISPOSE(ext) == 2 && bg) {
        for (y = 0; y < height; y++) {
            line = (uint32_t *) px;
            for (x = 0; x < width; x++) {
                line[x] = ARGB(255, bg->Red, bg->Green, bg->Blue);
            }
            px = (px + pxstride);
        }
        current_frame=(current_frame+1)%gif->ImageCount;
    } else if (ext && DISPOSE(ext) == 3 && current_frame > 1) {
        current_frame = current_frame - 2;
        gifDrawFrame(gif,current_frame,pxstride, pixels, true);
    }else current_frame=(current_frame+1)%gif->ImageCount;
    px = (uint8_t*)pixels;

    const bool isTransparent   = TRANSPARENCY(ext);
    const int transparentIndex = TRANS_INDEX(ext);
    if (frameInfo->Interlace) {
        int n = 0, inc = 8, p = 0,loc=0;
        px = (px + pxstride * frameInfo->Top);
        for (y = frameInfo->Top; y < frameInfo->Top + frameInfo->Height; y++) {
            for (x = frameInfo->Left; x < frameInfo->Left + frameInfo->Width; x++) {
                loc = (y - frameInfo->Top) * frameInfo->Width + (x - frameInfo->Left);
                if (ext && isTransparent && (frame->RasterBits[loc]==transparentIndex)) {
                    continue;
                }
                color = (ext && frame->RasterBits[loc] == transparentIndex) ? bg
                        : &colorMap->Colors[frame->RasterBits[loc]];
                if (color)
                    line[x] = ARGB(255, color->Red, color->Green, color->Blue);
            }
            px = (px + pxstride * inc);
            n += inc;
            if (n >= frameInfo->Height) {
                n = 0;
                switch (p) {
                case 0:
                    px = (pixels + pxstride * (4 + frameInfo->Top));
                    inc = 8;     p++;     break;
                case 1:
                    px = (pixels + pxstride * (2 + frameInfo->Top));
                    inc = 4;     p++;     break;
                case 2:
                    px = (pixels + pxstride * (1 + frameInfo->Top));
                    inc = 2;     p++;     break;
                }
            }
        }
    } else {
        px = ( px + pxstride * frameInfo->Top);
        for (y = frameInfo->Top; y < frameInfo->Top + frameInfo->Height; y++) {
            line = (uint32_t *) px;
            for (x = frameInfo->Left; x < frameInfo->Left + frameInfo->Width; x++) {
                int loc = (y - frameInfo->Top) * frameInfo->Width + (x - frameInfo->Left);
                if (ext && isTransparent &&(frame->RasterBits[loc] == transparentIndex)) {
                    continue;
                }
                color = (ext && frame->RasterBits[loc] == transparentIndex) ? bg
                        : &colorMap->Colors[frame->RasterBits[loc]];
                if (color)
                    line[x] = ARGB(255, color->Red, color->Green, color->Blue);
            }
            px +=pxstride;
        }
    }
    return DELAY(ext);
}
}/*endof namespace*/
#endif/*defined(ENABLE_GIF)&&ENABLE_GIF*/
