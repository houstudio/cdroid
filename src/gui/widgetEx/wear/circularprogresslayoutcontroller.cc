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
#include <widgetEx/wear/circularprogressdrawable.h>
#include <widgetEx/wear/circularprogresslayoutcontroller.h>
namespace cdroid{

CircularProgressLayoutController::CircularProgressLayoutController(CircularProgressLayout* layout) {
    mLayout = layout;
    mIsTimerRunning = false;
    mIsIndeterminate=false;
    mTotalTime = 0;
    mInterval = 0;
}

CircularProgressLayout::OnTimerFinishedListener CircularProgressLayoutController::getOnTimerFinishedListener() const{
    return mOnTimerFinishedListener;
}

void CircularProgressLayoutController::setOnTimerFinishedListener(const CircularProgressLayout::OnTimerFinishedListener& listener) {
    mOnTimerFinishedListener = listener;
}

bool CircularProgressLayoutController::isIndeterminate() const{
    return mIsIndeterminate;
}

bool CircularProgressLayoutController::isTimerRunning() const{
    return mIsTimerRunning;
}

/** Sets if the progress should be shown as an indeterminate spinner. */
void CircularProgressLayoutController::setIndeterminate(bool indeterminate) {
    if (mIsIndeterminate == indeterminate) {
        return;
    }
    mIsIndeterminate = indeterminate;
    if (mIsIndeterminate) {
        if (mIsTimerRunning) {
            stopTimer();
        }
        mLayout->getProgressDrawable()->start();
    } else {
        mLayout->getProgressDrawable()->stop();
    }
}

void CircularProgressLayoutController::startTimer(long totalTime, long updateInterval) {
    reset();
    mIsTimerRunning = true;
    mTotalTime= totalTime;
    mInterval = updateInterval;
    mMillisUntilFinished = mTotalTime;
    //mTimer = new CircularProgressTimer(totalTime, updateInterval);
    //mTimer.start();
    mRunnable = std::bind(&CircularProgressLayoutController::onRunnableProc,this);
    mLayout->postDelayed(mRunnable,mInterval);
}

void CircularProgressLayoutController::stopTimer() {
    if (mIsTimerRunning) {
        //mTimer.cancel();
        mLayout->removeCallbacks(mRunnable);
        mIsTimerRunning = false;
        mLayout->getProgressDrawable()->setStartEndTrim(0.0f, 0.0f); // Reset the progress
    }
}

void CircularProgressLayoutController::reset() {
    setIndeterminate(false); // If showing indeterminate progress, stop it
    stopTimer(); // Stop the previous timer if there is one
    mLayout->getProgressDrawable()->setStartEndTrim(0.0f, 0.0f); // Reset the progress
}

void CircularProgressLayoutController::onRunnableProc(){
    mMillisUntilFinished -= mInterval;
    onTick(mMillisUntilFinished);
    if(mMillisUntilFinished<mInterval){
        onFinish();
    }else{
        mLayout->postDelayed(mRunnable,mInterval);
    }
}

void CircularProgressLayoutController::onTick(long millisUntilFinished) {
    mLayout->getProgressDrawable()
            ->setStartEndTrim(0.0f, 1.0f - (float) millisUntilFinished / (float) mTotalTime);
    mLayout->invalidate();
}

void CircularProgressLayoutController::onFinish() {
    mLayout->getProgressDrawable()->setStartEndTrim(0.0f, 1.0f);
    if (mOnTimerFinishedListener != nullptr) {
        mOnTimerFinishedListener/*->onTimerFinished*/(*mLayout);
    }
    mIsTimerRunning = false;
}
}/*endof namespace*/
