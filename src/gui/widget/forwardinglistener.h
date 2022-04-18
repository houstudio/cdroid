#ifndef __FORWARDING_LISTENER_H__
#define __FORWARDING_LISTENER_H__
#include <widget/listview.h>
namespace cdroid{
typedef struct{
    std::function<void()>show;
    std::function<void()>dismiss;
    std::function<bool()>isShowing;
    std::function<ListView*()>getListView;
}ShowableListMenu;
class ForwardingListener{
private:
    /** Scaled touch slop, used for detecting movement outside bounds. */
    float mScaledTouchSlop;

    /** Timeout before disallowing intercept on the source's parent. */
    int mTapTimeout;

    /** Timeout before accepting a long-press to start forwarding. */
    int mLongPressTimeout;

    /** Source view from which events are forwarded. */
    View* mSrc;

    /** Runnable used to prevent conflicts with scrolling parents. */
    Runnable mDisallowIntercept;

    /** Runnable used to trigger forwarding on long-press. */
    Runnable mTriggerLongPress;

    /** Whether this listener is currently forwarding touch events. */
    bool mForwarding;

    /** The id of the first pointer down in the current event stream. */
    int mActivePointerId;
private:
    bool onTouchObserved(MotionEvent& srcEvent);
    void clearCallbacks();
    void onLongPress();
    bool onTouchForwarded(MotionEvent& srcEvent);
protected:
    virtual bool onForwardingStarted();
    virtual bool onForwardingStopped();
    /*ShowableListMenu*/
    virtual ShowableListMenu getPopup()=0;
public:
    ForwardingListener(View* src);
    bool onTouch(View* v, MotionEvent& event);
    void onViewAttachedToWindow(View* v);
    void onViewDetachedFromWindow(View* v);
};
}//endof namespace
#endif
