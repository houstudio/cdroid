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
#ifndef __FORWARDING_LISTENER_H__
#define __FORWARDING_LISTENER_H__
#include <widget/listview.h>
#include <widget/showablelistmenu.h>
namespace cdroid{
class ForwardingListener{
private:
    /** Scaled touch slop, used for detecting movement outside bounds. */
    float mScaledTouchSlop;
    /** Timeout before disallowing intercept on the source's parent. */
    int mTapTimeout;
    /** Timeout before accepting a long-press to start forwarding. */
    int mLongPressTimeout;
    /** Runnable used to prevent conflicts with scrolling parents. */
    Runnable mDisallowIntercept;
    /** Runnable used to trigger forwarding on long-press. */
    Runnable mTriggerLongPress;
    /** The id of the first pointer down in the current event stream. */
    int mActivePointerId;
    /** Whether this listener is currently forwarding touch events. */
    bool mForwarding;
protected:
    /** Source view from which events are forwarded. */
    View* mSrc;
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
    virtual ~ForwardingListener()=default;
    bool onTouch(View& v, MotionEvent& event);
    void onViewAttachedToWindow(View& v);
    void onViewDetachedFromWindow(View& v);
};
}/*endof namespace*/
#endif/*__FORWARDING_LISTENER_H__*/
