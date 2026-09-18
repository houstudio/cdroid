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
#include <drawable/transitiondrawable.h>
#include <core/systemclock.h>
#include <porting/cdlog.h>

namespace cdroid{

TransitionDrawable::TransitionState::TransitionState(TransitionState* orig, TransitionDrawable* owner, Resources* res)
    :LayerState::LayerState(orig,owner,res){
}

TransitionDrawable*TransitionDrawable::TransitionState::newDrawable(){
    return new TransitionDrawable(std::dynamic_pointer_cast<TransitionState>(shared_from_this()), nullptr);
}

Drawable*TransitionDrawable::TransitionState::newDrawable(Resources* res){
    return new TransitionDrawable(std::dynamic_pointer_cast<TransitionState>(shared_from_this()), res);
}

int TransitionDrawable::TransitionState::getChangingConfigurations()const{
    return mChangingConfigurations;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
TransitionDrawable::TransitionDrawable()
    :LayerDrawable(std::make_shared<TransitionState>(nullptr,this,nullptr)){
}

TransitionDrawable::TransitionDrawable(const std::vector<Drawable*>drawables)
    :TransitionDrawable(std::make_shared<TransitionState>(nullptr,this,nullptr), nullptr){
    // AOSP TransitionDrawable(Drawable[]) builds a TransitionState (not the plain LayerState that
    // LayerDrawable(layers) would create), so getConstantState()->newDrawable() yields a
    // TransitionDrawable and the ConstantState round-trip / copy-on-write mutate stay typed.
    for(auto d:drawables){
        addLayer(d);
    }
    ensurePadding();
    refreshPadding();
}

TransitionDrawable::TransitionDrawable(std::shared_ptr<TransitionState> state, Resources* res)
    :LayerDrawable(){
    // AOSP TransitionDrawable(TransitionState, Resources) → super(state, res) →
    // LayerDrawable ctor → createConstantState() → LayerState copy ctor, which
    // deep-copies every ChildDrawable (children re-created via their own
    // ConstantState). Passing the shared state to the LayerDrawable ctor adopts
    // it instead, so every clone from the drawable cache shared one children
    // array and one view's transition alpha/level poisoned its siblings.
    mLayerState = std::make_shared<TransitionState>(state.get(), this, nullptr);
    if (!mLayerState->mChildren.empty()) {
        ensurePadding();
        refreshPadding();
    }
    mAlpha = 0;
    mReverse = false;
    mCrossFade = false;
    mTransitionState = TRANSITION_NONE;
}

std::shared_ptr<LayerDrawable::LayerState> TransitionDrawable::createConstantState(LayerState* state,Resources*res){
    return std::make_shared<TransitionState>((TransitionState*) state, this, res);
}

void TransitionDrawable::showSecondLayer() {
    // AOSP TransitionDrawable.showSecondLayer(): display the second layer
    // immediately, canceling any in-flight transition.
    mAlpha = 255;
    mReverse = false;
    mTransitionState = TRANSITION_NONE;
    invalidateSelf();
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
    mDuration = (int) (mReverse ? time - mStartTimeMillis : mOriginalDuration - (time - mStartTimeMillis));
    mTransitionState = TRANSITION_STARTING;
}

bool TransitionDrawable::isCrossFadeEnabled()const{
    return mCrossFade;
}

void TransitionDrawable::setCrossFadeEnabled(bool enabled){
    mCrossFade = enabled;
}

int TransitionDrawable::getChangingConfigurations()const{
    return Drawable::getChangingConfigurations() | mLayerState->getChangingConfigurations();
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

    if (done) {
        // the setAlpha() calls below trigger invalidation and redraw. If we're done, just draw
        // the appropriate drawable[s] and return
        if (!mCrossFade || (mAlpha == 0)) array[0]->mDrawable->draw(canvas);
        if (mAlpha == 0xFF) array[1]->mDrawable->draw(canvas);
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

}
