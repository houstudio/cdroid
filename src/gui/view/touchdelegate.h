#ifndef __TOUCH_DELEGATE_H__
#define __TOUCH_DELEGATE_H__
#include <core/rect.h>
#include <view/motionevent.h>
namespace cdroid{
class View;
class TouchDelegate {
private:
    /**
     * View that should receive forwarded touch events
     */
    View* mDelegateView;

    /**
     * Bounds in local coordinates of the containing view that should be mapped to the delegate
     * view. This rect is used for initial hit testing.
     */
    Rect mBounds;

    /**
     * mBounds inflated to include some slop. This rect is to track whether the motion events
     * should be considered to be within the delegate view.
     */
    Rect mSlopBounds;

    /**
     * True if the delegate had been targeted on a down event (intersected mBounds).
     */
    bool mDelegateTargeted;
    int mSlop;
public:
    /**
     * The touchable region of the View extends above its actual extent.
     */
    static constexpr int ABOVE = 1;

    /**
     * The touchable region of the View extends below its actual extent.
     */
    static constexpr int BELOW = 2;

    /**
     * The touchable region of the View extends to the left of its actual extent.
     */
    static constexpr int TO_LEFT = 4;

    /**
     * The touchable region of the View extends to the right of its actual extent.
     */
    static constexpr int TO_RIGHT = 8;
public:
    /**
     * Constructor
     *
     * @param bounds Bounds in local coordinates of the containing view that should be mapped to
     *        the delegate view
     * @param delegateView The view that should receive motion events
     */
    TouchDelegate(const Rect& bounds, View* delegateView);

    /**
     * Will forward touch events to the delegate view if the event is within the bounds
     * specified in the constructor.
     *
     * @param event The touch event to forward
     * @return True if the event was forwarded to the delegate, false otherwise.
     */
    virtual bool onTouchEvent(MotionEvent& event);
};

}/*endof namespace*/

#endif/*__TOUCH_DELEGATE_H__*/
