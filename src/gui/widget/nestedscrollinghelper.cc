#include <widget/nestedscrollinghelper.h>

#define TYPE_TOUCH 0
#define TYPE_NON_TOUCH 1

namespace cdroid{

NestedScrollingParentHelper::NestedScrollingParentHelper(ViewGroup* viewGroup) {
    mNestedScrollAxes=0;
    mViewGroup = viewGroup;
}
 
void NestedScrollingParentHelper::onNestedScrollAccepted(View* child,View* target,
       int axes) {
    onNestedScrollAccepted(child, target, axes, TYPE_TOUCH);
}
 
void NestedScrollingParentHelper::onNestedScrollAccepted(View* child,View* target,
         int axes,int type) {
    mNestedScrollAxes = axes;
}
 
int NestedScrollingParentHelper::getNestedScrollAxes()const{
    return mNestedScrollAxes;
}
 
void NestedScrollingParentHelper::onStopNestedScroll(View* target) {
    onStopNestedScroll(target, TYPE_TOUCH);
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

bool NestedScrollingChildHelper::isNestedScrollingEnabled() {
    return mIsNestedScrollingEnabled;
}

bool NestedScrollingChildHelper::hasNestedScrollingParent() {
    return hasNestedScrollingParent(TYPE_TOUCH);
}

bool NestedScrollingChildHelper::hasNestedScrollingParent( int type) {
    return getNestedScrollingParentForType(type) != nullptr;
}

bool NestedScrollingChildHelper::startNestedScroll( int axes) {
    return startNestedScroll(axes, TYPE_TOUCH);
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
    stopNestedScroll(TYPE_TOUCH);
}

void NestedScrollingChildHelper::stopNestedScroll( int type) {
    ViewGroup* parent = getNestedScrollingParentForType(type);
    if (parent != nullptr) {
        parent->onStopNestedScroll(mView/*, type*/);
        setNestedScrollingParentForType(type, nullptr);
    }
}

bool NestedScrollingChildHelper::dispatchNestedScroll(int dxConsumed, int dyConsumed,
        int dxUnconsumed, int dyUnconsumed, int offsetInWindow[]) {
    return dispatchNestedScroll(dxConsumed, dyConsumed, dxUnconsumed, dyUnconsumed,
            offsetInWindow, TYPE_TOUCH);
}

bool NestedScrollingChildHelper::dispatchNestedScroll(int dxConsumed, int dyConsumed,
        int dxUnconsumed, int dyUnconsumed,int offsetInWindow[], int type) {
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

	    parent->onNestedScroll(mView, dxConsumed, dyConsumed, dxUnconsumed, dyUnconsumed/*, type*/);

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
	int consumed[], int offsetInWindow[]) {
    return dispatchNestedPreScroll(dx, dy, consumed, offsetInWindow, TYPE_TOUCH);
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
        ViewGroup* parent = getNestedScrollingParentForType(TYPE_TOUCH);
        if (parent != nullptr) {
            return parent->onNestedFling(mView, velocityX,velocityY, consumed);
        }
    }
    return false;
}

bool NestedScrollingChildHelper::dispatchNestedPreFling(float velocityX, float velocityY) {
    if (isNestedScrollingEnabled()) {
        ViewGroup* parent = getNestedScrollingParentForType(TYPE_TOUCH);
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

ViewGroup* NestedScrollingChildHelper::getNestedScrollingParentForType(int type) {
    switch (type) {
    case TYPE_TOUCH:
            return mNestedScrollingParentTouch;
    case TYPE_NON_TOUCH:
            return mNestedScrollingParentNonTouch;
    }
    return nullptr;
}

void NestedScrollingChildHelper::setNestedScrollingParentForType(int type, ViewGroup* p) {
    switch (type) {
    case TYPE_TOUCH:
            mNestedScrollingParentTouch = p;
            break;
    case TYPE_NON_TOUCH:
            mNestedScrollingParentNonTouch = p;
            break;
    }
}
}/*endof namespace*/