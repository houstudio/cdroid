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
#ifndef __TIME_ANIMATOR_H__
#define __TIME_ANIMATOR_H__
#include <animation/valueanimator.h>
namespace cdroid{

class TimeAnimator:public ValueAnimator{
public:
    typedef std::function<void(TimeAnimator&,int64_t totalTime,int64_t detaTime)>TimeListener;
private:
    TimeListener mListener;
    int64_t mPreviousTime;
public:
    TimeAnimator();
    void start()override;
    bool animateBasedOnTime(int64_t currentTime)override;
    void setCurrentPlayTime(int64_t playTime)override;
    void setTimeListener(const TimeListener& listener);
    void animateValue(float fraction)override;
    void initAnimation()override;
};

}//endof namespace
#endif
