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
#include <widget/viewanimator.h>
#include <cdlog.h>

namespace cdroid{

DECLARE_WIDGET(ViewAnimator)

ViewAnimator::ViewAnimator(int w,int h):FrameLayout(w,h){
    mInAnimation = nullptr;
    mOutAnimation= nullptr;
    setMeasureAllChildren(true);
}

ViewAnimator::ViewAnimator(Context* context,const AttributeSet& attrs)
  :FrameLayout(context,attrs){
    mInAnimation = nullptr;
    mOutAnimation= nullptr;
}
ViewAnimator::~ViewAnimator(){
    delete mInAnimation;
    delete mOutAnimation;
}

void ViewAnimator::initViewAnimator(Context* context, const AttributeSet& attrs) {
    // For compatibility, default to measure children, but allow XML
    // attribute to override.
    setMeasureAllChildren(attrs.getBoolean("measureAllChildren", true));
    mInAnimation  = nullptr;
    mOutAnimation = nullptr;
}
void ViewAnimator::setDisplayedChild(int whichChild) {
    mWhichChild = whichChild;
    if (whichChild >= getChildCount()) {
        mWhichChild = 0;
    } else if (whichChild < 0) {
        mWhichChild = getChildCount() - 1;
    }
    bool hasFocus = getFocusedChild() != nullptr;
    // This will clear old focus if we had it
    showOnly(mWhichChild);
    if (hasFocus) {
        // Try to retake focus if we had it
        requestFocus(FOCUS_FORWARD);
    }
}

int ViewAnimator::getDisplayedChild()const{
    return mWhichChild;
}

void ViewAnimator::showNext() {
    setDisplayedChild(mWhichChild + 1);
}
void ViewAnimator::showPrevious() {
    setDisplayedChild(mWhichChild - 1);
}

void ViewAnimator::showOnly(int childIndex, bool animate) {
    const int count = getChildCount();
    for (int i = 0; i < count; i++) {
       View* child = getChildAt(i);
       if (i == childIndex) {
           LOGV("set %d Visible",i);
           if (animate && mInAnimation != nullptr) {
               child->startAnimation(mInAnimation->clone());
           }
           child->setVisibility(View::VISIBLE);
           mFirstTime = false;
       } else {
           if (animate && mOutAnimation != nullptr && child->getVisibility() == View::VISIBLE) {
               child->startAnimation(mOutAnimation->clone());
           } else if (child->getAnimation() == mInAnimation){
               child->clearAnimation();
           }
           child->setVisibility(View::GONE);
       }
   }
}

void ViewAnimator::showOnly(int childIndex) {
    bool animate = (!mFirstTime || mAnimateFirstTime);
    showOnly(childIndex, animate);
}

void ViewAnimator::addView(View* child, int index, ViewGroup::LayoutParams* params) {
    FrameLayout::addView(child, index, params);
    if (getChildCount() == 1) {
        child->setVisibility(View::VISIBLE);
    } else {
        child->setVisibility(View::GONE);
    }
    if (index >= 0 && mWhichChild >= index) {
        // Added item above current one, increment the index of the displayed child
        setDisplayedChild(mWhichChild + 1);
    }
}

void ViewAnimator::removeAllViews() {
    FrameLayout::removeAllViews();
    mWhichChild = 0;
    mFirstTime = true;
}

void ViewAnimator::removeView(View* view) {
    int index = indexOfChild(view);
    if (index >= 0) {
        removeViewAt(index);
    }
}

void ViewAnimator::removeViewAt(int index) {
    FrameLayout::removeViewAt(index);
    int childCount = getChildCount();
    if (childCount == 0) {
        mWhichChild = 0;
        mFirstTime = true;
    } else if (mWhichChild >= childCount) {
        // Displayed is above child count, so float down to top of stack
        setDisplayedChild(childCount - 1);
    } else if (mWhichChild == index) {
        // Displayed was removed, so show the new child living in its place
        setDisplayedChild(mWhichChild);
    }
}

void ViewAnimator::removeViewInLayout(View* view) {
    removeView(view);
}

void ViewAnimator::removeViews(int start, int count) {
    FrameLayout::removeViews(start, count);
    if (getChildCount() == 0) {
        mWhichChild = 0;
        mFirstTime = true;
    } else if (mWhichChild >= start && mWhichChild < start + count) {
        // Try showing new displayed child, wrapping if needed
        setDisplayedChild(mWhichChild);
    }
}

void ViewAnimator::removeViewsInLayout(int start, int count) {
    removeViews(start, count);
}

View* ViewAnimator::getCurrentView() const{
    return getChildAt(mWhichChild);
}

Animation* ViewAnimator::getInAnimation()const{
    return mInAnimation;
}

void ViewAnimator::setInAnimation(Animation* inAnimation){
    mInAnimation=inAnimation;
}

Animation* ViewAnimator::getOutAnimation()const{
    return mOutAnimation;
}

void ViewAnimator::setOutAnimation(Animation* outAnimation){
    mOutAnimation = outAnimation;
}

bool ViewAnimator::getAnimateFirstView() const{
    return mAnimateFirstTime;
}

void ViewAnimator::setAnimateFirstView(bool animate) {
    mAnimateFirstTime = animate;
}

int ViewAnimator::getBaseline() {
    return getCurrentView() ? getCurrentView()->getBaseline() : FrameLayout::getBaseline();
}

}//endofnamespace



