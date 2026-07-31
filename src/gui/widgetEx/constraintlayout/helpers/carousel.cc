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

/*
 * Ported to C++ for CDROID from androidx.constraintlayout.helper.widget.Carousel.
 */
#include <widgetEx/constraintlayout/helpers/carousel.h>

#include <algorithm>
#include <unordered_map>

#include <widgetEx/constraintlayout/motion/motionlayout.h>
#include <widgetEx/constraintlayout/motion/motionscene.h>

DECLARE_WIDGET(Carousel)

namespace cdroid {

Carousel::Carousel(Context* ctx, const AttributeSet& attrs)
    : MotionHelper(ctx, attrs) {
    // The ConstraintHelper base ctor calls init(attrs), but during base construction that virtual
    // call statically binds to ConstraintHelper::init — so only constraint_referenced_ids is parsed
    // and every carousel_* attribute stays at its default (-1): mFirstViewReference missed (mStartIndex
    // stuck at 0), mForwardTransition/mBackwardTransition -1 (updateItems bails before wiring the
    // transitions -> drag never advances the index). Re-invoke init now that *this is fully
    // constructed so it dispatches to Carousel::init — same pattern as MotionEffect/Placeholder.
    // ConstraintHelper::init is idempotent on re-run (mIds cleared then refilled).
    init(attrs);
}

Carousel::Carousel(int width, int height)
    : MotionHelper(width, height) {
}

void Carousel::init(const AttributeSet& attrs) {
    ConstraintHelper::init(attrs); // resolves constraint_referenced_ids into mIds
    mFirstViewReference = attrs.getResourceId("carousel_firstView", -1);
    mBackwardTransition = attrs.getResourceId("carousel_backwardTransition", -1);
    mForwardTransition  = attrs.getResourceId("carousel_forwardTransition", -1);
    mPreviousState      = attrs.getResourceId("carousel_previousState", -1);
    mNextState          = attrs.getResourceId("carousel_nextState", -1);
    static const std::unordered_map<std::string, int> kEmpty = {
        {"visible", View::VISIBLE}, {"invisible", View::INVISIBLE}, {"gone", View::GONE}};
    mEmptyViewBehavior  = attrs.getInt("carousel_emptyViews_behavior", kEmpty, View::INVISIBLE);
    mDampening          = attrs.getFloat("carousel_touchUp_dampeningFactor", 0.9f);
    static const std::unordered_map<std::string, int> kTouchUp = {
        {"immediateStop", (int)TOUCH_UP_IMMEDIATE_STOP}, {"carryOn", (int)TOUCH_UP_CARRY_ON}};
    mTouchUpMode        = attrs.getInt("carousel_touchUpMode", kTouchUp, TOUCH_UP_IMMEDIATE_STOP);
    mVelocityThreshold  = attrs.getFloat("carousel_touchUp_velocityThreshold", 2.0f);
    mInfiniteCarousel   = attrs.getBoolean("carousel_infinite", false);
}

void Carousel::onAttachedToWindow() {
    ConstraintHelper::onAttachedToWindow();
    auto* container = dynamic_cast<MotionLayout*>(getParent());
    if (container == nullptr) return; // Carousel must live inside a MotionLayout
    mMotionLayout = container;

    mList.clear();
    for (int id : mIds) {
        View* view = container->findViewById(id);
        if (mFirstViewReference == id) mStartIndex = (int) mList.size();
        mList.push_back(view);
    }

    // AndroidX: Carousel is a MotionLayout.TransitionListener. In CDROID we register a
    // TransitionListener (EventSet + CallbackBase) whose lambdas forward back into ourselves.
    mListener.onTransitionChange = [this](MotionLayout*, int startId, int, float) {
        mLastStartId = startId;
    };
    mListener.onTransitionCompleted = [this](MotionLayout*, int currentId) {
        onTransitionCompleted(currentId);
    };
    mMotionLayout->addTransitionListener(mListener);

    // In CARRY_ON mode, hand the release momentum to the forward/backward transitions.
    if (mTouchUpMode == TOUCH_UP_CARRY_ON) {
        if (auto* t = mMotionLayout->getTransition(mForwardTransition))
            t->setOnTouchUp(MotionLayout::TOUCH_UP_DECELERATE_AND_COMPLETE);
        if (auto* t = mMotionLayout->getTransition(mBackwardTransition))
            t->setOnTouchUp(MotionLayout::TOUCH_UP_DECELERATE_AND_COMPLETE);
    }
    updateItems();
}

void Carousel::onDetachedFromWindow() {
    ConstraintHelper::onDetachedFromWindow();
    mList.clear();
}

int Carousel::getCount() {
    return mAdapter ? mAdapter->count() : 0;
}

void Carousel::transitionToIndex(int index, int delay) {
    if (mMotionLayout == nullptr) return;
    mTargetIndex = std::max(0, std::min(getCount() - 1, index));
    mAnimateTargetDelay = std::max(0, delay);
    mMotionLayout->setTransitionDuration(mAnimateTargetDelay);
    if (index < mIndex) mMotionLayout->transitionToState(mPreviousState, mAnimateTargetDelay);
    else                mMotionLayout->transitionToState(mNextState, mAnimateTargetDelay);
}

void Carousel::jumpToIndex(int index) {
    mIndex = std::max(0, std::min(getCount() - 1, index));
    refresh();
}

void Carousel::refresh() {
    if (mMotionLayout == nullptr) return;
    const int n = (int) mList.size();
    for (int i = 0; i < n; i++) {
        View* view = mList[i];
        if (view == nullptr) continue;
        if (mAdapter == nullptr || mAdapter->count() == 0) updateViewVisibility(view, mEmptyViewBehavior);
        else                                                updateViewVisibility(view, View::VISIBLE);
    }
    mMotionLayout->rebuildScene();
    updateItems();
}

void Carousel::onTransitionChange(int /*startId*/, int /*endId*/, float /*progress*/) {
    // (the registered lambda records mLastStartId directly; kept for API parity)
}

void Carousel::onTransitionCompleted(int currentId) {
    if (mAdapter == nullptr) return;
    mPreviousIndex = mIndex;
    if (currentId == mNextState)          mIndex++;
    else if (currentId == mPreviousState) mIndex--;
    const int count = mAdapter->count();
    if (mInfiniteCarousel) {
        if (mIndex >= count) mIndex = 0;
        if (mIndex < 0)      mIndex = count - 1;
    } else {
        if (mIndex >= count) mIndex = count - 1;
        if (mIndex < 0)      mIndex = 0;
    }
    if (mPreviousIndex != mIndex) {
        mMotionLayout->post([this] { runUpdate(); });
    }
}

void Carousel::runUpdate() {
    if (mAdapter == nullptr || mMotionLayout == nullptr) return;
    mMotionLayout->setProgress(0);
    updateItems();
    mAdapter->onNewItem(mIndex);
    const float velocity = mMotionLayout->getVelocity();
    if (mTouchUpMode == TOUCH_UP_CARRY_ON && velocity > mVelocityThreshold
            && mIndex < mAdapter->count() - 1) {
        const float v = velocity * mDampening;
        if (mIndex == 0 && mPreviousIndex > mIndex) return;                    // reached the first
        if (mIndex == mAdapter->count() - 1 && mPreviousIndex < mIndex) return; // reached the last
        mMotionLayout->post([this, v] {
            mMotionLayout->touchAnimateTo(MotionLayout::TOUCH_UP_DECELERATE_AND_COMPLETE, 1.0f, v);
        });
    }
}

void Carousel::updateItems() {
    if (mAdapter == nullptr || mMotionLayout == nullptr || mAdapter->count() == 0) return;
    const int viewCount = (int) mList.size();
    for (int i = 0; i < viewCount; i++) {
        View* view = mList[i];
        if (view == nullptr) continue;
        // mIndex maps to i == mStartIndex; other pool slots show mIndex ± offset.
        int index = mIndex + i - mStartIndex;
        if (mInfiniteCarousel) {
            if (index < 0) {
                updateViewVisibility(view, mEmptyViewBehavior != View::INVISIBLE ? mEmptyViewBehavior
                                                                                 : View::VISIBLE);
                const int c = mAdapter->count();
                mAdapter->populate(view, (index % c == 0) ? 0 : c + (index % c));
            } else if (index >= mAdapter->count()) {
                if (index == mAdapter->count()) index = 0;
                else if (index > mAdapter->count()) index = index % mAdapter->count();
                updateViewVisibility(view, mEmptyViewBehavior != View::INVISIBLE ? mEmptyViewBehavior
                                                                                 : View::VISIBLE);
                mAdapter->populate(view, index);
            } else {
                updateViewVisibility(view, View::VISIBLE);
                mAdapter->populate(view, index);
            }
        } else {
            if (index < 0 || index >= mAdapter->count()) {
                updateViewVisibility(view, mEmptyViewBehavior);
            } else {
                updateViewVisibility(view, View::VISIBLE);
                mAdapter->populate(view, index);
            }
        }
    }

    // Continue toward mTargetIndex if we haven't reached it yet.
    if (mTargetIndex != -1 && mTargetIndex != mIndex) {
        mMotionLayout->post([this] {
            mMotionLayout->setTransitionDuration(mAnimateTargetDelay);
            if (mTargetIndex < mIndex) mMotionLayout->transitionToState(mPreviousState, mAnimateTargetDelay);
            else                       mMotionLayout->transitionToState(mNextState, mAnimateTargetDelay);
        });
    } else if (mTargetIndex == mIndex) {
        mTargetIndex = -1;
    }

    if (mBackwardTransition == -1 || mForwardTransition == -1) return;
    if (mInfiniteCarousel) return;

    const int count = mAdapter->count();
    if (mIndex == 0) {
        enableTransition(mBackwardTransition, false);
    } else {
        enableTransition(mBackwardTransition, true);
        mMotionLayout->setTransition(mBackwardTransition);
    }
    if (mIndex == count - 1) {
        enableTransition(mForwardTransition, false);
    } else {
        enableTransition(mForwardTransition, true);
        mMotionLayout->setTransition(mForwardTransition);
    }
}

bool Carousel::updateViewVisibility(View* view, int visibility) {
    if (mMotionLayout == nullptr || view == nullptr) return false;
    // AndroidX also sets constraint.propertySet.mVisibilityMode = VISIBILITY_MODE_IGNORE on every
    // ConstraintSet so the Motion controller does not override this visibility on apply. CDROID's
    // ConstraintSet exposes no getConstraint(viewId) yet, so we set the view visibility directly.
    const bool changed = view->getVisibility() != visibility;
    view->setVisibility(visibility);
    return changed;
}

bool Carousel::updateViewVisibility(int /*constraintSetId*/, View* view, int visibility) {
    return updateViewVisibility(view, visibility);
}

bool Carousel::enableTransition(int transitionId, bool enable) {
    if (transitionId == -1 || mMotionLayout == nullptr) return false;
    MotionScene::Transition* transition = mMotionLayout->getTransition(transitionId);
    if (transition == nullptr || transition->isEnabled() == enable) return false;
    transition->setEnabled(enable);
    return true;
}

} // namespace cdroid
