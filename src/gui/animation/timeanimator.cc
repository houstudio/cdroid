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
#include <animation/timeanimator.h>
#include <animation/animationutils.h>
namespace cdroid{

TimeAnimator::TimeAnimator(){
    mPreviousTime = -1;
    mListener =nullptr;
}

void TimeAnimator::start(){
    mPreviousTime = -1;
    ValueAnimator::start();
}

bool TimeAnimator::animateBasedOnTime(int64_t currentTime){
    if(mListener != nullptr){
        const int64_t totalTime = currentTime - mStartTime;
        const int64_t detaTime  = (mPreviousTime<0)?0:(currentTime -mPreviousTime);
        mPreviousTime  = currentTime;
        mListener(*this,totalTime,detaTime);
    }
    return true;
}

void TimeAnimator::setCurrentPlayTime(int64_t playTime){
    const int64_t currentTime=AnimationUtils::currentAnimationTimeMillis();
    mStartTime = std::max(mStartTime,currentTime - playTime);
    animateBasedOnTime(currentTime);
}

void TimeAnimator::setTimeListener(const TimeAnimator::TimeListener& listener){
    mListener = listener;
}

void TimeAnimator::animateValue(float fraction){
    //NOTHING
}

void TimeAnimator::initAnimation(){
    //NOTHING
}

}//endof namespace
