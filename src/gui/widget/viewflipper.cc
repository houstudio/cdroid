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
#include <widget/viewflipper.h>
#include <core/context.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
#include <porting/cdlog.h>

namespace cdroid{
using namespace cdroid::internal;

DECLARE_WIDGET(ViewFlipper)

ViewFlipper::ViewFlipper(Context*ctx)
    :ViewFlipper(ctx,nullptr){}

ViewFlipper::ViewFlipper(Context* context,const AttributeSet* attrs):ViewFlipper(context,attrs,0){}

ViewFlipper::ViewFlipper(Context* context,const AttributeSet* pAttrs,int defStyleAttr)
  :ViewAnimator(context,pAttrs, defStyleAttr){
    mFlipRunnable = [this](){doFlip();};
    // AOSP reads the ViewFlipper styleable; this tree declares the same two
    // attrs under AdapterViewFlipper with identical ids and order.
    auto ta = context->obtainStyledAttributes(pAttrs, R::styleable::AdapterViewFlipper, defStyleAttr);
    if (ta) {
        mFlipInterval = ta->getInt(R::styleable::AdapterViewFlipper_flipInterval, DEFAULT_INTERVAL);
        mAutoStart = ta->getBoolean(R::styleable::AdapterViewFlipper_autoStart, false);
    }
}

void ViewFlipper::onAttachedToWindow(){
    ViewAnimator::onAttachedToWindow();
    if (mAutoStart) {
        // Automatically start when requested
        startFlipping();
    }
}

void ViewFlipper::onDetachedFromWindow(){
    ViewAnimator::onDetachedFromWindow();
    mVisible = false;
    updateRunning(true);
}

void ViewFlipper::onWindowVisibilityChanged(int visibility){
    ViewAnimator::onWindowVisibilityChanged(visibility);
    mVisible = (visibility == View::VISIBLE);
    updateRunning(false);
}

void ViewFlipper::doFlip(){
    if (mRunning) {
        showNext();
        postDelayed(mFlipRunnable, mFlipInterval);
    }
}

void ViewFlipper::setFlipInterval(int milliseconds) {
    mFlipInterval = milliseconds;
}

int ViewFlipper::getFlipInterval()const{
    return mFlipInterval;
}

void ViewFlipper::startFlipping() {
    mStarted = true;
    updateRunning(true);
}

void ViewFlipper::stopFlipping() {
    mStarted = false;
    updateRunning(true);
}

bool ViewFlipper::isFlipping() const{
    return mStarted;
}

void ViewFlipper::setAutoStart(bool autoStart) {
    mAutoStart = autoStart;
}

bool ViewFlipper::isAutoStart()const{
    return mAutoStart;
}

void ViewFlipper::updateRunning(bool flipNow){
    bool running = mVisible && mStarted && mUserPresent;
    LOGV("running=%d mVisible=%d mStarted=%d mUserPresent=%d mRunning=%d",
         running,mVisible,mStarted,mUserPresent,mRunning);
    if (running != mRunning) {
        if (running) {
            showOnly(mWhichChild, flipNow);
            postDelayed(mFlipRunnable, mFlipInterval);
        } else {
            removeCallbacks(mFlipRunnable);
        }
        mRunning = running;
    }
}

}
