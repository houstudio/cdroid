#ifndef __ANIMATED_IMAGE_DRAWABLE_H__
#define __ANIMATED_IMAGE_DRAWABLE_H__
#include <drawables/drawable.h>
#include <core/handler.h>
namespace cdroid{
/*for drawing animated images (like GIF)*/
class ImageDecoder;
class AnimatedImageDrawable:public Drawable,public Animatable2{
private:
    class AnimatedImageState:public std::enable_shared_from_this<AnimatedImageState>,public ConstantState{
    public:
        bool mAutoMirrored;
        int mFrameCount;
        ImageDecoder*mDecoder;
        Cairo::RefPtr<Cairo::ImageSurface>mImage;

        AnimatedImageState();
        AnimatedImageState(const AnimatedImageState& state);
        ~AnimatedImageState();
        Drawable* newDrawable()override;
        int getChangingConfigurations()const override;
    };
    
    int mIntrinsicWidth;
    int mIntrinsicHeight;
    bool mStarting;
    int mCurrentFrame;
    int mRepeatCount;
    Handler* mHandler;
    std::shared_ptr<AnimatedImageState> mAnimatedImageState;
    Runnable mRunnable;
    ColorFilter* mColorFilter;
    std::vector<Animatable2::AnimationCallback> mAnimationCallbacks;
    int loadGIF(std::istream&);
    Handler* getHandler();
    void postOnAnimationStart();
    void postOnAnimationEnd();
    AnimatedImageDrawable(std::shared_ptr<AnimatedImageState> state);
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
    int getIntrinsicWidth()const override;
    int getIntrinsicHeight()const override;
    void setAlpha(int alpha)override;
    int getAlpha()const override;
    void draw(Canvas& canvas)override;
    bool isRunning()override;
    void start()override;
    void stop()override;
    void registerAnimationCallback(Animatable2::AnimationCallback callback);
    bool unregisterAnimationCallback(Animatable2::AnimationCallback callback);
    void clearAnimationCallbacks();
    static Drawable*inflate(Context*,const AttributeSet&);
};

}//end namespace
#endif
