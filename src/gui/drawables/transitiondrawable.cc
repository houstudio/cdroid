#include <drawables/transitiondrawable.h>
#include <systemclock.h>
#include <cdlog.h>


namespace cdroid{
#define TRANSITION_STARTING 0
#define TRANSITION_RUNNING  1
#define TRANSITION_NONE     2

/*TransitionDrawable::TransitionState::TransitionState(const Context*ctx,const AttributeSet&atts){   
}*/

TransitionDrawable::TransitionState::TransitionState(TransitionState* orig, TransitionDrawable* owner)
    :LayerState::LayerState(orig,owner){
}

TransitionDrawable*TransitionDrawable::TransitionState::newDrawable(){
    return new TransitionDrawable(std::dynamic_pointer_cast<TransitionState>(shared_from_this()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
TransitionDrawable::TransitionDrawable()
    :LayerDrawable(std::make_shared<TransitionState>(nullptr,this)){
}

TransitionDrawable::TransitionDrawable(Context*ctx,const AttributeSet&atts)
   :LayerDrawable(std::make_shared<TransitionState>(nullptr,this)){
    mAlpha = 0;
    mCrossFade = false;
    mTransitionState = TRANSITION_NONE;
}

TransitionDrawable::TransitionDrawable(const std::vector<Drawable*>drawables)
    :LayerDrawable(drawables){
    mAlpha=0;
    mCrossFade =false;
    mTransitionState=TRANSITION_NONE;
}

TransitionDrawable::TransitionDrawable(std::shared_ptr<TransitionState> state)
    :LayerDrawable(state){    
    mAlpha=0;
    mCrossFade =false;
    mTransitionState=TRANSITION_NONE;
}

std::shared_ptr<LayerDrawable::LayerState> TransitionDrawable::createConstantState(LayerState* state,const AttributeSet*attrs){
    return std::make_shared<TransitionState>((TransitionState*) state, this);
}

void TransitionDrawable::startTransition(int durationMillis) {
    mFrom = 0;
    mTo = 255;
    mAlpha = 0;
    mDuration = mOriginalDuration = durationMillis;
    mReverse = false;
    mTransitionState = TRANSITION_STARTING;
    invalidateSelf();
}

void TransitionDrawable::resetTransition() {
    mAlpha = 0;
    mTransitionState = TRANSITION_NONE;
    invalidateSelf();
}

void TransitionDrawable::reverseTransition(int duration) {
    const auto time = SystemClock::uptimeMillis();
    // Animation is over
    if (time - mStartTimeMillis > mDuration) {
        if (mTo == 0) {
           mFrom = 0;
           mTo = 255;
           mAlpha = 0;
           mReverse = false;
        } else {
           mFrom = 255;
           mTo = 0;
           mAlpha = 255;
           mReverse = true;
        }
        mDuration = mOriginalDuration = duration;
        mTransitionState = TRANSITION_STARTING;
        invalidateSelf();
        return;
    }

    mReverse = !mReverse;
    mFrom = mAlpha;
    mTo = mReverse ? 0 : 255;
    mDuration = (int) (mReverse ? time - mStartTimeMillis :
                mOriginalDuration - (time - mStartTimeMillis));
    mTransitionState = TRANSITION_STARTING;
}

bool TransitionDrawable::isCrossFadeEnabled()const{
    return mCrossFade;
}

void TransitionDrawable::setCrossFadeEnabled(bool enabled){
    mCrossFade= enabled;
}

void TransitionDrawable::draw(Canvas&canvas){
    bool done = true;

    switch (mTransitionState) {
    case TRANSITION_STARTING:
        mStartTimeMillis = SystemClock::uptimeMillis();
        done = false;
        mTransitionState = TRANSITION_RUNNING;
        break;
    case TRANSITION_RUNNING:
        if (mStartTimeMillis >= 0) {
            float normalized = (float) (SystemClock::uptimeMillis() - mStartTimeMillis) / mDuration;
            done = normalized >= 1.0f;
            normalized = std::min(normalized, 1.0f);
            mAlpha = (int) (mFrom  + (mTo - mFrom) * normalized);
        }
        break;
    }

    const std::vector<ChildDrawable*>&array = mLayerState->mChildren;

    if (done) {// the setAlpha() calls below trigger invalidation and redraw. If we're done, just draw
               // the appropriate drawable[s] and return
        if (!mCrossFade || mAlpha == 0) array[0]->mDrawable->draw(canvas);
        if (mAlpha == 0xFF)  array[1]->mDrawable->draw(canvas);
        return;
    }
    Drawable*d = array[0]->mDrawable;
    if (mCrossFade) d->setAlpha(255 - mAlpha);

    d->draw(canvas);
    if (mCrossFade) d->setAlpha(0xFF);

    if (mAlpha > 0) {
        d = array[1]->mDrawable;
        d->setAlpha(mAlpha);
        d->draw(canvas);
        d->setAlpha(0xFF);
    }

    if (!done) invalidateSelf();
}

Drawable*TransitionDrawable::inflate(Context*ctx,const AttributeSet&atts){
    return new TransitionDrawable(ctx,atts);
}

}
