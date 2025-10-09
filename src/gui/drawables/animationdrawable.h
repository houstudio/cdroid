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
        void mutate()override;
        AnimationDrawable*newDrawable()override;
        void addFrame(Drawable*,int dur);
        int64_t getTotalDuration()const;
    };
    std::shared_ptr<AnimationState>mAnimationState;
    int  mCurFrame;
    bool mRunning;
    bool mAnimating;
    bool mMutated;
    Runnable mRunnable;
private:
    void setFrame(int frame,bool unschedule,bool animate);
    AnimationDrawable(std::shared_ptr<AnimationState>);
    void updateStateFromTypedArray(const AttributeSet&a);
    void inflateChildElements(XmlPullParser& parser,const AttributeSet& atts);
protected:
    void setConstantState(std::shared_ptr<DrawableContainerState>state)override;
    void run();
    std::shared_ptr<DrawableContainerState> cloneConstantState()override;
public:
    AnimationDrawable();
    ~AnimationDrawable()override;
    bool setVisible(bool visible,bool restart)override;
    void start()override;
    void stop()override;
    bool isRunning()override;
    void unscheduleSelf(const Runnable& what)override;
    int getNumberOfFrames()const;
    Drawable*getFrame(int index)const;
    int64_t getDuration(int)const;
    int64_t getTotalDuration()const;
    bool isOneShot()const;
    void setOneShot(bool oneShot);
    void addFrame(Drawable*frame,int duration);
    void nextFrame(bool unschedule);
    AnimationDrawable*mutate()override;
    void clearMutated()override;
    void inflate(XmlPullParser& parser,const AttributeSet& atts)override;
};

}
#endif
