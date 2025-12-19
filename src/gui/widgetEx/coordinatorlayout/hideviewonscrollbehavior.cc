#include <widgetEx/coordinatorlayout/hideviewonscrollbehavior.h>
#include <widgetEx/coordinatorlayout/hideleftviewonscrolldelegate.h>
#include <widgetEx/coordinatorlayout/hiderightviewonscrolldelegate.h>
#include <widgetEx/coordinatorlayout/hidebottomviewonscrolldelegate.h>
namespace cdroid{
HideViewOnScrollBehavior::HideViewOnScrollBehavior() {}

HideViewOnScrollBehavior::HideViewOnScrollBehavior(int viewEdge):HideViewOnScrollBehavior(){
    setViewEdge(viewEdge);
}

HideViewOnScrollBehavior::HideViewOnScrollBehavior(Context* context, const AttributeSet& attrs)
    :CoordinatorLayout::Behavior(context, attrs){
}

HideViewOnScrollBehavior::~HideViewOnScrollBehavior(){
}

void HideViewOnScrollBehavior::setViewEdge(View* view, int layoutDirection) {
  if (mViewEdgeOverride) {
      return;
  }

  const CoordinatorLayout::LayoutParams* params = (CoordinatorLayout::LayoutParams*) view->getLayoutParams();
  int viewGravity = params->gravity;

  if (isGravityBottom(viewGravity)) {
      setViewEdgeInternal(EDGE_BOTTOM);
  } else {
      viewGravity = Gravity::getAbsoluteGravity(viewGravity, layoutDirection);
      setViewEdgeInternal(isGravityLeft(viewGravity) ? EDGE_LEFT : EDGE_RIGHT);
  }
}

void HideViewOnScrollBehavior::setViewEdge(int viewEdge) {
    mViewEdgeOverride = true;
    setViewEdgeInternal(viewEdge);
}

void HideViewOnScrollBehavior::setViewEdgeInternal(int viewEdge) {
    if((mHideOnScrollViewDelegate!=nullptr)&&(mHideOnScrollViewDelegate->getViewEdge() == viewEdge)){
        return;
    }
    switch (viewEdge) {
    case EDGE_RIGHT:
        mHideOnScrollViewDelegate = new HideRightViewOnScrollDelegate();
        break;
    case EDGE_BOTTOM:
        mHideOnScrollViewDelegate = new HideBottomViewOnScrollDelegate();
        break;
    case EDGE_LEFT:
        mHideOnScrollViewDelegate = new HideLeftViewOnScrollDelegate();
        break;
    default:
        LOGE("Invalid view edge position value: %d . Must be EDGE_RIGHT(%d),EDGE_BOTTOM(%d) or EDGE_LEFT(%d).",
                viewEdge,EDGE_RIGHT,EDGE_BOTTOM,EDGE_LEFT);
    }
}

bool HideViewOnScrollBehavior::isGravityBottom(int viewGravity) const{
    return (viewGravity == Gravity::BOTTOM) || (viewGravity == (Gravity::BOTTOM | Gravity::CENTER));
}

bool HideViewOnScrollBehavior::isGravityLeft(int viewGravity) const{
    return viewGravity == Gravity::LEFT || (viewGravity == (Gravity::LEFT | Gravity::CENTER));
}

void HideViewOnScrollBehavior::disableIfTouchExplorationEnabled(View* child) {
#if 0
    if (mAccessibilityManager == nullptr) {
        mAccessibilityManager = getSystemService(child.getContext(), AccessibilityManager.class);
    }
    if (mAccessibilityManager != nullptr && mTouchExplorationListener == nullptr) {
        mTouchExplorationListener =
            enabled -> {
              if (mDisableOnTouchExploration && enabled && isScrolledOut()) {
                  slideIn(child);
              }
            };
      mAccessibilityManager->addTouchExplorationStateChangeListener(mTouchExplorationListener);
      child.addOnAttachStateChangeListener(
          new OnAttachStateChangeListener() {
            public void onViewAttachedToWindow(@NonNull View v) {}
            public void onViewDetachedFromWindow(@NonNull View v) {
              if (mTouchExplorationListener != null && mAccessibilityManager != null) {
                  mAccessibilityManager.removeTouchExplorationStateChangeListener(
                      mTouchExplorationListener);
                  mTouchExplorationListener = null;
              }
            }
          });
    }
#endif
}

bool HideViewOnScrollBehavior::onLayoutChild(CoordinatorLayout& parent, View& child, int layoutDirection) {

    disableIfTouchExplorationEnabled(&child);

    ViewGroup::MarginLayoutParams* marginParams =
        (ViewGroup::MarginLayoutParams*) child.getLayoutParams();
    setViewEdge(&child, layoutDirection);

    mSize = mHideOnScrollViewDelegate->getSize(child, marginParams);

    /*mEnterAnimDuration = MotionUtils.resolveThemeDuration(
            child.getContext(), ENTER_ANIM_DURATION_ATTR, DEFAULT_ENTER_ANIMATION_DURATION_MS);
    mExitAnimDuration =  MotionUtils.resolveThemeDuration(
            child.getContext(), EXIT_ANIM_DURATION_ATTR, DEFAULT_EXIT_ANIMATION_DURATION_MS);
    mEnterAnimInterpolator =  MotionUtils.resolveThemeInterpolator(
            child.getContext(), ENTER_EXIT_ANIM_EASING_ATTR,
            AnimationUtils.LINEAR_OUT_SLOW_IN_INTERPOLATOR);
    mExitAnimInterpolator =  MotionUtils.resolveThemeInterpolator(
            child.getContext(), ENTER_EXIT_ANIM_EASING_ATTR,
            AnimationUtils.FAST_OUT_LINEAR_IN_INTERPOLATOR);*/
    return CoordinatorLayout::Behavior::onLayoutChild(parent, child, layoutDirection);
}

/**
 * Sets an additional offset used to hide the view.
 *
 * @param child the child view that is hidden by this behavior
 * @param offset the additional offset in pixels that should be added when the view slides away
 */
void HideViewOnScrollBehavior::setAdditionalHiddenOffset(View* child, int offset) {
    mAdditionalHiddenOffset = offset;

    if (mCurrentState == STATE_SCROLLED_OUT) {
        mHideOnScrollViewDelegate->setAdditionalHiddenOffset(*child, mSize, mAdditionalHiddenOffset);
    }
}

bool HideViewOnScrollBehavior::onStartNestedScroll(CoordinatorLayout& coordinatorLayout, View& child,
          View& directTargetChild, View& target, int nestedScrollAxes,int type) {
    return nestedScrollAxes == View::SCROLL_AXIS_VERTICAL;
}

void HideViewOnScrollBehavior::onNestedScroll(CoordinatorLayout& coordinatorLayout, View& child, View& target,
          int dxConsumed, int dyConsumed, int dxUnconsumed, int dyUnconsumed, int type, int*consumed) {
    if (dyConsumed > 0) {
        slideOut(&child);
    } else if (dyConsumed < 0) {
        slideIn(&child);
    }
}

/** Returns true if the current state is scrolled in. */
bool HideViewOnScrollBehavior::isScrolledIn() const{
    return mCurrentState == STATE_SCROLLED_IN;
}

void HideViewOnScrollBehavior::slideIn(View* child) {
    slideIn(child, /* animate= */ true);
}

void HideViewOnScrollBehavior::slideIn(View* child, bool animate) {
    if (isScrolledIn()) {
        return;
    }

    if (mCurrentAnimator != nullptr) {
        mCurrentAnimator->cancel();
        child->clearAnimation();
    }
    updateCurrentState(child, STATE_SCROLLED_IN);
    const int targetTranslation = mHideOnScrollViewDelegate->getTargetTranslation();

    if (animate) {
        animateChildTo(child, targetTranslation, mEnterAnimDuration, mEnterAnimInterpolator);
    } else {
        mHideOnScrollViewDelegate->setViewTranslation(*child, targetTranslation);
    }
}

bool HideViewOnScrollBehavior::isScrolledOut() const{
    return mCurrentState == STATE_SCROLLED_OUT;
}

void HideViewOnScrollBehavior::slideOut(View* child) {
    slideOut(child, /* animate= */ true);
}

void HideViewOnScrollBehavior::slideOut(View* child, bool animate) {
    if (isScrolledOut()) {
        return;
    }

    // If Touch Exploration is on, we prevent sliding out due to a11y issues.
    if (mDisableOnTouchExploration
        && mAccessibilityManager != nullptr
        && mAccessibilityManager->isTouchExplorationEnabled()) {
        return;
    }

    if (mCurrentAnimator != nullptr) {
        mCurrentAnimator->cancel();
        child->clearAnimation();
    }
    updateCurrentState(child, STATE_SCROLLED_OUT);
    int targetTranslation = mSize + mAdditionalHiddenOffset;
    if (animate) {
        animateChildTo(child, targetTranslation, mExitAnimDuration, mExitAnimInterpolator);
    } else {
        mHideOnScrollViewDelegate->setViewTranslation(*child, targetTranslation);
    }
}

void HideViewOnScrollBehavior::updateCurrentState(View* child, int state) {
    mCurrentState = state;
    for (OnScrollStateChangedListener listener : mOnScrollStateChangedListeners) {
        listener/*.onStateChanged*/(*child, mCurrentState);
    }
}

void HideViewOnScrollBehavior::animateChildTo( View* child, int targetTranslation, long duration, TimeInterpolator* interpolator) {
    mCurrentAnimator = mHideOnScrollViewDelegate->getViewTranslationAnimator(*child, targetTranslation);
    Animator::AnimatorListener al;
    al.onAnimationEnd=[this](Animator& animation,bool){
        mCurrentAnimator = nullptr;
    };
    mCurrentAnimator->setInterpolator(interpolator)
        .setDuration(duration)
        .setListener(al);
}

void HideViewOnScrollBehavior::addOnScrollStateChangedListener(const OnScrollStateChangedListener& listener) {
    auto it = std::find(mOnScrollStateChangedListeners.begin(),mOnScrollStateChangedListeners.end(),listener);
    if(it==mOnScrollStateChangedListeners.end()){
        mOnScrollStateChangedListeners.push_back(listener);
    }
}

void HideViewOnScrollBehavior::removeOnScrollStateChangedListener(const OnScrollStateChangedListener& listener) {
    auto it = std::find(mOnScrollStateChangedListeners.begin(),mOnScrollStateChangedListeners.end(),listener);
    if(it!=mOnScrollStateChangedListeners.end()){
        mOnScrollStateChangedListeners.erase(it);
    }
}

void HideViewOnScrollBehavior::clearOnScrollStateChangedListeners() {
    mOnScrollStateChangedListeners.clear();
}

void HideViewOnScrollBehavior::disableOnTouchExploration(bool disableOnTouchExploration) {
    mDisableOnTouchExploration = disableOnTouchExploration;
}

bool HideViewOnScrollBehavior::isDisabledOnTouchExploration()const {
    return mDisableOnTouchExploration;
}

HideViewOnScrollBehavior* HideViewOnScrollBehavior::from(View* view) {
    ViewGroup::LayoutParams* params = view->getLayoutParams();
    if (dynamic_cast<CoordinatorLayout::LayoutParams*>(params)==nullptr) {
        throw std::runtime_error("The view is not a child of CoordinatorLayout");
    }
    CoordinatorLayout::Behavior*behavior = ((CoordinatorLayout::LayoutParams*) params)->getBehavior();
    if (dynamic_cast<HideViewOnScrollBehavior*>(behavior)==nullptr) {
        throw std::runtime_error("The view is not associated with HideViewOnScrollBehavior");
    }
    return (HideViewOnScrollBehavior*)behavior;
}
}/*endof namespace*/
