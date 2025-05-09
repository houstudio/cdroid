#ifndef __ANIMATION_SCALE_LIST_DRAWABLE_H__
#define __ANIMATION_SCALE_LIST_DRAWABLE_H__
#include <drawables/drawablecontainer.h>
namespace cdroid{
class AnimationScaleListDrawable:public DrawableContainer,public Animatable {
protected:
    class AnimationScaleListState:public DrawableContainerState {
        // The index of the last static drawable.
        int mStaticDrawableIndex = -1;
        // The index of the last animatable drawable.
        int mAnimatableDrawableIndex = -1;
    public:
        AnimationScaleListState(const AnimationScaleListState* orig, AnimationScaleListDrawable* owner);
        void mutate()override;
        int addDrawable(Drawable* drawable);
        AnimationScaleListDrawable* newDrawable() override;
        //bool canApplyTheme() override;
        int getCurrentDrawableIndexBasedOnScale();
    };
private:
    bool mMutated;
    std::shared_ptr<AnimationScaleListState> mAnimationScaleListState;
private:
    AnimationScaleListDrawable(std::shared_ptr<AnimationScaleListState> state);
    void inflateChildElements(XmlPullParser& parser,const AttributeSet& attrs);
protected:
    bool onStateChange(const std::vector<int>& stateSet)override;
    void setConstantState(std::shared_ptr<DrawableContainerState> state) override;
public:
    AnimationScaleListDrawable();
    void inflate(XmlPullParser& parser,const AttributeSet& attrs)override;
    AnimationScaleListDrawable* mutate() override;
    void clearMutated() override;
    void start() override;
    void stop() override;
    bool isRunning()override;
    //void applyTheme(@NonNull Theme theme);
};
}/*endof namespace*/
#endif/*__ANIMATION_SCALE_LIST_DRAWABLE_H__*/
