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

bool TouchDelegate::onTouchExplorationHoverEvent(MotionEvent& event){
    if (mBounds.empty()) {
        return false;
    }

    const int x = (int) event.getX();
    const int y = (int) event.getY();
    bool hit = true;
    bool handled = false;

    const bool isInbound = mBounds.contains(x, y);
    switch (event.getActionMasked()) {
    case MotionEvent::ACTION_HOVER_ENTER:
        mDelegateTargeted = isInbound;
        break;
    case MotionEvent::ACTION_HOVER_MOVE:
        if (isInbound) {
            mDelegateTargeted = true;
        } else { // delegated previously
            if (mDelegateTargeted && !mSlopBounds.contains(x, y)) {
                hit = false;
            }
        }
        break;
    case MotionEvent::ACTION_HOVER_EXIT:
        mDelegateTargeted = true;
        break;
    }
    if (mDelegateTargeted) {
        if (hit) {
            event.setLocation(mDelegateView->getWidth() / 2, mDelegateView->getHeight() / 2);
        } else {
            mDelegateTargeted = false;
        }
        handled = mDelegateView->dispatchHoverEvent(event);
    }
    return handled;
}

}
