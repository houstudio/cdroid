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
#include <widgetEx/wear/swipedismissframelayout.h>
namespace cdroid{

DECLARE_WIDGET(SwipeDismissFrameLayout);

SwipeDismissFrameLayout::SwipeDismissFrameLayout(Context* context,const AttributeSet& attrs)
    :DismissibleFrameLayout(context, attrs){
}

void SwipeDismissFrameLayout::addCallback(const SwipeDismissFrameLayout::Callback& callback) {
    /*if (callback == null) {
        throw new NullPointerException("addCallback called with null callback");
    }*/
    mCallbacksCompat.push_back(callback);
}

void SwipeDismissFrameLayout::removeCallback(const SwipeDismissFrameLayout::Callback& callback) {
    /*if (callback == null) {
        throw new NullPointerException("removeCallback called with null callback");
    }
    if (!mCallbacksCompat.remove(callback)) {
        throw new IllegalStateException("removeCallback called with nonexistent callback");
    }*/
    auto it =std::find(mCallbacksCompat.begin(),mCallbacksCompat.end(),callback);
    if(it!=mCallbacksCompat.end()){
        mCallbacksCompat.erase(it);
    }
}

void SwipeDismissFrameLayout::setSwipeable(bool swipeable) {
    DismissibleFrameLayout::setSwipeDismissible(swipeable);
}

bool SwipeDismissFrameLayout::isSwipeable() const{
    return DismissibleFrameLayout::isDismissableBySwipe();
}

void SwipeDismissFrameLayout::setDismissMinDragWidthRatio(float ratio) {
    if (isSwipeable()) {
        getSwipeDismissController()->setDismissMinDragWidthRatio(ratio);
    }
}

float SwipeDismissFrameLayout::getDismissMinDragWidthRatio() const{
    if (isSwipeable()) {
        return getSwipeDismissController()->getDismissMinDragWidthRatio();
    }
    return DEFAULT_DISMISS_DRAG_WIDTH_RATIO;
}

void SwipeDismissFrameLayout::performDismissFinishedCallbacks() {
    DismissibleFrameLayout::performDismissFinishedCallbacks();
    for (int i = mCallbacksCompat.size() - 1; i >= 0; i--) {
        mCallbacksCompat.at(i).onDismissed(*this);
    }
}

void SwipeDismissFrameLayout::performDismissStartedCallbacks() {
    DismissibleFrameLayout::performDismissStartedCallbacks();
    for (int i = mCallbacksCompat.size() - 1; i >= 0; i--) {
        mCallbacksCompat.at(i).onSwipeStarted(*this);
    }
}

void SwipeDismissFrameLayout::performDismissCanceledCallbacks() {
    DismissibleFrameLayout::performDismissCanceledCallbacks();
    for (int i = mCallbacksCompat.size() - 1; i >= 0; i--) {
        mCallbacksCompat.at(i).onSwipeCanceled(*this);
    }
}
}
