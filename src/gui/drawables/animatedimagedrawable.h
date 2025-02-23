#ifndef __ANIMATED_IMAGE_DRAWABLE_H__
#define __ANIMATED_IMAGE_DRAWABLE_H__
#include <drawables/drawable.h>
#include <core/handler.h>
namespace cdroid{
class FrameSequence;
class FrameSequenceState;
/*for drawing animated images (like GIF)*/
class AnimatedImageDrawable:public Drawable,public Animatable2{
private:
    class AnimatedImageState:public std::enable_shared_from_this<AnimatedImageState>,public ConstantState{
    public:
        bool mAutoMirrored;
        int mFrameCount;
        int mRepeatCount;
        int mAlpha;
        FrameSequence*mFrameSequence;
        AnimatedImageState();
        AnimatedImageState(const AnimatedImageState& state);
        ~AnimatedImageState();
        AnimatedImageDrawable* newDrawable()override;
        int getChangingConfigurations()const override;
    };
    
    int mIntrinsicWidth;
    int mIntrinsicHeight;
    bool mStarting;
    bool mFrameScheduled;
    bool mMutated;
    int mCurrentFrame;
    int mNextFrame;
    int mFrameDelay;
    int mRepeatCount;
    int mRepeated;/*repeated played rounds*/
    std::shared_ptr<AnimatedImageState> mAnimatedImageState;
    Cairo::RefPtr<Cairo::ImageSurface>mImage;
    FrameSequenceState*mFrameSequenceState;
    Runnable mRunnable;
    ColorFilter* mColorFilter;
    std::vector<Animatable2::AnimationCallback> mAnimationCallbacks;
    void* mImageHandler;
    void postOnAnimationStart();
    void postOnAnimationEnd();
    AnimatedImageDrawable(std::shared_ptr<AnimatedImageState> state);
protected:
    void onBoundsChange(const Rect& bounds)override;
public:
    static constexpr int REPEAT_INFINITE=-1;
    static constexpr int LOOP_INFINITE = REPEAT_INFINITE;
    static constexpr int REPEAT_UNDEFINED = -2;
public:
    AnimatedImageDrawable();
    AnimatedImageDrawable(cdroid::Context*,const std::string&res);
    ~AnimatedImageDrawable();
    std::shared_ptr<ConstantState>getConstantState()override;
    void setRepeatCount(int repeatCount);
    int getRepeatCount()const;
    int getIntrinsicWidth() override;
    int getIntrinsicHeight() override;
    void setAlpha(int alpha)override;
    int getAlpha()const override;
    int getOpacity()override;
    void setAutoMirrored(bool);
    bool isAutoMirrored()const;
    bool onLayoutDirectionChanged(int layoutDirection)override;
    void draw(Canvas& canvas)override;
    bool isRunning()override;
    void start()override;
    void stop()override;
    void restart(int fromFrame=0);
    AnimatedImageDrawable*mutate()override;
    void registerAnimationCallback(Animatable2::AnimationCallback callback);
    bool unregisterAnimationCallback(Animatable2::AnimationCallback callback);
    void clearAnimationCallbacks();
    static Drawable*inflate(Context*,const AttributeSet&);
};

}//end namespace
#endif
