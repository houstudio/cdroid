#include <widgetEx/hidebottomviewonscrollbehavior.h>
namespace cdroid{
HideBottomViewOnScrollBehavior::HideBottomViewOnScrollBehavior() {
    mEnterAnimDuration = DEFAULT_ENTER_ANIMATION_DURATION_MS;
    mExitAnimDuration = DEFAULT_EXIT_ANIMATION_DURATION_MS;
    mEnterAnimInterpolator = nullptr;
    mExitAnimInterpolator = nullptr;
}

HideBottomViewOnScrollBehavior::HideBottomViewOnScrollBehavior(Context* context,const AttributeSet& attrs)
    :CoordinatorLayout::Behavior(context, attrs){
    mEnterAnimDuration = attrs.getInt("enterAnimDuration",DEFAULT_ENTER_ANIMATION_DURATION_MS);
    mExitAnimDuration = attrs.getInt("exitAnimDuration",DEFAULT_EXIT_ANIMATION_DURATION_MS);
    mEnterAnimInterpolator = nullptr;
    mExitAnimInterpolator = nullptr;
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

void HideBottomViewOnScrollBehavior::setAdditionalHiddenOffsetY(View& child, int offset) {
    mAdditionalHiddenOffsetY = offset;
    if (mCurrentState == 1) {
        child.setTranslationY((float)(mHeight + mAdditionalHiddenOffsetY));
    }

}

bool HideBottomViewOnScrollBehavior::onStartNestedScroll(CoordinatorLayout& coordinatorLayout, View& child, View& directTargetChild, View& target, int nestedScrollAxes, int type) {
    return nestedScrollAxes == 2;
}

void HideBottomViewOnScrollBehavior::onNestedScroll(CoordinatorLayout& coordinatorLayout, View& child, View& target, int dxConsumed, int dyConsumed, int dxUnconsumed, int dyUnconsumed, int type, int* consumed) {
    if (dyConsumed > 0) {
        slideDown(child);
    } else if (dyConsumed < 0) {
        slideUp(child);
    }

}

bool HideBottomViewOnScrollBehavior::isScrolledUp() const{
    return mCurrentState == 2;
}

void HideBottomViewOnScrollBehavior::slideUp(View& child) {
    slideUp(child, true);
}

void HideBottomViewOnScrollBehavior::slideUp(View& child, bool animate) {
    if (!isScrolledUp()) {
        if (mCurrentAnimator != nullptr) {
            mCurrentAnimator->cancel();
            child.clearAnimation();
        }

        updateCurrentState(child, 2);
        int targetTranslationY = 0;
        if (animate) {
            animateChildTo(child, targetTranslationY, (long)mEnterAnimDuration, mEnterAnimInterpolator);
        } else {
            child.setTranslationY((float)targetTranslationY);
        }

    }
}

bool HideBottomViewOnScrollBehavior::isScrolledDown() const{
    return mCurrentState == 1;
}

void HideBottomViewOnScrollBehavior::slideDown(View& child) {
    slideDown(child, true);
}

void HideBottomViewOnScrollBehavior::slideDown(View& child, bool animate) {
    if (!isScrolledDown()) {
        if (mCurrentAnimator != nullptr) {
            mCurrentAnimator->cancel();
            child.clearAnimation();
        }

        updateCurrentState(child, 1);
        int targetTranslationY = mHeight + mAdditionalHiddenOffsetY;
        if (animate) {
            animateChildTo(child, targetTranslationY, (long)mExitAnimDuration, mExitAnimInterpolator);
        } else {
            child.setTranslationY((float)targetTranslationY);
        }

    }
}

void HideBottomViewOnScrollBehavior::updateCurrentState(View& child, int state) {
    mCurrentState = state;

    for(OnScrollStateChangedListener& listener : mOnScrollStateChangedListeners) {
        listener/*.onStateChanged*/(child, mCurrentState);
    }

}

void HideBottomViewOnScrollBehavior::animateChildTo(View& child, int targetY, long duration, TimeInterpolator* interpolator) {
    Animator::AnimatorListener al;
    al.onAnimationEnd=[this](Animator& animation,bool){
        mCurrentAnimator = nullptr;
    };
    mCurrentAnimator = &child.animate().translationY((float)targetY);
    mCurrentAnimator->setInterpolator(interpolator).setDuration(duration).setListener(al);
    /*new AnimatorListenerAdapter() {
        public void onAnimationEnd(Animator& animation) {
            HideBottomViewOnScrollBehavior.this.mCurrentAnimator = null;
        }
    });*/
}

void HideBottomViewOnScrollBehavior::addOnScrollStateChangedListener(const HideBottomViewOnScrollBehavior::OnScrollStateChangedListener& listener) {
    auto it = std::find(mOnScrollStateChangedListeners.begin(),mOnScrollStateChangedListeners.end(),listener);
    if(it==mOnScrollStateChangedListeners.end()){
        mOnScrollStateChangedListeners.push_back(listener);
    }
}

void HideBottomViewOnScrollBehavior::removeOnScrollStateChangedListener(const HideBottomViewOnScrollBehavior::OnScrollStateChangedListener& listener) {
    /*auto it = std::find(mOnScrollStateChangedListeners.begin(),mOnScrollStateChangedListeners.end(),listener);
    if(it!=mOnScrollStateChangedListeners.end()){
        mOnScrollStateChangedListeners.erase(it);
    }*/
}

void HideBottomViewOnScrollBehavior::clearOnScrollStateChangedListeners() {
    mOnScrollStateChangedListeners.clear();
}

}
