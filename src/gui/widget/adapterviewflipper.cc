/*********************************************************************************
+ * Copyright (C) [2019] [houzh@msn.com]
+ *
+ * This library is free software; you can redistribute it and/or
+ * modify it under the terms of the GNU Lesser General Public
+ * License as published by the Free Software Foundation; either
+ * version 2.1 of the License, or (at your option) any later version.
+ *
+ * This library is distributed in the hope that it will be useful,
+ * but WITHOUT ANY WARRANTY; without even the implied warranty of
+ * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
+ * Lesser General Public License for more details.
+ *
+ * You should have received a copy of the GNU Lesser General Public
+ * License along with this library; if not, write to the Free Software
+ * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
+ *********************************************************************************/
#include <widget/adapterviewflipper.h>
namespace cdroid{

DECLARE_WIDGET(AdapterViewFlipper);

AdapterViewFlipper::AdapterViewFlipper(Context* context,const AttributeSet& attrs)
    :AdapterViewAnimator(context, attrs){

    mFlipInterval = attrs.getInt("flipInterval", DEFAULT_INTERVAL);
    mAutoStart = attrs.getBoolean("autoStart", false);

    // A view flipper should cycle through the views
    mLoopViews = true;
    mFlipRunnable = [this](){
        if (mRunning) showNext();
    };
}

void AdapterViewFlipper::onAttachedToWindow() {
    AdapterViewAnimator::onAttachedToWindow();

    if (mAutoStart) {
        // Automatically start when requested
        startFlipping();
    }
}

void AdapterViewFlipper::onDetachedFromWindow() {
    AdapterViewAnimator::onDetachedFromWindow();
    mVisible = false;
    updateRunning(true);
}

void AdapterViewFlipper::onWindowVisibilityChanged(int visibility) {
    AdapterViewAnimator::onWindowVisibilityChanged(visibility);
    mVisible = (visibility == VISIBLE);
    updateRunning(false);
}

void AdapterViewFlipper::setAdapter(Adapter* adapter) {
    AdapterViewAnimator::setAdapter(adapter);
    updateRunning(true);
}

int AdapterViewFlipper::getFlipInterval() const{
    return mFlipInterval;
}

void AdapterViewFlipper::setFlipInterval(int flipInterval) {
    mFlipInterval = flipInterval;
}

/**
 * Start a timer to cycle through child views
 */
void AdapterViewFlipper::startFlipping() {
    mStarted = true;
    updateRunning(true);
}

void AdapterViewFlipper::stopFlipping() {
    mStarted = false;
    updateRunning(true);
}

void AdapterViewFlipper::showNext() {
   // if the flipper is currently flipping automatically, and showNext() is called
   // we should we should make sure to reset the timer
   if (mRunning) {
       removeCallbacks(mFlipRunnable);
       postDelayed(mFlipRunnable, mFlipInterval);
   }
   AdapterViewAnimator::showNext();
}

void AdapterViewFlipper::showPrevious() {
   // if the flipper is currently flipping automatically, and showPrevious() is called
   // we should we should make sure to reset the timer
   if (mRunning) {
       removeCallbacks(mFlipRunnable);
       postDelayed(mFlipRunnable, mFlipInterval);
   }
   AdapterViewAnimator::showPrevious();
}

void AdapterViewFlipper::updateRunning(bool flipNow) {
    bool running = !mAdvancedByHost && mVisible && mStarted && mAdapter != nullptr;
    if (running != mRunning) {
        if (running) {
            showOnly(mWhichChild, flipNow);
            postDelayed(mFlipRunnable, mFlipInterval);
        } else {
            removeCallbacks(mFlipRunnable);
        }
        mRunning = running;
    }
    LOGD("updateRunning() mVisible=%d, mStarted=%d mRunning=%d",mVisible,mStarted,mRunning);
}

bool AdapterViewFlipper::isFlipping() const{
    return mStarted;
}

/**
 * Set if this view automatically calls {@link #startFlipping()} when it
 * becomes attached to a window.
 */
void AdapterViewFlipper::setAutoStart(bool autoStart) {
    mAutoStart = autoStart;
}

/**
 * Returns true if this view automatically calls {@link #startFlipping()}
 * when it becomes attached to a window.
 */
bool AdapterViewFlipper::isAutoStart() const{
    return mAutoStart;
}

void AdapterViewFlipper::fyiWillBeAdvancedByHostKThx() {
    mAdvancedByHost = true;
    updateRunning(false);
}

std::string AdapterViewFlipper::getAccessibilityClassName() const{
    return "AdapterViewFlipper";
}
}
