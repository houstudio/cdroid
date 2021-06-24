#include <drawables/animatedimagedrawable.h>
#include <systemclock.h>
#include <cdlog.h>


namespace cdroid{

#ifdef ENABLE_GIF

#include <gif/gif_lib.h>
#define  argb(a, r, g, b) ( ((a) & 0xff) << 24 ) | ( ((r) & 0xff) << 16 ) | ( ((g) & 0xff) << 8 ) | ((b) & 0xff)

#define  dispose(ext) (((ext)->Bytes[0] & 0x1c) >> 2)
#define  trans_index(ext) ((ext)->Bytes[3])
#define  transparency(ext) ((ext)->Bytes[0] & 1)
#define  delay(ext) (10*((ext)->Bytes[2] << 8 | (ext)->Bytes[1]))
static int gifDrawFrame(GifFileType*gif,int*current_frame,size_t pxstride, void *pixels,bool force_dispose_1);
#endif

AnimatedImageDrawable::State::State(){
}
AnimatedImageDrawable::State::~State(){
}
AnimatedImageDrawable::AnimatedImageDrawable():Drawable(){

}

void AnimatedImageDrawable::setRepeatCount(int repeatCount){
    if (repeatCount < REPEAT_INFINITE) {
         LOGE("invalid value passed to setRepeatCount %d",repeatCount);
    }
    if (mState.mRepeatCount != repeatCount) {
        mState.mRepeatCount = repeatCount;
    }
}

int AnimatedImageDrawable::getRepeatCount()const{
    return mState.mRepeatCount;
}

int AnimatedImageDrawable::getIntrinsicWidth()const{
    return mIntrinsicWidth;
}

int AnimatedImageDrawable::getIntrinsicHeight()const{
    return mIntrinsicHeight;
}

void AnimatedImageDrawable::setAlpha(int alpha){
    
}

int AnimatedImageDrawable::getAlpha()const{
    return 255;
}
#ifdef ENABLE_GIF
static int GIFRead(GifFileType *gifFile, GifByteType *buff, int rdlen){
    std::istream*is=(std::istream*)gifFile->UserData;
    is->read((char*)buff,rdlen);
    return is->gcount();
}
#endif
int AnimatedImageDrawable::loadGIF(std::istream&is){
    int err;
#ifdef ENABLE_GIF
    GifFileType*gifFileType=DGifOpen(&is,GIFRead,&err);
    if(gifFileType==nullptr)return 0;
    DGifSlurp(gifFileType);
    ExtensionBlock *ext;

    mState.mImage=ImageSurface::create(Surface::Format::ARGB32,gifFileType->SWidth,gifFileType->SHeight);
    mState.mFrameCount=gifFileType->ImageCount;
    mState.mCurrentFrame=0;
    for (int i = 0; i < gifFileType->ImageCount; ++i) {
        SavedImage frame = gifFileType->SavedImages[i];
        for (int j = 0; j < frame.ExtensionBlockCount; ++j) {
            if (frame.ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {
                ext = &frame.ExtensionBlocks[j];
                break;
            }
        }
        if (ext) {
            int frame_delay = 10 * (ext->Bytes[2] << 8 | ext->Bytes[1]);
            //gifBean->delays[i] = frame_delay;
        }
    }
    mState.mHandler=gifFileType;
    mIntrinsicWidth=gifFileType->SWidth;
    mIntrinsicHeight=gifFileType->SHeight;
    return gifFileType->ImageCount; 
#else
    return 0;
#endif
}

constexpr int FINISHED=-1;
void AnimatedImageDrawable::draw(Canvas& canvas){
    if (mStarting) {
        mStarting = false;
        postOnAnimationStart();
    }
#ifdef ENABLE_GIF
    long nextUpdate = gifDrawFrame((GifFileType*)mState.mHandler,&mState.mCurrentFrame,
             mState.mImage->get_stride(),mState.mImage->get_data(),false);//nDraw(mState.mNativePtr, canvas.getNativeCanvasWrapper());
    // a value <= 0 indicates that the drawable is stopped or that renderThread
    // will manage the animation
    if (nextUpdate > 0) {
        if (mRunnable == nullptr) {
            mRunnable = std::bind(&AnimatedImageDrawable::invalidateSelf,this);
        }
        scheduleSelf(mRunnable, nextUpdate + SystemClock::uptimeMillis());
    } else if (nextUpdate == FINISHED) {
        // This means the animation was drawn in software mode and ended.
        postOnAnimationEnd();
    }
#endif
}

bool AnimatedImageDrawable::isRunning(){
    return true;    
}

void AnimatedImageDrawable::start(){

}

void AnimatedImageDrawable::stop(){
}

void AnimatedImageDrawable::registerAnimationCallback(Animatable2::AnimationCallback callback){
    mAnimationCallbacks.push_back(callback);
}

bool AnimatedImageDrawable::unregisterAnimationCallback(Animatable2::AnimationCallback callback){
    auto itr=std::find(mAnimationCallbacks.begin(),mAnimationCallbacks.end(),nullptr);//callback);
    bool rc=(itr!=mAnimationCallbacks.end());
    if(rc)
        mAnimationCallbacks.erase(itr);
    return rc;
}

void AnimatedImageDrawable::postOnAnimationStart(){

}

void AnimatedImageDrawable::postOnAnimationEnd(){
}

void AnimatedImageDrawable::clearAnimationCallbacks(){
}

#ifdef ENABLE_GIF
static int gifDrawFrame(GifFileType*gif,int*current_frame,size_t pxstride, void *pixels,bool force_dispose_1) {
    GifColorType *bg;
    GifColorType *color;
    SavedImage *frame;
    ExtensionBlock *ext = 0;
    GifImageDesc *frameInfo;
    ColorMapObject *colorMap;
    int *line;
    int width, height, x, y, j, loc, n, inc, p;
    void *px;
    width = gif->SWidth;
    height = gif->SHeight;
    frame = &(gif->SavedImages[*current_frame]);
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
    // For dispose = 1, we assume its been drawn
    px = pixels;
    if (ext && dispose(ext) == 1 && force_dispose_1 && *current_frame > 0) {
        *current_frame = *current_frame - 1;
        gifDrawFrame(gif,current_frame,pxstride, pixels, true);
    } else if (ext && dispose(ext) == 2 && bg) {
        for (y = 0; y < height; y++) {
            line = (int *) px;
            for (x = 0; x < width; x++) {
                line[x] = argb(255, bg->Red, bg->Green, bg->Blue);
            }
            px = (int *) ((char *) px + pxstride);
        }
        *current_frame=(*current_frame+1)%gif->ImageCount;
    } else if (ext && dispose(ext) == 3 && *current_frame > 1) {
        *current_frame = *current_frame - 2;
        gifDrawFrame(gif,current_frame,pxstride, pixels, true);
    }else *current_frame=(*current_frame+1)%gif->ImageCount;
    px = pixels;
    if (frameInfo->Interlace) {
        n = 0;
        inc = 8;
        p = 0;
        px = (int *) ((char *) px + pxstride * frameInfo->Top);
        for (y = frameInfo->Top; y < frameInfo->Top + frameInfo->Height; y++) {
            for (x = frameInfo->Left; x < frameInfo->Left + frameInfo->Width; x++) {
                loc = (y - frameInfo->Top) * frameInfo->Width + (x - frameInfo->Left);
                if (ext && frame->RasterBits[loc] == trans_index(ext) && transparency(ext)) {
                    continue;
                }
                color = (ext && frame->RasterBits[loc] == trans_index(ext)) ? bg
                        : &colorMap->Colors[frame->RasterBits[loc]];
                if (color)
                    line[x] = argb(255, color->Red, color->Green, color->Blue);
            }
            px = (int *) ((char *) px + pxstride * inc);
            n += inc;
            if (n >= frameInfo->Height) {
                n = 0;
                switch (p) {
                case 0:
                    px = (int *) ((char *) pixels + pxstride * (4 + frameInfo->Top));
                    inc = 8;     p++;     break;
                case 1:
                    px = (int *) ((char *) pixels + pxstride * (2 + frameInfo->Top));
                    inc = 4;     p++;     break;
                case 2:
                    px = (int *) ((char *) pixels + pxstride * (1 + frameInfo->Top));
                    inc = 2;     p++;     break;
                }
            }
        }
    } else {
        px = (int *) ((char *) px + pxstride * frameInfo->Top);
        for (y = frameInfo->Top; y < frameInfo->Top + frameInfo->Height; y++) {
            line = (int *) px;
            for (x = frameInfo->Left; x < frameInfo->Left + frameInfo->Width; x++) {
                loc = (y - frameInfo->Top) * frameInfo->Width + (x - frameInfo->Left);
                if (ext && frame->RasterBits[loc] == trans_index(ext) && transparency(ext)) {
                    continue;
                }
                color = (ext && frame->RasterBits[loc] == trans_index(ext)) ? bg
                        : &colorMap->Colors[frame->RasterBits[loc]];
                if (color)
                    line[x] = argb(255, color->Red, color->Green, color->Blue);
            }
            px = (int *) ((char *) px + pxstride);
        }
    }
    return delay(ext);
}
#endif

}
