#ifndef __ANIMATION_DRAWABLE_H__
#define __ANIMATION_DRAWABLE_H__
#include <drawables/drawablecontainer.h>
namespace cdroid{
class AnimationDrawable:public DrawableContainer,public Animatable{
private:
    class AnimationState:public DrawableContainer::DrawableContainerState{
    public:
        bool mOneShot; 
        std::vector<int>mDurations;
        AnimationState(const AnimationState*,AnimationDrawable*owner);
        void mutate();
        Drawable*newDrawable()override;
        void addFrame(Drawable*,int dur);
    };
     std::shared_ptr<AnimationState>mAnimationState;
    int mCurFrame;
    bool mRunning;
    bool mAnimating;
    bool mMutated;
    Runnable mRunnable;
private:
    void setFrame(int frame,bool unschedule,bool animate);
    AnimationDrawable(std::shared_ptr<AnimationState>);
protected:
    void setConstantState(std::shared_ptr<DrawableContainerState>state)override;
    void run();
    std::shared_ptr<DrawableContainerState> cloneConstantState()override;
public:
    AnimationDrawable();
    bool setVisible(bool visible,bool restart)override;
    void start()override;
    void stop()override;
    bool isRunning()override;
    void unscheduleSelf(Runnable& what);
    int getNumberOfFrames();
    Drawable*getFrame(int index);
    int getDuration(int);
    bool isOneShot();
    void setOneShot(bool oneShot);
    void addFrame(Drawable*frame,int duration);
    void nextFrame(bool unschedule);
    Drawable*mutate()override;
    void clearMutated()override;
    static Drawable*inflate(Context*ctx,const AttributeSet&);
};

}
#endif
