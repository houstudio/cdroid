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
#include <drawables/propertyvaluesanimatorset.h>
#include <algorithm>
#ifdef ENABLE_VECTOR_RENDER_THREAD
namespace cdroid {
namespace hwui {

PropertyValuesAnimatorSet::PropertyValuesAnimatorSet()/* : BaseRenderNodeAnimator(1.0f) */{
    setStartValue(0);
    mStartDelay =0;
    mLastFraction = 0.0f;
    setInterpolator(new LinearInterpolator());
    //setListener(new PropertyAnimatorSetListener(this));
}

void PropertyValuesAnimatorSet::setInterpolator(Interpolator* interpolator){
}

void PropertyValuesAnimatorSet::setStartValue(float value){
}

void PropertyValuesAnimatorSet::setDuration(int64_t duration){
   mDuration =duration;
}
void PropertyValuesAnimatorSet::setStartDelay(int64_t startDelay){
   mStartDelay = startDelay;
}

void PropertyValuesAnimatorSet::addPropertyAnimator(PropertyValuesHolder* propertyValuesHolder,
        TimeInterpolator* interpolator, int64_t startDelay, int64_t duration, int repeatCount,int repeatMode) {
    PropertyAnimator* animator = new PropertyAnimator(
            propertyValuesHolder, interpolator, startDelay, duration, repeatCount, repeatMode);
    mAnimators.emplace_back(animator);

    mStartDelay = 0;
    // Check whether any child animator is infinite after adding it them to the set.
    if (repeatCount == -1) {
        mIsInfinite = true;
    }
}
/*void PropertyValuesAnimatorSet::onFinished(BaseRenderNodeAnimator* animator) {
    if (mOneShotListener.get()) {
        sp<AnimationListener> listener = std::move(mOneShotListener);
        // Set the listener to nullptr before the onAnimationFinished callback, rather than after,
        // for two reasons:
        // 1) We need to prevent changes to mOneShotListener during the onAnimationFinished
        // callback (specifically in AnimationListenerBridge::onAnimationFinished(...) from
        // triggering dtor of the bridge and potentially unsafely re-entering
        // AnimationListenerBridge::onAnimationFinished(...).
        // 2) It's possible that there are changes to the listener during the callback, therefore
        // we need to reset the listener before the callback rather than afterwards.
        mOneShotListener = nullptr;
        listener->onAnimationFinished(animator);
    }
}

float PropertyValuesAnimatorSet::getValue(RenderNode* target) const {
    return mLastFraction;
}

void PropertyValuesAnimatorSet::setValue(RenderNode* target, float value) {
    mLastFraction = value;
}*/

void PropertyValuesAnimatorSet::onPlayTimeChanged(int64_t playTime) {
    if (playTime == 0 && mDuration > 0) {
        // Reset all the animators
        for (auto it = mAnimators.rbegin(); it != mAnimators.rend(); it++) {
            // Note that this set may containing animators modifying the same property, so when we
            // reset the animators, we need to make sure the animators that end the first will
            // have the final say on what the property value should be.
            (*it)->setFraction(0, 0);
        }
    } else {
        for (auto& anim : mAnimators) {
            anim->setCurrentPlayTime(playTime);
        }
    }
}

/*void PropertyValuesAnimatorSet::start(const Animator::AnimationListener& listener) {
    init();
    mOneShotListener = listener;
    mRequestId++;
    BaseRenderNodeAnimator::start();
}

void PropertyValuesAnimatorSet::reverse(const Animator::AnimationListener& listener) {
    init();
    mOneShotListener = listener;
    mRequestId++;
    BaseRenderNodeAnimator::reverse();
}*/

void PropertyValuesAnimatorSet::reset() {
    mRequestId++;
    //BaseRenderNodeAnimator::reset();
}

void PropertyValuesAnimatorSet::end() {
    mRequestId++;
    //BaseRenderNodeAnimator::end();
}

void PropertyValuesAnimatorSet::init() {
    if (mInitialized) {
        return;
    }

    // Sort the animators by their total duration. Note that all the animators in the set start at
    // the same time, so the ones with longer total duration (which includes start delay) will
    // be the ones that end later.
    std::sort(mAnimators.begin(), mAnimators.end(),
              [](auto& a, auto& b) { return a->getTotalDuration() < b->getTotalDuration(); });
    mDuration = mAnimators.empty() ? 0 : mAnimators[mAnimators.size() - 1]->getTotalDuration();
    mInitialized = true;
}

/*uint32_t PropertyValuesAnimatorSet::dirtyMask() {
    return RenderNode::DISPLAY_LIST;
}*/

PropertyAnimator::PropertyAnimator(PropertyValuesHolder* holder, TimeInterpolator* interpolator,
                    int64_t startDelay, int64_t duration, int repeatCount,int repeatMode)
        : mPropertyValuesHolder(holder)
        , mInterpolator(interpolator)
        , mStartDelay(startDelay)
        , mDuration(duration) {
    if (repeatCount < 0) {
        mRepeatCount = UINT32_MAX;
    } else {
        mRepeatCount = repeatCount;
    }
    mRepeatMode = repeatMode;
    mTotalDuration = ((int64_t)mRepeatCount + 1) * mDuration + mStartDelay;
}

void PropertyAnimator::setCurrentPlayTime(int64_t playTime) {
    if (playTime < mStartDelay) {
        return;
    }

    float currentIterationFraction;
    int64_t iteration;
    if (playTime >= mTotalDuration) {
        // Reached the end of the animation.
        iteration = mRepeatCount;
        currentIterationFraction = 1.0f;
    } else {
        // play time here is in range [mStartDelay, mTotalDuration)
        iteration = (playTime - mStartDelay) / mDuration;
        currentIterationFraction = ((playTime - mStartDelay) % mDuration) / (float)mDuration;
    }
    LOGD("currentIterationFraction=%.3f",currentIterationFraction);
    setFraction(currentIterationFraction, iteration);
}

void PropertyAnimator::setFraction(float fraction, int64_t iteration) {
    double totalFraction = fraction + iteration;
    // This makes sure we only set the fraction = repeatCount + 1 once. It is needed because there
    // might be another animator modifying the same property after this animator finishes, we need
    // to make sure we don't set conflicting values on the same property within one frame.
    if ((mLatestFraction == mRepeatCount + 1.0) && (totalFraction >= mRepeatCount + 1.0)) {
        return;
    }

    mLatestFraction = totalFraction;
    // Check the play direction (i.e. reverse or restart) every other iteration, and calculate the
    // fraction based on the play direction.
    if (iteration % 2 && mRepeatMode == ValueAnimator::REVERSE) {
        fraction = 1.0f - fraction;
    }
    float interpolatedFraction = mInterpolator->getInterpolation(fraction);
    //mPropertyValuesHolder->setFraction(interpolatedFraction);
}

/*void PropertyAnimatorSetListener::onAnimationFinished(BaseRenderNodeAnimator* animator) {
    mSet->onFinished(animator);
}*/
}
}
#endif/*ENABLE_VECTOR_RENDER_THREAD*/
