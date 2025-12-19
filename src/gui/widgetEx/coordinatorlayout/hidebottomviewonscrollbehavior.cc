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
#include <widgetEx/coordinatorlayout/hidebottomviewonscrollbehavior.h>
namespace cdroid{
HideBottomViewOnScrollBehavior::HideBottomViewOnScrollBehavior() {
    mEnterAnimDuration = DEFAULT_ENTER_ANIMATION_DURATION_MS;
    mExitAnimDuration = DEFAULT_EXIT_ANIMATION_DURATION_MS;
    mEnterAnimInterpolator = nullptr;
    mExitAnimInterpolator = nullptr;
    mDisableOnTouchExploration = true;
}

HideBottomViewOnScrollBehavior::HideBottomViewOnScrollBehavior(Context* context,const AttributeSet& attrs)
    :CoordinatorLayout::Behavior(context, attrs){
    mEnterAnimDuration = attrs.getInt("enterAnimDuration",DEFAULT_ENTER_ANIMATION_DURATION_MS);
    mExitAnimDuration = attrs.getInt("exitAnimDuration",DEFAULT_EXIT_ANIMATION_DURATION_MS);
    mEnterAnimInterpolator = nullptr;
    mExitAnimInterpolator = nullptr;
    mDisableOnTouchExploration = true;
}

HideBottomViewOnScrollBehavior::~HideBottomViewOnScrollBehavior(){
}

bool HideBottomViewOnScrollBehavior::onLayoutChild(CoordinatorLayout& parent, View& child, int layoutDirection) {
    ViewGroup::MarginLayoutParams* paramsCompat = (ViewGroup::MarginLayoutParams*)child.getLayoutParams();
    mHeight = child.getMeasuredHeight() + paramsCompat->bottomMargin;
    //mEnterAnimDuration = MotionUtils.resolveThemeDuration(child.getContext(), ENTER_ANIM_DURATION_ATTR, 225);
    //mExitAnimDuration = MotionUtils.resolveThemeDuration(child.getContext(), EXIT_ANIM_DURATION_ATTR, 175);
    //mEnterAnimInterpolator = MotionUtils.resolveThemeInterpolator(child.getContext(), ENTER_EXIT_ANIM_EASING_ATTR, AnimationUtils.LINEAR_OUT_SLOW_IN_INTERPOLATOR);
    //mExitAnimInterpolator = MotionUtils.resolveThemeInterpolator(child.getContext(), ENTER_EXIT_ANIM_EASING_ATTR, AnimationUtils.FAST_OUT_LINEAR_IN_INTERPOLATOR);
    return CoordinatorLayout::Behavior::onLayoutChild(parent, child, layoutDirection);
}

void HideBottomViewOnScrollBehavior::setAdditionalHiddenOffsetY(View* child, int offset) {
    mAdditionalHiddenOffsetY = offset;
    if ((child!=nullptr)&&mCurrentState == STATE_SCROLLED_DOWN) {
        child->setTranslationY((float)(mHeight + mAdditionalHiddenOffsetY));
    }

}

bool HideBottomViewOnScrollBehavior::onStartNestedScroll(CoordinatorLayout& coordinatorLayout, View& child, View& directTargetChild, View& target, int nestedScrollAxes, int type) {
    return nestedScrollAxes == View::TYPE_TOUCH/*2*/;
}

void HideBottomViewOnScrollBehavior::onNestedScroll(CoordinatorLayout& coordinatorLayout, View& child, View& target, int dxConsumed, int dyConsumed, int dxUnconsumed, int dyUnconsumed, int type, int* consumed) {
    if (dyConsumed > 0) {
        slideDown(&child);
    } else if (dyConsumed < 0) {
        slideUp(&child);
    }
}

bool HideBottomViewOnScrollBehavior::isScrolledUp() const{
    return mCurrentState == STATE_SCROLLED_UP;
}

void HideBottomViewOnScrollBehavior::slideUp(View* child) {
    slideUp(child, true);
}

void HideBottomViewOnScrollBehavior::slideUp(View* child, bool animate) {
    if (!isScrolledUp()) {
        if (mCurrentAnimator != nullptr) {
            mCurrentAnimator->cancel();
            child->clearAnimation();
        }

        updateCurrentState(child, STATE_SCROLLED_UP);
        int targetTranslationY = 0;
        if (animate) {
            animateChildTo(child, targetTranslationY, (long)mEnterAnimDuration, mEnterAnimInterpolator);
        } else {
            child->setTranslationY((float)targetTranslationY);
        }

    }
}

bool HideBottomViewOnScrollBehavior::isScrolledDown() const{
    return mCurrentState == STATE_SCROLLED_DOWN;
}

void HideBottomViewOnScrollBehavior::slideDown(View* child) {
    slideDown(child, true);
}

void HideBottomViewOnScrollBehavior::slideDown(View* child, bool animate) {
    if (isScrolledDown()){
        return;
    }
     // If Touch Exploration is on, we should disable sliding down due to a11y issues.
    if (mDisableOnTouchExploration
        && mAccessibilityManager != nullptr
        && mAccessibilityManager->isTouchExplorationEnabled()) {
         return;
    }
    if (mCurrentAnimator != nullptr) {
        mCurrentAnimator->cancel();
        child->clearAnimation();
    }

    updateCurrentState(child, STATE_SCROLLED_DOWN);
    int targetTranslationY = mHeight + mAdditionalHiddenOffsetY;
    if (animate) {
        animateChildTo(child, targetTranslationY, (long)mExitAnimDuration, mExitAnimInterpolator);
    } else {
        child->setTranslationY((float)targetTranslationY);
    }

}

void HideBottomViewOnScrollBehavior::updateCurrentState(View* child, int state) {
    mCurrentState = state;

    for(OnScrollStateChangedListener& listener : mOnScrollStateChangedListeners) {
        listener/*.onStateChanged*/(*child, mCurrentState);
    }

}

void HideBottomViewOnScrollBehavior::animateChildTo(View* child, int targetY, long duration, TimeInterpolator* interpolator) {
    Animator::AnimatorListener al;
    al.onAnimationEnd=[this](Animator& animation,bool){
        mCurrentAnimator = nullptr;
    };
    mCurrentAnimator = &child->animate().translationY((float)targetY);
    mCurrentAnimator->setInterpolator(interpolator)
        .setDuration(duration).setListener(al);
}

void HideBottomViewOnScrollBehavior::addOnScrollStateChangedListener(const HideBottomViewOnScrollBehavior::OnScrollStateChangedListener& listener) {
    auto it = std::find(mOnScrollStateChangedListeners.begin(),mOnScrollStateChangedListeners.end(),listener);
    if(it==mOnScrollStateChangedListeners.end()){
        mOnScrollStateChangedListeners.push_back(listener);
    }
}

void HideBottomViewOnScrollBehavior::removeOnScrollStateChangedListener(const HideBottomViewOnScrollBehavior::OnScrollStateChangedListener& listener) {
    auto it = std::find(mOnScrollStateChangedListeners.begin(),mOnScrollStateChangedListeners.end(),listener);
    if(it!=mOnScrollStateChangedListeners.end()){
        mOnScrollStateChangedListeners.erase(it);
    }
}

void HideBottomViewOnScrollBehavior::clearOnScrollStateChangedListeners() {
    mOnScrollStateChangedListeners.clear();
}

/** Sets whether or not to disable this behavior if touch exploration is enabled. */
void HideBottomViewOnScrollBehavior::disableOnTouchExploration(bool disableOnTouchExploration) {
    mDisableOnTouchExploration = disableOnTouchExploration;
}

/** Returns whether or not this behavior is disabled if touch exploration is enabled. */
bool HideBottomViewOnScrollBehavior::isDisabledOnTouchExploration() const{
    return mDisableOnTouchExploration;
}
}
