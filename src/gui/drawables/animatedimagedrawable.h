#ifndef __ANIMATED_IMAGE_DRAWABLE_H__
#define __ANIMATED_IMAGE_DRAWABLE_H__
#include <drawables/drawable.h>
#include <core/handler.h>
namespace cdroid{
/*for drawing animated images (like GIF)*/
class AnimatedImageDrawable:public Drawable,public Animatable2{
private:
    class State{
    public:
        bool mAutoMirrored;
        int mRepeatCount;
        int mCurrentFrame;
        int mFrameCount;
        void*mHandler;
	Cairo::RefPtr<Cairo::ImageSurface>mImage;
        State();
        ~State();
    };
    int mIntrinsicWidth;
    int mIntrinsicHeight;
    bool mStarting;
    Handler* mHandler;
    State mState;
    Runnable mRunnable;
    ColorFilter* mColorFilter;
    std::vector<Animatable2::AnimationCallback> mAnimationCallbacks;
    int loadGIF(std::istream&);
    Handler* getHandler();
    void postOnAnimationStart();
    void postOnAnimationEnd();
public:
    static constexpr int REPEAT_INFINITE=-1;
    static constexpr int LOOP_INFINITE = REPEAT_INFINITE;
    static constexpr int REPEAT_UNDEFINED = -2;
public:
    AnimatedImageDrawable();
    AnimatedImageDrawable(cdroid::Context*,const std::string&res);
    ~AnimatedImageDrawable();
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
