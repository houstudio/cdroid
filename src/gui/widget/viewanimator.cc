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
#include <core/context.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
#include <animation/animationutils.h>
#include <cdlog.h>

namespace cdroid{
using namespace cdroid::internal;

DECLARE_WIDGET(ViewAnimator)

ViewAnimator::ViewAnimator(Context*ctx):ViewAnimator(ctx,nullptr){}

ViewAnimator::ViewAnimator(Context* context,const AttributeSet* attrs):ViewAnimator(context,attrs,0){}

ViewAnimator::ViewAnimator(Context* context,const AttributeSet* pAttrs,int defStyleAttr)
  :FrameLayout(context,pAttrs, defStyleAttr){
    mInAnimation = nullptr;
    mOutAnimation= nullptr;
    // AOSP ViewAnimator ctor: in/out animations and animateFirstView from XML.
    // The ViewAnimator styleable is not generated in this tree; AdapterViewAnimator
    // declares the same attrs with identical ids (in=0, out=1, animateFirstView=3).
    auto ta = context->obtainStyledAttributes(pAttrs, R::styleable::AdapterViewAnimator, defStyleAttr);
    if (ta) {
        const int resource = ta->getResourceId(R::styleable::AdapterViewAnimator_inAnimation, 0);
        if (resource > 0) setInAnimation(context, resource);
        const int resource2 = ta->getResourceId(R::styleable::AdapterViewAnimator_outAnimation, 0);
        if (resource2 > 0) setOutAnimation(context, resource2);
        setAnimateFirstView(ta->getBoolean(R::styleable::AdapterViewAnimator_animateFirstView, true));
    }
    initViewAnimator(context, pAttrs);
}
ViewAnimator::~ViewAnimator(){
    delete mInAnimation;
    delete mOutAnimation;
}

void ViewAnimator::initViewAnimator(Context* context, const AttributeSet* attrs) {
    if (attrs == nullptr) {
        // For compatibility, always measure children when undefined.
        setMeasureAllChildren(true);
        return;
    }
    // For compatibility, default to measure children, but allow XML
    // attribute to override.
    auto ta = context->obtainStyledAttributes(attrs, R::styleable::FrameLayout, 0);
    if (ta) {
        setMeasureAllChildren(ta->getBoolean(R::styleable::FrameLayout_measureAllChildren, true));
    }
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
    if (mInAnimation != inAnimation){
        delete mInAnimation;
    }
    mInAnimation = inAnimation;
}

Animation* ViewAnimator::getOutAnimation()const{
    return mOutAnimation;
}

void ViewAnimator::setOutAnimation(Animation* outAnimation){
    if (mOutAnimation != outAnimation){
        delete mOutAnimation;
    }
    mOutAnimation = outAnimation;
}

void ViewAnimator::setInAnimation(Context* context, int resourceID) {
    setInAnimation(AnimationUtils::loadAnimation(context, resourceID));
}

void ViewAnimator::setOutAnimation(Context* context, int resourceID) {
    setOutAnimation(AnimationUtils::loadAnimation(context, resourceID));
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

std::string ViewAnimator::getAccessibilityClassName()const{
    return "ViewAnimator";
}

}//endofnamespace



