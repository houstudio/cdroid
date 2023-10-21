#include <core/context.h>
#include <cairomm/surface.h>
#include <drawables/imagereader.h>
#include <gui/gui_features.h>

namespace cdroid{

ImageReader::ImageReader(){
    mImageWidth = -1;
    mImageHeight= -1;
    mFrameCount =0;
    mPrivate = nullptr;
}

ImageReader::~ImageReader(){
}

int ImageReader::getFrameCount()const{
    return mFrameCount;
}

int ImageReader::getWidth()const{
    return mImageWidth;
}

int ImageReader::getHeight()const{
    return mImageHeight;
}

GIFReader::GIFReader(){

}

#ifdef ENABLE_GIF
#include <gif_lib.h>
static int GIFRead(GifFileType *gifFile, GifByteType *buff, int rdlen){
    std::istream*is=(std::istream*)gifFile->UserData;
    is->read((char*)buff,rdlen);
    return is->gcount();
}
static int gifDrawFrame(GifFileType*gif,int&current_frame,size_t pxstride,uint8_t *pixels,bool force_DISPOSE_1);
#endif

GIFReader::~GIFReader(){
#ifdef ENABLE_GIF
    DGifCloseFile((GifFileType*)mPrivate,nullptr);
#endif
}


bool GIFReader::load(std::istream&is){
    int err;
#ifdef ENABLE_GIF
    GifFileType*gifFileType = DGifOpen(&is,GIFRead,&err);
    LOGE_IF(gifFileType==nullptr,"git load failed");
    if(gifFileType==nullptr)return 0;
    DGifSlurp(gifFileType);

    mFrameCount = gifFileType->ImageCount;
    mPrivate = gifFileType;
    mImageWidth = gifFileType->SWidth;
    mImageHeight= gifFileType->SHeight;
    LOGD("GIF %d frames loaded size(%dx%d)",mFrameCount,mImageWidth,mImageHeight);
#endif
    return true;
}


int GIFReader::readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int& frameIndex){
#ifdef ENABLE_GIF
    return gifDrawFrame((GifFileType*)mPrivate,frameIndex,image->get_stride(),image->get_data(),false);
#else
    return -1;
#endif
}

#ifdef ENABLE_GIF

#define  ARGB(a, r, g, b) ( ((a) & 0xff) << 24 ) | ( ((r) & 0xff) << 16 ) | ( ((g) & 0xff) << 8 ) | ((b) & 0xff)

#define  DISPOSE(ext) (((ext)->Bytes[0] & 0x1c) >> 2)
#define  TRANS_INDEX(ext) ((ext)->Bytes[3])
#define  TRANSPARENCY(ext) ((ext)->Bytes[0] & 1)
#define  DELAY(ext) (10*((ext)->Bytes[2] << 8 | (ext)->Bytes[1]))

static int gifDrawFrame(GifFileType*gif,int&current_frame,size_t pxstride,uint8_t *pixels,bool force_DISPOSE_1) {
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
    if (frameInfo->ColorMap) {
        colorMap = frameInfo->ColorMap;
    } else {
        colorMap = gif->SColorMap;
    }

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
    if (frameInfo->Interlace) {
        int n = 0, inc = 8, p = 0,loc=0;
        px = (px + pxstride * frameInfo->Top);
        for (y = frameInfo->Top; y < frameInfo->Top + frameInfo->Height; y++) {
            for (x = frameInfo->Left; x < frameInfo->Left + frameInfo->Width; x++) {
                loc = (y - frameInfo->Top) * frameInfo->Width + (x - frameInfo->Left);
                if (ext && frame->RasterBits[loc] == TRANS_INDEX(ext) && TRANSPARENCY(ext)) {
                    continue;
                }
                color = (ext && frame->RasterBits[loc] == TRANS_INDEX(ext)) ? bg
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
                if (ext && frame->RasterBits[loc] == TRANS_INDEX(ext) && TRANSPARENCY(ext)) {
                    continue;
                }
                color = (ext && frame->RasterBits[loc] == TRANS_INDEX(ext)) ? bg
                        : &colorMap->Colors[frame->RasterBits[loc]];
                if (color)
                    line[x] = ARGB(255, color->Red, color->Green, color->Blue);
            }
            px +=pxstride;
        }
    }
    return DELAY(ext);
}
#endif
///////////////////////////////////////////////////////////////////////////

APNGReader::APNGReader(){
}

APNGReader::~APNGReader(){
}

bool APNGReader::load(std::istream&){
    return true;
}

int APNGReader::readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int& frameIndex){
   return 0;
}
}/*endof namespace*/
