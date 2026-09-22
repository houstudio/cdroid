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
*/
#include <widgetEx/wear/dismissibleframelayout.h>
#include <widgetEx/wear/backbuttondismisscontroller.h>
namespace cdroid{

DECLARE_WIDGET2(DismissibleFrameLayout, "androidx.wear.widget.DismissibleFrameLayout");

DismissibleFrameLayout::DismissibleFrameLayout(Context* context,const AttributeSet* attrs):DismissibleFrameLayout(context,attrs,0){}

DismissibleFrameLayout::DismissibleFrameLayout(Context* context,const AttributeSet* pAttrs,int defStyleAttr)
    :FrameLayout(context, pAttrs/*, defStyle, defStyleRes*/){

    mContext = context;
    setSwipeDismissible(true);//WearableNavigationHelper::isSwipeToDismissEnabled(context));
    setBackButtonDismissible(false);
    mDismissListener.onDismissStarted=[this](){
        performDismissStartedCallbacks();
    };
    mDismissListener.onDismissCanceled=[this](){
        performDismissCanceledCallbacks();
    };
    mDismissListener.onDismissed =[this](){
        performDismissFinishedCallbacks();
    };
}

void DismissibleFrameLayout::registerCallback(const Callback& callback) {
    mCallbacks.push_back(callback);
}

void DismissibleFrameLayout::unregisterCallback(const Callback& callback) {
    auto it = std::find(mCallbacks.begin(),mCallbacks.end(),callback);
    if (it==mCallbacks.end()) {
        throw std::runtime_error("removeCallback called with nonexistent callback");
    }
    mCallbacks.erase(it);
}

DismissibleFrameLayout::~DismissibleFrameLayout() {
    // Java GC collects the controllers with the view; CDROID owns them.
    delete mSwipeDismissController;
    delete mBackButtonDismissController;
}

void DismissibleFrameLayout::setSwipeDismissible(bool swipeDismissible) {
    if (swipeDismissible) {
        if (mSwipeDismissController == nullptr) {
            mSwipeDismissController = new SwipeDismissController(mContext, this);
            mSwipeDismissController->setOnDismissListener(mDismissListener);
        }
    } else if (mSwipeDismissController != nullptr) {
        mSwipeDismissController->setOnDismissListener({});//nullptr);
        delete mSwipeDismissController;   // Java GC; CDROID owns it
        mSwipeDismissController = nullptr;
    }
}

/** Returns true if the frame layout can be dismissed by swipe gestures. */
bool DismissibleFrameLayout::isDismissableBySwipe() const{
    return mSwipeDismissController != nullptr;
}

void DismissibleFrameLayout::setBackButtonDismissible(bool backButtonDismissible) {
    if (backButtonDismissible) {
        if (mBackButtonDismissController == nullptr) {
            mBackButtonDismissController = new BackButtonDismissController(mContext, this);
            mBackButtonDismissController->setOnDismissListener(mDismissListener);
        }
    } else if (mBackButtonDismissController != nullptr) {
        mBackButtonDismissController->disable(this);
        delete mBackButtonDismissController;   // Java GC; CDROID owns it
        mBackButtonDismissController = nullptr;
    }
}

/** Returns true if the frame layout would be dismissed with back button click */
bool DismissibleFrameLayout::isDismissableByBackButton()  const{
    return mBackButtonDismissController != nullptr;
}

SwipeDismissController* DismissibleFrameLayout::getSwipeDismissController() const{
    return mSwipeDismissController;
}

void DismissibleFrameLayout::performDismissFinishedCallbacks() {
    for (int i = mCallbacks.size() - 1; i >= 0; i--) {
        auto& cb = mCallbacks.at(i); if (cb.onDismissFinished) cb.onDismissFinished(*this);
    }
}

void DismissibleFrameLayout::performDismissStartedCallbacks() {
    for (int i = mCallbacks.size() - 1; i >= 0; i--) {
        auto& cb = mCallbacks.at(i); if (cb.onDismissStarted) cb.onDismissStarted(*this);
    }
}

void DismissibleFrameLayout::performDismissCanceledCallbacks() {
    for (int i = mCallbacks.size() - 1; i >= 0; i--) {
        auto& cb = mCallbacks.at(i); if (cb.onDismissCanceled) cb.onDismissCanceled(*this);
    }
}

void DismissibleFrameLayout::requestDisallowInterceptTouchEvent(bool disallowIntercept) {
    if (mSwipeDismissController != nullptr) {
        mSwipeDismissController->requestDisallowInterceptTouchEvent(disallowIntercept);
    } else {
        FrameLayout::requestDisallowInterceptTouchEvent(disallowIntercept);
    }
}

bool DismissibleFrameLayout::onInterceptTouchEvent(MotionEvent& ev) {
    if (mSwipeDismissController != nullptr) {
        return mSwipeDismissController->onInterceptTouchEvent(ev);
    }
    return FrameLayout::onInterceptTouchEvent(ev);
}

bool DismissibleFrameLayout::canScrollHorizontally(int direction) {
    if (mSwipeDismissController != nullptr) {
        return mSwipeDismissController->canScrollHorizontally(direction);
    }
    return FrameLayout::canScrollHorizontally(direction);
}

bool DismissibleFrameLayout::onTouchEvent(MotionEvent& ev) {
    if ((mSwipeDismissController != nullptr)
            && mSwipeDismissController->onTouchEvent(ev)) {
        return true;
    }
    return FrameLayout::onTouchEvent(ev);
}
}

