#ifndef __ANIMATION_H__
#define __ANIMATION_H__
#include <core/context.h>

#include <animation/transformation.h>

namespace cdroid{
class Animation{
public:
    static constexpr int INFINITE=-1;
    static constexpr int RESTART = 1;
    static constexpr int REVERSE = 2;
    static constexpr int START_ON_FIRST_FRAME = -1;
    static constexpr int ABSOLUTE = 0;
    static constexpr int RELATIVE_TO_SELF = 1;
    static constexpr int RELATIVE_TO_PARENT = 2;
    static constexpr int ZORDER_NORMAL = 0;
    static constexpr int ZORDER_TOP = 1;
    static constexpr int ZORDER_BOTTOM = -1;
    struct AnimationListener{
        std::function<void(Animation&)>onAnimationStart;
        std::function<void(Animation&)>onAnimationEnd;
        std::function<void(Animation&)>onAnimationRepeat;
    };
    friend class AnimationSet;
private:
    int mZAdjustment;
    int mBackgroundColor;
    float mScaleFactor = 1.f;
    bool mDetachWallpaper = false;
    bool mShowWallpaper;
    bool mMore = true;
    bool mOneMoreTime = true;
    void*mListenerHandler=nullptr;
    Runnable mOnStart;
    Runnable mOnRepeat;
    Runnable mOnEnd;
private:
    bool isCanceled();
    void fireAnimationStart();
    void fireAnimationRepeat();
    void fireAnimationEnd();

protected:
    bool mEnded = false;
    bool mStarted = false;
    bool mCycleFlip = false;
    bool mInitialized = false;
    bool mFillBefore = true;
    bool mFillAfter = false;
    bool mFillEnabled = false;
    long mStartTime = -1;
    long mStartOffset;
    long mDuration;
    int mRepeatCount = 0;
    int mRepeated = 0;
    int mRepeatMode = RESTART;
    Interpolator* mInterpolator;
    AnimationListener mListener;
    Rect mPreviousRegion ;
    Rect mRegion;
    Transformation mTransformation;
    Transformation mPreviousTransformation;
protected:
    Animation(const Animation&other);
    float getScaleFactor()const;
    void ensureInterpolator();
    virtual void applyTransformation(float interpolatedTime, Transformation& t);
    float resolveSize(int type, float value, int size, int parentSize);
    void finalize() ;
public:
    Animation();
    Animation(Context* context,const AttributeSet& attrs);
    virtual void reset();
    void cancel();
    void detach();
    bool isInitialized()const;
    virtual void initialize(int width, int height, int parentWidth, int parentHeight);
    virtual Animation* clone();
    //void setListenerHandler(Handler handler);
    virtual void setInterpolator(Context* context,const std::string& resID) ;
    virtual void setInterpolator(Interpolator* i);
    virtual void setStartOffset(long startOffset) ;
    virtual void setDuration(long durationMillis);
    virtual void restrictDuration(long durationMillis);
    virtual void scaleCurrentDuration(float scale);
    virtual void setStartTime(long startTimeMillis);
    virtual void start();
    void startNow();
    virtual void setRepeatMode(int repeatMode) ;
    void setRepeatCount(int repeatCount);
    bool isFillEnabled()const;
    virtual void setFillEnabled(bool fillEnabled);
    virtual void setFillBefore(bool fillBefore);
    virtual void setFillAfter(bool fillAfter);
    void setZAdjustment(int zAdjustment);
    void setBackgroundColor( int bg);
    bool getDetachWallpaper()const;
    void setDetachWallpaper(bool detachWallpaper);
    void setShowWallpaper(bool showWallpaper);
    Interpolator* getInterpolator();
    virtual long getStartTime()const;
    virtual long getDuration()const;
    long getStartOffset()const;
    int getRepeatMode()const;
    int getRepeatCount()const;
    bool getFillBefore()const;
    bool getFillAfter()const;
    int getZAdjustment()const;
    int getBackgroundColor()const;
    bool getShowWallpaper()const;
    virtual bool willChangeTransformationMatrix()const;
    virtual bool willChangeBounds()const;
    void setAnimationListener(AnimationListener listener);
    virtual long computeDurationHint();
    virtual bool getTransformation(long currentTime, Transformation& outTransformation);
    virtual bool getTransformation(long currentTime, Transformation& outTransformation, float scale);
    bool hasStarted()const;
    bool hasEnded()const;
    void getInvalidateRegion(int left, int top, int width, int height, Rect& invalidate, Transformation& transformation);
    void initializeInvalidateRegion(int left, int top, int width, int height);
    virtual bool hasAlpha();
};
}
#endif
