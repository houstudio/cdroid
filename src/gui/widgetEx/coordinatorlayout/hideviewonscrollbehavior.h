#ifndef __HIDEVIEW_ONSCROLL_BEHAVIOR_H__
#define __HIDEVIEW_ONSCROLL_BEHAVIOR_H__
#include <widgetEx/coordinatorlayout/coordinatorlayout.h>
#include <widgetEx/coordinatorlayout/hideviewonscrolldelegate.h>
namespace cdroid{
class HideViewOnScrollBehavior:public CoordinatorLayout::Behavior {
public:
     /** The sheet slides out from the right edge of the screen. */
    static constexpr int EDGE_RIGHT = 0;

    /** The sheet slides out from the bottom edge of the screen. */
    static constexpr int EDGE_BOTTOM = 1;

    /** The sheet slides out from the left edge of the screen. */
    static constexpr int EDGE_LEFT = 2;

    /** State of the view when it's scrolled out. */
    static constexpr int STATE_SCROLLED_OUT = 1;

    /** State of the view when it's scrolled in. */
    static constexpr int STATE_SCROLLED_IN = 2;
    using OnScrollStateChangedListener = CallbackBase<void,View&,int>;/*onStateChanged(View& view,int newState);*/
private:
    HideViewOnScrollDelegate* mHideOnScrollViewDelegate;
    AccessibilityManager* mAccessibilityManager;
    //TouchExplorationStateChangeListener* mTouchExplorationListener;

    std::vector<OnScrollStateChangedListener> mOnScrollStateChangedListeners;

    static constexpr int DEFAULT_ENTER_ANIMATION_DURATION_MS = 225;
    static constexpr int DEFAULT_EXIT_ANIMATION_DURATION_MS = 175;

    int mEnterAnimDuration;
    int mExitAnimDuration;
    int mSize = 0;
    int mCurrentState = STATE_SCROLLED_IN;
    int mAdditionalHiddenOffset = 0;
    TimeInterpolator* mEnterAnimInterpolator;
    TimeInterpolator* mExitAnimInterpolator;
    ViewPropertyAnimator* mCurrentAnimator;

    bool mDisableOnTouchExploration = true;
    bool mViewEdgeOverride = false;
private:
    void setViewEdge(View* view, int layoutDirection);
    void setViewEdgeInternal(int viewEdge);
    bool isGravityBottom(int viewGravity) const;
    bool isGravityLeft(int viewGravity) const;
    void disableIfTouchExplorationEnabled(View* child);
    void updateCurrentState(View* child, int state);
    void animateChildTo(View* child, int targetTranslation, long duration, TimeInterpolator* interpolator);
public:
    HideViewOnScrollBehavior();
    HideViewOnScrollBehavior(int viewEdge);
    HideViewOnScrollBehavior(Context* context, const AttributeSet& attrs);
    ~HideViewOnScrollBehavior()override;

    void setViewEdge(int viewEdge);

    bool onLayoutChild(CoordinatorLayout& parent, View& child, int layoutDirection)override;

    /**
     * Sets an additional offset used to hide the view.
     *
     * @param child the child view that is hidden by this behavior
     * @param offset the additional offset in pixels that should be added when the view slides away
     */
    void setAdditionalHiddenOffset(View* child, int offset);

    bool onStartNestedScroll(CoordinatorLayout& coordinatorLayout, View& child,
          View& directTargetChild, View& target, int nestedScrollAxes,int type) override;

    void onNestedScroll(CoordinatorLayout& coordinatorLayout, View& child, View& target,
          int dxConsumed, int dyConsumed, int dxUnconsumed, int dyUnconsumed, int type, int*consumed) override;

    /** Returns true if the current state is scrolled in. */
    bool isScrolledIn() const;

    /**
     * Performs an animation that will slide the child from its current position to be totally on the
     * screen.
     */
    void slideIn(View* child);

    /**
     * Slides the child with or without animation from its current position to be totally on the
     * screen.
     *
     * @param animate {@code true} to slide with animation.
     */
    void slideIn(View* child, bool animate);

    /** Returns true if the current state is scrolled out. */
    bool isScrolledOut() const;

    /**
     * Performs an animation that will slide the child from it's current position to be totally off
     * the screen.
     */
    void slideOut(View* child);

    /**
     * Slides the child with or without animation from its current position to be totally off the
     * screen.
     *
     * @param animate {@code true} to slide with animation.
     */
    void slideOut(View* child, bool animate);

    /**
     * Adds a listener to be notified of View scroll state changes.
     *
     * @param listener The listener to notify when View scroll state changes.
     */
    void addOnScrollStateChangedListener(const OnScrollStateChangedListener& listener);
    void removeOnScrollStateChangedListener(const OnScrollStateChangedListener& listener);
    void clearOnScrollStateChangedListeners();

    /**
     * Sets whether or not to disable this behavior if touch exploration is enabled.
     */
    void disableOnTouchExploration(bool disableOnTouchExploration);
    bool isDisabledOnTouchExploration()const;

    static HideViewOnScrollBehavior* from(View* view);
};
}/*endof namespace*/
#endif/*__HIDEVIEW_ONSCROLL_BEHAVIOR_H__*/
