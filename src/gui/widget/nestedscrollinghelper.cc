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
#include <widget/nestedscrollinghelper.h>

namespace cdroid{

NestedScrollingParentHelper::NestedScrollingParentHelper(ViewGroup* viewGroup) {
    mNestedScrollAxes=0;
    mViewGroup = viewGroup;
}
 
void NestedScrollingParentHelper::onNestedScrollAccepted(View* child,View* target,
       int axes) {
    onNestedScrollAccepted(child, target, axes, View::TYPE_TOUCH);
}
 
void NestedScrollingParentHelper::onNestedScrollAccepted(View* child,View* target,
         int axes,int type) {
    mNestedScrollAxes = axes;
}
 
int NestedScrollingParentHelper::getNestedScrollAxes()const{
    return mNestedScrollAxes;
}
 
void NestedScrollingParentHelper::onStopNestedScroll(View* target) {
    onStopNestedScroll(target, View::TYPE_TOUCH);
}
	
void NestedScrollingParentHelper::onStopNestedScroll(View* target,int type) {
    mNestedScrollAxes = 0;
}

/////////////////////////////////////////////////////////////////////////////////////

NestedScrollingChildHelper::NestedScrollingChildHelper( View* view) {
    mView = view;
    mNestedScrollingParentTouch = nullptr;
    mNestedScrollingParentNonTouch = nullptr;
    mIsNestedScrollingEnabled = false;
}

void NestedScrollingChildHelper::setNestedScrollingEnabled(bool enabled) {
    if (mIsNestedScrollingEnabled) {
         mView->stopNestedScroll();
    }
    mIsNestedScrollingEnabled = enabled;
}

bool NestedScrollingChildHelper::isNestedScrollingEnabled() const{
    return mIsNestedScrollingEnabled;
}

bool NestedScrollingChildHelper::hasNestedScrollingParent() const{
    return hasNestedScrollingParent(View::TYPE_TOUCH);
}

bool NestedScrollingChildHelper::hasNestedScrollingParent( int type) const{
    return getNestedScrollingParentForType(type) != nullptr;
}

bool NestedScrollingChildHelper::startNestedScroll( int axes) {
    return startNestedScroll(axes, View::TYPE_TOUCH);
}

bool NestedScrollingChildHelper::startNestedScroll( int axes, int type) {
    if (hasNestedScrollingParent(type)) {
        // Already in progress
        return true;
    }
    if (isNestedScrollingEnabled()) {
        ViewGroup* p = mView->getParent();
        View* child = mView;
        while (p != nullptr) {
            if (p->onStartNestedScroll(child, mView, axes/*, type*/)) {
                setNestedScrollingParentForType(type, p);
                p->onNestedScrollAccepted(child, mView, axes/*, type*/);
                return true;
            }
            if (1/*p instanceof View*/) {
                child = (View*) p;
            }
            p = p->getParent();
        }
    }
    return false;
}

void NestedScrollingChildHelper::stopNestedScroll() {
    stopNestedScroll(View::TYPE_TOUCH);
}

void NestedScrollingChildHelper::stopNestedScroll( int type) {
    ViewGroup* parent = getNestedScrollingParentForType(type);
    if (parent != nullptr) {
        parent->onStopNestedScroll(mView/*, type*/);
        setNestedScrollingParentForType(type, nullptr);
    }
}

bool NestedScrollingChildHelper::dispatchNestedScroll(int dxConsumed, int dyConsumed,
        int dxUnconsumed, int dyUnconsumed, int* offsetInWindow) {
    return dispatchNestedScrollInternal(dxConsumed, dyConsumed, dxUnconsumed, dyUnconsumed,
            offsetInWindow, View::TYPE_TOUCH,nullptr);
}

bool NestedScrollingChildHelper::dispatchNestedScroll(int dxConsumed, int dyConsumed,
        int dxUnconsumed, int dyUnconsumed,int* offsetInWindow, int type){
    int consumed[2];
    return dispatchNestedScrollInternal(dxConsumed,dyConsumed,dxUnconsumed,dyUnconsumed,offsetInWindow,type,consumed);
}

bool NestedScrollingChildHelper::dispatchNestedScroll(int dxConsumed, int dyConsumed,
        int dxUnconsumed, int dyUnconsumed,int* offsetInWindow, int type,int*consumed){
    return dispatchNestedScrollInternal(dxConsumed,dyConsumed,dxUnconsumed,dyUnconsumed,offsetInWindow,type,consumed);
}

bool NestedScrollingChildHelper::dispatchNestedScrollInternal(int dxConsumed, int dyConsumed,
        int dxUnconsumed, int dyUnconsumed,int* offsetInWindow, int type,int*consumed) {
    if (isNestedScrollingEnabled()) {
        ViewGroup* parent = getNestedScrollingParentForType(type);
        if (parent == nullptr) {
            return false;
        }

        if (dxConsumed != 0 || dyConsumed != 0 || dxUnconsumed != 0 || dyUnconsumed != 0) {
            int startX = 0;
            int startY = 0;
            if (offsetInWindow != nullptr) {
                mView->getLocationInWindow(offsetInWindow);
                startX = offsetInWindow[0];
                startY = offsetInWindow[1];
            }
            if(consumed==nullptr){
                consumed = mTempNestedScrollConsumed;
                consumed[0] = consumed[1] =0;
            }
	        parent->onNestedScroll(mView, dxConsumed, dyConsumed, dxUnconsumed, dyUnconsumed, type,consumed);

            if (offsetInWindow != nullptr) {
                mView->getLocationInWindow(offsetInWindow);
                offsetInWindow[0] -= startX;
                offsetInWindow[1] -= startY;
            }
            return true;
        } else if (offsetInWindow != nullptr) {
            // No motion, no dispatch. Keep offsetInWindow up to date.
            offsetInWindow[0] = 0;
            offsetInWindow[1] = 0;
        }
    }
    return false;
}

bool NestedScrollingChildHelper::dispatchNestedPreScroll(int dx, int dy,
        int* consumed, int* offsetInWindow){
    return dispatchNestedPreScroll(dx, dy, consumed, offsetInWindow, View::TYPE_TOUCH);
}

bool NestedScrollingChildHelper::dispatchNestedPreScroll(int dx, int dy,
        int consumed[], int offsetInWindow[], int type) {
    if (isNestedScrollingEnabled()) {
        ViewGroup*parent = getNestedScrollingParentForType(type);
        if (parent == nullptr) {
            return false;
        }

        if (dx != 0 || dy != 0) {
            int startX = 0;
            int startY = 0;
            if (offsetInWindow != nullptr) {
                mView->getLocationInWindow(offsetInWindow);
                startX = offsetInWindow[0];
                startY = offsetInWindow[1];
            }

            if (consumed == nullptr) {
                consumed = mTempNestedScrollConsumed;
            }
            consumed[0] = 0;
            consumed[1] = 0;
            parent->onNestedPreScroll(mView, dx, dy, consumed/*, type*/);

            if (offsetInWindow != nullptr) {
                mView->getLocationInWindow(offsetInWindow);
                offsetInWindow[0] -= startX;
                offsetInWindow[1] -= startY;
            }
            return consumed[0] != 0 || consumed[1] != 0;
        } else if (offsetInWindow != nullptr) {
            offsetInWindow[0] = 0;
            offsetInWindow[1] = 0;
        }
    }
    return false;
}

bool NestedScrollingChildHelper::dispatchNestedFling(float velocityX, float velocityY, bool consumed) {
    if (isNestedScrollingEnabled()) {
        ViewGroup* parent = getNestedScrollingParentForType(View::TYPE_TOUCH);
        if (parent != nullptr) {
            return parent->onNestedFling(mView, velocityX,velocityY, consumed);
        }
    }
    return false;
}

bool NestedScrollingChildHelper::dispatchNestedPreFling(float velocityX, float velocityY) {
    if (isNestedScrollingEnabled()) {
        ViewGroup* parent = getNestedScrollingParentForType(View::TYPE_TOUCH);
        if (parent != nullptr) {
            return parent->onNestedPreFling(mView, velocityX, velocityY);
        }
    }
    return false;
}

void NestedScrollingChildHelper::onDetachedFromWindow() {
    mView->stopNestedScroll();
}

void NestedScrollingChildHelper::onStopNestedScroll( View* child) {
    mView->stopNestedScroll();
}

ViewGroup* NestedScrollingChildHelper::getNestedScrollingParentForType(int type) const{
    switch (type) {
    case View::TYPE_TOUCH:    return mNestedScrollingParentTouch;
    case View::TYPE_NON_TOUCH:return mNestedScrollingParentNonTouch;
    }
    return nullptr;
}

void NestedScrollingChildHelper::setNestedScrollingParentForType(int type, ViewGroup* p) {
    switch (type) {
    case View::TYPE_TOUCH:     mNestedScrollingParentTouch = p;     break;
    case View::TYPE_NON_TOUCH: mNestedScrollingParentNonTouch = p;  break;
    }
}

}/*endof namespace*/
