#include <core/context.h>
#include <cairomm/surface.h>
#include <image-decoders/imagedecoder.h>
#include <gui/gui_features.h>
#include <gif_lib.h>
#include <cdlog.h>

namespace cdroid{

struct PRIVATE{
    std::vector<int>delays;
    GifFileType*gif;
}GIFPrivate;

GIFDecoder::GIFDecoder(std::istream&stm):ImageDecoder(stm){
    mPrivate = new PRIVATE;
    mPrivate->gif=nullptr;
}

GIFDecoder::~GIFDecoder(){
    DGifCloseFile(mPrivate->gif,nullptr);
    delete mPrivate;
}

#define  ARGB(a, r, g, b) ( ((a) & 0xff) << 24 ) | ( ((r) & 0xff) << 16 ) | ( ((g) & 0xff) << 8 ) | ((b) & 0xff)
#define  DISPOSE(ext) (((ext)->Bytes[0] & 0x1c) >> 2)
#define  TRANS_INDEX(ext) ((ext)->Bytes[3])
#define  TRANSPARENCY(ext) ((ext)->Bytes[0] & 1)
#define  DELAY(ext) (10*((ext)->Bytes[2] << 8 | (ext)->Bytes[1]))

static int GIFRead(GifFileType *gifFile, GifByteType *buff, int rdlen){
    std::istream*is=(std::istream*)gifFile->UserData;
    is->read((char*)buff,rdlen);
    return is->gcount();
}

static int gifDrawFrame(GifFileType*gif,int current_frame,size_t pxstride,uint8_t *pixels,bool force_DISPOSE_1);

int GIFDecoder::decode(bool sizeOnly){
    int err;
    GifFileType*gifFileType = DGifOpen(istream,GIFRead,&err);
    LOGE_IF(gifFileType==nullptr,"git load failed");
    if(gifFileType==nullptr)return 0;
    DGifSlurp(gifFileType);

    mFrameCount = gifFileType->ImageCount;
    mPrivate->gif = gifFileType;
    mImageWidth = gifFileType->SWidth;
    mImageHeight= gifFileType->SHeight;
    LOGD("GIF %d frames loaded size(%dx%d)",mFrameCount,mImageWidth,mImageHeight);
    return mFrameCount;
}

int GIFDecoder::readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int frameIndex){
    const int duration = gifDrawFrame(mPrivate->gif,frameIndex,image->get_stride(),image->get_data(),false);
    if(frameIndex>=mPrivate->delays.size()){
        mPrivate->delays.push_back(duration);
    }
    return duration;
}

int GIFDecoder::getFrameDuration(int frameIndex)const{
    return mPrivate->delays.at(frameIndex);
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
