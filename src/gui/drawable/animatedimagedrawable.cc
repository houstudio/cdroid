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
#include <drawable/animatedimagedrawable.h>
#include <core/systemclock.h>
#include <porting/cdlog.h>
#include <view/view.h>
#include <view/gravity.h>
#include <view/choreographer.h>
#include <image-decoders/imagedecoder.h>
#include <image-decoders/framesequence.h>
#include <porting/cdgraph.h>
#include <cstring>
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#if HAVE_SYS_PRCTL_H
#include <sys/prctl.h>
#elif HAVE_LINUX_PRCTL_H
#include <linux/prctl.h>
#endif

namespace cdroid{
#define ENABLE_DMABLIT 0
/*delay (ms) used to re-check whether the decode thread has finished a slow frame;
  everything scheduled with this is posted on the UI thread, never on the decode thread*/
#define ANIMATION_POLL_DELAY_MS 10

std::queue<AnimatedImageDrawable::DecodeTask> AnimatedImageDrawable::sDecodeQueue;
std::thread AnimatedImageDrawable::sDecodeThread;
std::once_flag AnimatedImageDrawable::sDecodeOnce;
std::mutex AnimatedImageDrawable::sDecodeMutex;
std::condition_variable AnimatedImageDrawable::sDecodeCV;

AnimatedImageDrawable::AnimatedImageDrawable()
  :AnimatedImageDrawable(std::make_shared<AnimatedImageState>()){
}

AnimatedImageDrawable::AnimatedImageDrawable(std::shared_ptr<AnimatedImageState> state)
   :Drawable(){
    mStarting = false;
    mMutated  = false;
    mRepeated = 0;
    mRepeatCount = state?state->mRepeatCount:REPEAT_UNDEFINED;
    mIntrinsicWidth = mIntrinsicHeight = 0;
    mAnimatedImageState = state;
    mCurrentFrame = -1;
    mNextFrame = 0;
    mAlpha = 1.f;
    mFrameScheduled = false;
    mImageHandler = nullptr;
    mFrameSequenceState = nullptr;
    auto frmSequence = mAnimatedImageState->mFrameSequence;
    LOGD("%p frmSequence=%p",this,frmSequence);

    if(frmSequence){
        mFrameSequenceState = frmSequence->createState();
        mImage = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,frmSequence->getWidth(),frmSequence->getHeight());
    }

    mRenderImage = mImage;
    mDecodeImage = frmSequence ? Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, frmSequence->getWidth(), frmSequence->getHeight()) : nullptr;
    mDecodeInProgress = false;
    mDecodeFuture = std::shared_future<void>();
    std::call_once(sDecodeOnce, []{ sDecodeThread = std::thread(decodeWorker); });

    /*Animation cadence tick: just request a redraw on the UI thread. draw() promotes the
      next frame (decoded on the decode thread) and reschedules the following tick.*/
    mRunnable = [this](){
        mFrameScheduled = false;
        invalidateSelf();
    };
}

AnimatedImageDrawable::AnimatedImageDrawable(cdroid::Context*ctx,const std::string&res)
   :AnimatedImageDrawable(){
    auto frmSequence = FrameSequence::create(ctx,res);
    if(frmSequence==nullptr)return;
    mAnimatedImageState->mFrameSequence = frmSequence;
    mRepeatCount = frmSequence->getDefaultLoopCount();
    if(mRepeatCount<=0)
        mRepeatCount = REPEAT_UNDEFINED;
    mFrameSequenceState = frmSequence->createState();
#if ENABLE(DMABLIT)
    GFXCreateSurface(0,&mImageHandler,frmSequence->getWidth(),frmSequence->getHeight(),0,0);
    GFXLockSurface(mImageHandler,(void**)&buffer,&pitch);
    mImage = Cairo::ImageSurface::create(buffer,Cairo::Surface::Format::ARGB32,frmSequence->getWidth(),frmSequence->getHeight(),pitch);
#else
    mImage = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,frmSequence->getWidth(),frmSequence->getHeight());
#endif
    LOGD("%p %s %dx%dx%d frmSequence=%p",this,res.c_str(),frmSequence->getWidth(),frmSequence->getHeight(),frmSequence->getFrameCount(),frmSequence);
    mAnimatedImageState->mFrameCount = frmSequence->getFrameCount();
    mRenderImage = mImage;
    mDecodeImage = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, frmSequence->getWidth(), frmSequence->getHeight());
    mDecodeInProgress = false;
    mDecodeFuture = std::shared_future<void>();
    std::call_once(sDecodeOnce, []{ sDecodeThread = std::thread(decodeWorker); });
}

AnimatedImageDrawable::~AnimatedImageDrawable(){
    mStarting = false;
    auto frmSequence = mAnimatedImageState->mFrameSequence;
    LOGD_IF(frmSequence,"%p/%p %dx%dx%d",this,frmSequence,frmSequence->getWidth(),frmSequence->getHeight(),frmSequence->getFrameCount());
    Choreographer::getInstance().removeCallbacks(Choreographer::CALLBACK_ANIMATION,nullptr,this);
    if(mRunnable) unscheduleSelf(mRunnable);
    mRunnable = nullptr;
    {
        std::lock_guard<std::mutex> lock(sDecodeMutex);
        std::queue<DecodeTask> filteredQueue;
        while(!sDecodeQueue.empty()){
            auto task = sDecodeQueue.front();
            sDecodeQueue.pop();
            if(task.instance != this){
                filteredQueue.push(task);
            }
        }
        std::swap(sDecodeQueue, filteredQueue);
    }
    /*If a decode for this instance is currently in flight, block (efficient wait, not a
      busy spin) until the decode thread is done with `this`. Only the in-flight task
      (mDecodeInProgress) still references this instance; the queued ones were removed
      above, so we never wait on a future whose task was dropped and would never be set.*/
    if(mDecodeInProgress.load() && mDecodeFuture.valid()){
        mDecodeFuture.wait();
    }
    delete mFrameSequenceState;
    if(mImageHandler){
        GFXDestroySurface(mImageHandler);
        mImageHandler = nullptr;
    }
}

AnimatedImageDrawable*AnimatedImageDrawable::mutate(){
    if (!mMutated && Drawable::mutate() == this) {
        mAnimatedImageState = std::make_shared<AnimatedImageState>(*mAnimatedImageState);
        mMutated = true;
    }
    return this;
}

std::shared_ptr<Drawable::ConstantState>AnimatedImageDrawable::getConstantState(){
    return std::dynamic_pointer_cast<ConstantState>(mAnimatedImageState);
}

void AnimatedImageDrawable::setRepeatCount(int repeatCount){
    if (repeatCount < REPEAT_INFINITE) {
        LOGE("invalid value passed to setRepeatCount %d",repeatCount);
    }
    if (mRepeatCount != repeatCount) {
        mRepeatCount = repeatCount;
    }
}

int AnimatedImageDrawable::getRepeatCount()const{
    return mRepeatCount;
}

int AnimatedImageDrawable::getIntrinsicWidth() {
    auto frameSequence = mAnimatedImageState->mFrameSequence;
    return frameSequence ? frameSequence->getWidth():mIntrinsicWidth;
}

int AnimatedImageDrawable::getIntrinsicHeight() {
    auto frameSequence = mAnimatedImageState->mFrameSequence;
    return frameSequence ? frameSequence->getHeight():mIntrinsicHeight;
}

void AnimatedImageDrawable::setAlpha(int alpha){
    mAnimatedImageState->mAlpha = alpha&0xFF;
    mAlpha = float(alpha&0xFF)/255.f;
    invalidateSelf();
}

int AnimatedImageDrawable::getAlpha()const{
    return mAnimatedImageState->mAlpha;
}

int AnimatedImageDrawable::getOpacity()const{
    return PixelFormat::TRANSLUCENT;
}

void AnimatedImageDrawable::setAutoMirrored(bool mirrored) {
    if (mAnimatedImageState->mAutoMirrored != mirrored) {
        mAnimatedImageState->mAutoMirrored = mirrored;
        if (getLayoutDirection() == LayoutDirection::RTL) {
            //nSetMirrored(mState.mNativePtr, mirrored);
            invalidateSelf();
        }
    }
}

bool AnimatedImageDrawable::isAutoMirrored() const{
    return mAnimatedImageState->mAutoMirrored;
}

bool AnimatedImageDrawable::onLayoutDirectionChanged(int layoutDirection) {
    if (!mAnimatedImageState->mAutoMirrored) {
        return false;
    }

    const bool mirror = layoutDirection == LayoutDirection::RTL;
    mAnimatedImageState->mAutoMirrored= mirror;
    return true;
}

void AnimatedImageDrawable::draw(Canvas& canvas){
    if(mRenderImage == nullptr)return;
    const int frameCount = mAnimatedImageState->mFrameCount;
    /*one-shot: onAnimationStart fires exactly once, on the first draw after start()
      (mirrors Android's "if (mStarting) { mStarting = false; postOnAnimationStart(); }").*/
    if(mStarting){
        mStarting = false;
        postOnAnimationStart();
    }
    bool promoted = false;

    /*Show the frame mNextFrame. This runs whether or not the animation is running, so the
      drawable displays its current frame as a poster — the original on-demand draw() decoded
      frame 0 on the first draw with no mRunning guard, and we keep that behavior. Only the
      advance/schedule step below is gated on mRunning. The decode thread stays the sole
      caller of drawFrame().*/
    if(frameCount > 0 && mCurrentFrame != mNextFrame){
        if(!mDecodeFuture.valid()){
            submitDecodeTask(mNextFrame, mCurrentFrame);
        }
        if(mDecodeFuture.valid()){
            /*For the first frame after start()/restart() (mCurrentFrame == -1) block until
              the decode thread finishes, so frame 0 shows on this draw. After that only
              promote when the prefetch is already ready, polling otherwise, so a slow
              decode never stalls the UI thread.*/
            const bool ready = (mCurrentFrame == -1)
                ? (mDecodeFuture.wait(), true)
                : (mDecodeFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
            if(ready){
                if(mDecodeImage){
                    std::lock_guard<std::mutex> lock(mFrameSequenceMutex);
                    const int stride = mRenderImage->get_stride();
                    const int height = mRenderImage->get_height();
                    std::memcpy(mRenderImage->get_data(), mDecodeImage->get_data(), (size_t)stride * height);
                    mFrameDelay = mDecodedFrameDelay;
                    mCurrentFrame = mNextFrame;
                    mRenderImage->mark_dirty();
                }
                mDecodeFuture = std::shared_future<void>();
                promoted = true;

                /*Natural end: a full play just completed (its last frame was shown). Android
                  fires onAnimationEnd exactly once when the repeat count is exhausted (or on
                  stop()), never once per loop. repeatCount==N plays N+1 times; REPEAT_INFINITE
                  (<0) never ends. The just-shown last frame stays displayed, matching Android.*/
                if(mRunning && mCurrentFrame == frameCount - 1){
                    mRepeated ++;
                    if((mRepeatCount >= 0) && (mRepeated > mRepeatCount)){
                        mRunning = false;
                        postOnAnimationEnd();
                    }
                }
                /*advance + prefetch the next frame only while still running*/
                if(mRunning){
                    mNextFrame = (mNextFrame + 1) % frameCount;
                    submitDecodeTask(mNextFrame, mCurrentFrame);
                }
            }
        }
    }

    LOGV("%p draw Frame %d/%d running=%d repeat=%d/%d nextDelay=%d",this,mCurrentFrame,
          frameCount,mRunning,mRepeated,mRepeatCount,mFrameDelay);

    canvas.save();
    void *handler = canvas.getHandler();
    if( (mImageHandler == nullptr) || (handler == nullptr) ){
        const bool isOpaque = mAnimatedImageState->mFrameSequence->isOpaque();
        canvas.set_source(mRenderImage,mBounds.left,mBounds.top);
        canvas.set_operator(isOpaque?Cairo::Context::Operator::SOURCE:Cairo::Context::Operator::OVER);
        canvas.translate(mBounds.left,mBounds.top);
        if(mAnimatedImageState->mAutoMirrored && (getLayoutDirection() == View::LAYOUT_DIRECTION_RTL)){
            canvas.scale(-1,1);
        }
        canvas.rectangle(0,0,mBounds.width,mBounds.height);
        canvas.paint_with_alpha(mAlpha);
    }else{
#if ENABLE(DMABLIT)
        Rect rd = {0,0,mBounds.width,mBounds.height};
        Cairo::Matrix mtx = canvas.get_matrix();
        double angle = std::atan2(mtx.xx,-mtx.xy)*180.0/M_PI;
        LOGD("canvas.angle = %d",(int(angle) - 90) % 360);
        GFXBlit(handler,0,0,mImageHandler,nullptr);
#endif
    }
    canvas.restore();

    /*Drive the cadence on the UI thread. After a promotion, show the next frame after
      mFrameDelay; while waiting for a slow decode, poll briefly so we promote as soon as
      the decode thread is done. None of this is ever invoked from the decode thread.
      Gated on mRunning: once the repeat count is exhausted (or stop() is called) mRunning
      becomes false and no more frames are scheduled.*/
    if(mRunning){
        if(!mFrameScheduled){
            int64_t delay;
            if(promoted){
                delay = (mFrameDelay > 0) ? mFrameDelay : Choreographer::DEFAULT_FRAME_DELAY;
            } else if(mDecodeFuture.valid()){
                delay = ANIMATION_POLL_DELAY_MS;
            } else {
                delay = 0;
            }
            if(delay > 0){
                unscheduleSelf(mRunnable);
                scheduleSelf(mRunnable, SystemClock::uptimeMillis() + delay);
                LOGV("%p schedule next frame %d in %lld ms",this,mNextFrame,(long long)delay);
                mFrameScheduled = true;
            }
        }
    }
}

bool AnimatedImageDrawable::isRunning(){
    return mRunning;
}

void AnimatedImageDrawable::start(){
    if (mAnimatedImageState->mFrameSequence == nullptr) {
        LOGE("called start on empty AnimatedImageDrawable");
        return ;
    }

    if ((!mRunning) && (mAnimatedImageState->mFrameCount > 1)){
        mRunning = true;
        mRepeated = 0;
        mCurrentFrame = -1;
        mNextFrame = 0;
        mFrameScheduled = false;
        /*one-shot: draw() fires onAnimationStart on the next draw, like Android*/
        mStarting = true;
        /*drop any stale decode from a previous run; draw() kicks off a fresh prefetch*/
        if(mDecodeFuture.valid())
            mDecodeFuture = std::shared_future<void>();
        invalidateSelf();
    }
}

void AnimatedImageDrawable::restart(int fromFrame){
    mCurrentFrame = -1;
    mNextFrame = fromFrame;
    mFrameScheduled = false;
    mRepeated = 0;
    if(mDecodeFuture.valid())
        mDecodeFuture = std::shared_future<void>();
    if((!mRunning) && mAnimatedImageState->mFrameCount){
        mRunning = true;
        mStarting = true;/*one-shot start callback, fires on next draw*/
    }
    invalidateSelf();
}

void AnimatedImageDrawable::stop(){
    if (mAnimatedImageState->mFrameSequence == nullptr) {
        LOGE("called stop on empty AnimatedImageDrawable");
        return;
    }
    if(mRunnable)unscheduleSelf(mRunnable);
    if (mRunning){
        mRunning = false;
        mStarting = false;
        mFrameScheduled = false;
        postOnAnimationEnd();
    }
}

void AnimatedImageDrawable::registerAnimationCallback(const Animatable2::AnimationCallback& callback){
    mAnimationCallbacks.push_back(callback);
}

static bool operator==(const Animatable2::AnimationCallback&a,const Animatable2::AnimationCallback&b){
    return (a.onAnimationStart==b.onAnimationStart) && (a.onAnimationEnd==b.onAnimationEnd);
}

bool AnimatedImageDrawable::unregisterAnimationCallback(const Animatable2::AnimationCallback& callback){
    auto it = std::find(mAnimationCallbacks.begin(),mAnimationCallbacks.end(),callback);
    const bool rc = (it!=mAnimationCallbacks.end());
    if(rc)
        mAnimationCallbacks.erase(it);
    return rc;
}

void AnimatedImageDrawable::postOnAnimationStart(){
    if (mAnimationCallbacks.size() == 0) {
        return;
    }
    Runnable r([this](){
        for (Animatable2::AnimationCallback callback : mAnimationCallbacks) {
            if(callback.onAnimationStart)callback.onAnimationStart(*this);
        }
    });
    Choreographer::getInstance().postCallback(Choreographer::CALLBACK_ANIMATION,r,this);
}

void AnimatedImageDrawable::postOnAnimationEnd(){
    if (mAnimationCallbacks.size()==0) {
        return;
    }
    Runnable r([this]{
        for (Animatable2::AnimationCallback callback : mAnimationCallbacks) {
            if(callback.onAnimationEnd)callback.onAnimationEnd(*this);
        }
    });
    Choreographer::getInstance().postCallback(Choreographer::CALLBACK_ANIMATION,r,this);
}

void AnimatedImageDrawable::clearAnimationCallbacks(){
    mAnimationCallbacks.clear();
}

void AnimatedImageDrawable::onBoundsChange(const Rect& bounds) {
    /*if (mAnimatedImageState.mNativePtr != 0) {
        nSetBounds(mState.mNativePtr, bounds);
    }*/
}

void AnimatedImageDrawable::inflate(XmlPullParser&parser,const AttributeSet&atts){
    Drawable::inflate(parser,atts);
    updateStateFromTypedArray(atts, mSrcDensityOverride);
}

void AnimatedImageDrawable::updateStateFromTypedArray(const AttributeSet&atts,int srcDensityOverride){
    std::string srcResid =atts.getString("src");
    if(!srcResid.empty()){
        Drawable* drawable = nullptr;
        // This may have previously been set without a src if we were waiting for a  theme.
        /*const int repeatCount = mState->mRepeatCount;
        // Transfer the state of other to this one. other will be discarded.
        AnimatedImageDrawable* other = (AnimatedImageDrawable*) drawable;
        mState = other.mState;
        other.mState = null;
        mIntrinsicWidth =  other.mIntrinsicWidth;
        mIntrinsicHeight = other.mIntrinsicHeight;
        if (repeatCount != REPEAT_UNDEFINED) {
            this.setRepeatCount(repeatCount);
        }*/
        auto frmSequence = FrameSequence::create(atts.getContext(),srcResid);
        if(frmSequence==nullptr)return;
        mAnimatedImageState->mFrameSequence = frmSequence;
        mAnimatedImageState->mFrameCount = frmSequence->getFrameCount();
        mIntrinsicWidth = frmSequence->getWidth();
        mIntrinsicHeight= frmSequence->getHeight();
        mRepeatCount = frmSequence->getDefaultLoopCount();
        if(mRepeatCount<=0)
            mRepeatCount = REPEAT_UNDEFINED;
        this->mFrameSequenceState = frmSequence->createState();
        /*The state ctor runs before the frame sequence is known, so it leaves the pixel
          buffers null; (re)create them here. Idempotent — skipped if already allocated
          (e.g. the buffers created by the Context ctor or by newDrawable).*/
        if(mImage == nullptr){
            mImage = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, frmSequence->getWidth(), frmSequence->getHeight());
            mRenderImage = mImage;
            mDecodeImage = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, frmSequence->getWidth(), frmSequence->getHeight());
        }
    }
    mAnimatedImageState->mAutoMirrored = atts.getBoolean("autoMirrored",false);
    const int repeatCount= atts.getInt("repeatCount",REPEAT_UNDEFINED);
    const bool autoStart = atts.getBoolean("autoStart",false);
    if(repeatCount!=REPEAT_UNDEFINED)
        setRepeatCount(repeatCount);
    if(autoStart && mFrameSequenceState){
        start();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
AnimatedImageDrawable::AnimatedImageState::AnimatedImageState(){
    mAutoMirrored= false;
    mFrameCount  = 0;
    mRepeatCount = REPEAT_UNDEFINED;
    mFrameSequence = nullptr;
}

AnimatedImageDrawable::AnimatedImageState::AnimatedImageState(const AnimatedImageState& state){
    mAutoMirrored = state.mAutoMirrored;
    mFrameCount = state.mFrameCount;
    mRepeatCount= state.mRepeatCount;
    mFrameSequence = state.mFrameSequence;
}

AnimatedImageDrawable::AnimatedImageState::~AnimatedImageState(){
    delete mFrameSequence;
}

AnimatedImageDrawable* AnimatedImageDrawable::AnimatedImageState::newDrawable(){
    return new AnimatedImageDrawable(std::dynamic_pointer_cast<AnimatedImageState>(shared_from_this()));
}

int AnimatedImageDrawable::AnimatedImageState::getChangingConfigurations()const{
    return 0;
}

void AnimatedImageDrawable::submitDecodeTask(int frameIndex, int prevFrame) {
    mDecodePromise = std::promise<void>();
    mDecodeFuture = mDecodePromise.get_future();
    {
        std::lock_guard<std::mutex> lock(sDecodeMutex);
        sDecodeQueue.push({this, frameIndex, prevFrame});
    }
    sDecodeCV.notify_one();
}

void AnimatedImageDrawable::decodeWorker() {
#if HAVE_PRCTL
    prctl(PR_SET_NAME,"AniImageDecoder",0,0,0);
#elif HAVE_PTHREAD_SETNAME_NP
    pthread_setname_np(pthread_self(), "AniImageDecoder");
#endif
    while(true) {
        DecodeTask task;
        {
            std::unique_lock<std::mutex> lock(sDecodeMutex);
            sDecodeCV.wait(lock, []{ return !sDecodeQueue.empty(); });
            task = sDecodeQueue.front();
            sDecodeQueue.pop();
            task.instance->mDecodeInProgress = true;
        }
        AnimatedImageDrawable* instance = task.instance;
        const int frameIndex = task.frameIndex;
        /*The decode thread is the SOLE caller of drawFrame(). prevFrame is supplied by
          the UI thread and equals the frame currently held in mDecodeImage, so delta
          decoding stays correct. mDecodedFrameDelay is read back by the UI thread under
          the same mutex when it copies the finished frame. Nothing here touches
          Choreographer/scheduleSelf/invalidateSelf (UI-thread-only APIs).*/
        if(instance->mFrameSequenceState && instance->mDecodeImage){
            std::lock_guard<std::mutex> lock(instance->mFrameSequenceMutex);
            instance->mDecodedFrameDelay = instance->mFrameSequenceState->drawFrame(
                frameIndex,
                (uint32_t*)instance->mDecodeImage->get_data(),
                instance->mDecodeImage->get_stride()>>2,
                task.prevFrame);
            instance->mDecodeImage->mark_dirty();
        }
        LOGV("%p decode frame %d completed cpuid=%d",instance,frameIndex,sched_getcpu());
        instance->mDecodePromise.set_value();
        instance->mDecodeInProgress = false;
    }
}

}
