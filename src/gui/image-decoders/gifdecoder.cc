#include <core/context.h>
#include <cairomm/surface.h>
#include <image-decoders/imagedecoder.h>
#include <gui/gui_features.h>
#include <gif_lib.h>
#include <cdlog.h>

namespace cdroid{


GIFDecoder::GIFDecoder(){
}

static int GIFRead(GifFileType *gifFile, GifByteType *buff, int rdlen){
    std::istream*is=(std::istream*)gifFile->UserData;
    is->read((char*)buff,rdlen);
    return is->gcount();
}
static int gifDrawFrame(GifFileType*gif,int&current_frame,size_t pxstride,uint8_t *pixels,bool force_DISPOSE_1);

GIFDecoder::~GIFDecoder(){
    DGifCloseFile((GifFileType*)mPrivate,nullptr);
}

#define  ARGB(a, r, g, b) ( ((a) & 0xff) << 24 ) | ( ((r) & 0xff) << 16 ) | ( ((g) & 0xff) << 8 ) | ((b) & 0xff)
#define  DISPOSE(ext) (((ext)->Bytes[0] & 0x1c) >> 2)
#define  TRANS_INDEX(ext) ((ext)->Bytes[3])
#define  TRANSPARENCY(ext) ((ext)->Bytes[0] & 1)
#define  DELAY(ext) (10*((ext)->Bytes[2] << 8 | (ext)->Bytes[1]))

#if 10
void GIFDecoder::setFramePixels(int frameIndex){
    int x,y;
    ImageFrame*frame = mFrames[frameIndex];
    const int pxstride = frame->width*4;
    GifFileType*gif = (GifFileType*)mPrivate;
    SavedImage *gifFrame = &gif->SavedImages[frameIndex];
    const GifImageDesc *frameInfo=&(gifFrame->ImageDesc);
    ColorMapObject *colorMap=(frameInfo->ColorMap?frameInfo->ColorMap:gif->SColorMap);
    GifColorType* bg = &colorMap->Colors[gif->SBackGroundColor];
    GifColorType* color = nullptr;
    ExtensionBlock *ext = nullptr;
    uint8_t* pixels= frame->pixels;
    uint32_t* line = nullptr;
    for (int j = 0; j < gifFrame->ExtensionBlockCount; j++) {
        if (gifFrame->ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {
            ext = &(gifFrame->ExtensionBlocks[j]);
            break;
        }
    }
    frame->duration = DELAY(ext);
    frame->disposalMethod = DISPOSE(ext);
        // For DISPOSE = 1, we assume its been drawn
    if (ext && DISPOSE(ext) == 1 /*&& force_DISPOSE_1*/ && frameIndex > 0) {
        //gifDrawFrame(gif,frameIndex-1,pxstride, pixels, true);
    } else if (ext && DISPOSE(ext) == 2 && bg) {
        uint8_t*px= frame->pixels;
        for (y = 0; y < frame->height; y++) {
            line = (uint32_t *) px;
            for (x = 0; x < frame->width; x++) {
                line[x] = ARGB(255, bg->Red, bg->Green, bg->Blue);
            }
            px = (px + pxstride);
        }
        //current_frame=(current_frame+1)%gif->ImageCount;
    } else if (ext && DISPOSE(ext) == 3 && frameIndex > 1) {
        //gifDrawFrame(gif,frameIndex-2,pxstride, pixels, true);
    }//else current_frame=(current_frame+1)%gif->ImageCount;
    const bool isTransparent   = TRANSPARENCY(ext);
    const int transparentIndex = TRANS_INDEX(ext);
    if (frameInfo->Interlace) {
        int n = 0, inc = 8, p = 0,loc=0;
        uint8_t* px = frame->pixels;
        for (y = 0; y <  frameInfo->Height; y++) {
            for (x = 0; x < frameInfo->Width; x++) {
                loc = y * frameInfo->Width + x;
                if (ext && isTransparent && (gifFrame->RasterBits[loc] == transparentIndex)) {
                    continue;
                }
                color = (ext && gifFrame->RasterBits[loc] == TRANS_INDEX(ext)) ? bg
                        : &colorMap->Colors[gifFrame->RasterBits[loc]];
                if (color)
                    line[x] = ARGB(255, color->Red, color->Green, color->Blue);
            }
            px = (px + pxstride * inc);
            n += inc;
            if (n >= frameInfo->Height) {
                n = 0;
                switch (p) {
                case 0:
                    px = (pixels + pxstride * (4/*+ frameInfo->Top*/));
                    inc = 8;     p++;     break;
                case 1:
                    px = (pixels + pxstride * (2/*+ frameInfo->Top*/));
                    inc = 4;     p++;     break;
                case 2:
                    px = (pixels + pxstride * (1/*+ frameInfo->Top*/));
                    inc = 2;     p++;     break;
                }
            }
        }
    } else {
        uint8_t* px = frame->pixels;//( px + pxstride * frameInfo->Top);
        for (y = 0; y < frameInfo->Height; y++) {
            line = (uint32_t *) px;
            for (x = 0; x < frameInfo->Width; x++) {
                int loc = y * frameInfo->Width + x;
                if (ext && isTransparent && (gifFrame->RasterBits[loc] == transparentIndex)) {
                    continue;
                }
                color = (ext && gifFrame->RasterBits[loc] == TRANS_INDEX(ext)) ? bg
                        : &colorMap->Colors[gifFrame->RasterBits[loc]];
                if (color)
                    line[x] = ARGB(255, color->Red, color->Green, color->Blue);
            }
            px +=pxstride;
        }
    }
}
#endif
int GIFDecoder::load(std::istream&is){
    int err;
    GifFileType*gifFileType = DGifOpen(&is,GIFRead,&err);
    LOGE_IF(gifFileType==nullptr,"git load failed");
    if(gifFileType==nullptr)return 0;
    DGifSlurp(gifFileType);

    mFrameCount = gifFileType->ImageCount;
    mPrivate = gifFileType;
    mImageWidth = gifFileType->SWidth;
    mImageHeight= gifFileType->SHeight;
    for(int i=0 ; i < mFrameCount;i++){
        SavedImage *frame = &(gifFileType->SavedImages[i]);
        GifImageDesc *frameInfo=&(frame->ImageDesc);
        ImageDecoder::ImageFrame*frm=new ImageDecoder::ImageFrame();
        frm->x = frameInfo->Left;
        frm->y = frameInfo->Top;
        frm->width = frameInfo->Width;
        frm->height= frameInfo->Height;
        frm->pixels= new unsigned char[frm->width*frm->height*4];
        mFrames.push_back(frm);
        setFramePixels(i);
        LOGV("frame[%d](%d,%d,%d,%d)disposalMethod=%d delay=%d",i,frm->x,frm->y,frm->width,frm->height,frm->disposalMethod,frm->duration);
    }
    LOGD("GIF %d frames loaded size(%dx%d)",mFrameCount,mImageWidth,mImageHeight);
    return mFrameCount;
}


int GIFDecoder::readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int& frameIndex){
    if(!mFrames.empty()){
        ImageFrame*frm = mFrames[frameIndex];
        const int dststride = image->get_stride();
        const int srcstride = frm->width*4;
        uint8_t* dst = (image->get_data() + frm->y*dststride + frm->x*4);
        uint8_t* src = frm->pixels;
        for(int y=0;y<frm->height;y++){
            uint32_t*ld =(uint32_t*)dst;
            uint32_t*ls =(uint32_t*)src;
            for(int x=0;x<frm->width;x++)
               ld[x]= ls[x];
            dst += dststride;
            src += srcstride;
        }
        frameIndex = (frameIndex+1)%mFrameCount;
        image->mark_dirty();
        return frm->duration;
    }
    return gifDrawFrame((GifFileType*)mPrivate,frameIndex,image->get_stride(),image->get_data(),false);
}

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
