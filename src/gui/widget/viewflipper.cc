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
#include <porting/cdlog.h>

namespace cdroid{

DECLARE_WIDGET(ViewFlipper)

ViewFlipper::ViewFlipper(int w,int h):ViewAnimator(w,h){
    mFlipRunnable=std::bind(&ViewFlipper::doFlip,this);
    mVisible =true;
}

ViewFlipper::ViewFlipper(Context* context,const AttributeSet& attrs)
  :ViewAnimator(context,attrs){
    mFlipRunnable=std::bind(&ViewFlipper::doFlip,this);
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
