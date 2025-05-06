#ifndef __TRANSMITION_DRAWABLE_H__
#define __TRANSMITION_DRAWABLE_H__
#include<drawables/layerdrawable.h>

namespace cdroid{

class TransitionDrawable:public LayerDrawable{
private:
    static constexpr int TRANSITION_STARTING = 0;
    static constexpr int TRANSITION_RUNNING = 1;
    static constexpr int TRANSITION_NONE = 2;
private:
    int  mTransitionState;
    bool mReverse;
    int64_t mStartTimeMillis;
    int  mFrom;
    int  mTo;
    int  mDuration;
    int  mOriginalDuration;
    int  mAlpha;
    bool mCrossFade;

    class TransitionState:public LayerDrawable::LayerState{
    public:
        TransitionState(TransitionState* orig, TransitionDrawable* owner);
        TransitionDrawable*newDrawable()override;
    };

    TransitionDrawable(std::shared_ptr<TransitionState> state);
    std::shared_ptr<LayerDrawable::LayerState> createConstantState(LayerState* state,const AttributeSet*)override;
public:
    TransitionDrawable();
    TransitionDrawable(const std::vector<Drawable*>drawables);
    void startTransition(int durationMillis);
    void resetTransition();
    void reverseTransition(int duration);
    bool isCrossFadeEnabled()const;
    void setCrossFadeEnabled(bool enabled);
    void draw(Canvas&canvas)override;
};

}/*endof namespace*/
#endif
