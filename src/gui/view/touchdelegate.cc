#include <view/touchdelegate.h>
#include <view/view.h>
namespace cdroid{
TouchDelegate::TouchDelegate(const Rect& bounds, View* delegateView) {
    mBounds = bounds;

    mSlop = ViewConfiguration::get(delegateView->getContext()).getScaledTouchSlop();
    mSlopBounds = bounds;
    mSlopBounds.inset(-mSlop, -mSlop);
    mDelegateView = delegateView;
}

bool TouchDelegate::onTouchEvent(MotionEvent& event) {
    int x = (int)event.getX();
    int y = (int)event.getY();
    bool sendToDelegate = false;
    bool hit = true;
    bool handled = false;

    switch (event.getActionMasked()) {
    case MotionEvent::ACTION_DOWN:
        mDelegateTargeted = mBounds.contains(x, y);
        sendToDelegate = mDelegateTargeted;
        break;
    case MotionEvent::ACTION_POINTER_DOWN:
    case MotionEvent::ACTION_POINTER_UP:
    case MotionEvent::ACTION_UP:
    case MotionEvent::ACTION_MOVE:
        sendToDelegate = mDelegateTargeted;
        if (sendToDelegate) {
            const Rect slopBounds = mSlopBounds;
            if (!slopBounds.contains(x, y)) {
                hit = false;
            }
        }
        break;
    case MotionEvent::ACTION_CANCEL:
        sendToDelegate = mDelegateTargeted;
        mDelegateTargeted = false;
        break;
    }
    if (sendToDelegate) {
        View* delegateView = mDelegateView;

        if (hit) {
            // Offset event coordinates to be inside the target view
            event.setLocation(delegateView->getWidth() / 2, delegateView->getHeight() / 2);
        } else {
            // Offset event coordinates to be outside the target view (in case it does
            // something like tracking pressed state)
            const int slop = mSlop;
            event.setLocation(-(slop * 2), -(slop * 2));
        }
        handled = delegateView->dispatchTouchEvent(event);
    }
    return handled;
}
}
