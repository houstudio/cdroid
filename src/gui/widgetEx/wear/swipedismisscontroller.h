#ifndef __SWIPE_DISMISS_CONTROLLER_H__
#define __SWIPE_DISMISS_CONTROLLER_H__
#include <widgetEx/wear/dismisscontroller.h>
#include <widgetEx/wear/dismissableframelayout.h>

namespace cdroid{
class DismissibleFrameLayout;
class BackButtonDismissController;
class SwipeDismissTransitionHelper;
class SwipeDismissController:public DismissController {
private:
    static constexpr float EDGE_SWIPE_THRESHOLD = 0.1f;
    static constexpr int VELOCITY_UNIT = 1000;
public:
    static constexpr float DEFAULT_DISMISS_DRAG_WIDTH_RATIO = 0.33f;
private:
    int mSlop;
    int mMinFlingVelocity;
    int mActiveTouchId;
    float mDownX;
    float mDownY;
    float mLastX;
    float mGestureThresholdPx;
    float mDismissMinDragWidthRatio = DEFAULT_DISMISS_DRAG_WIDTH_RATIO;
    bool mSwiping;
    bool mDismissed;
    bool mDiscardIntercept;
    bool mBlockGesture = false;
    SwipeDismissTransitionHelper* mSwipeDismissTransitionHelper;
private:
    bool isPotentialSwipe(float dx, float dy)const;
    void resetSwipeDetectMembers();
    void updateSwiping(MotionEvent& ev);
    void updateDismiss(MotionEvent& ev);
    void checkGesture(MotionEvent& ev);
protected:
     bool canScroll(View* v, bool checkV, float dx, float x, float y);
public:
    SwipeDismissController(Context* context, DismissibleFrameLayout* layout);
    ~SwipeDismissController()override;

    void requestDisallowInterceptTouchEvent(bool disallowIntercept);

    void setDismissMinDragWidthRatio(float ratio);
    float getDismissMinDragWidthRatio() const;

    bool onInterceptTouchEvent(MotionEvent& ev);

    bool canScrollHorizontally(int direction);

    bool onTouchEvent(MotionEvent& ev);
};
}/*endof namespace*/
#endif/*__SWIPE_DISMISS_CONTROLLER_H__*/
