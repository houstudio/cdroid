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
#ifndef __VIEWFLIPPER_H__
#define __VIEWFLIPPER_H__
#include <widget/viewanimator.h>

namespace cdroid{

class ViewFlipper:public ViewAnimator{
private:
    static constexpr int DEFAULT_INTERVAL = 3000;
    int mFlipInterval = DEFAULT_INTERVAL;
    bool mAutoStart = false;

    bool mRunning = false;
    bool mStarted = false;
    bool mVisible = false;
    bool mUserPresent = true;
    Runnable mFlipRunnable;
private:
    void updateRunning(bool flipNow);
    void doFlip();
public:
    ViewFlipper(int w,int h);
    ViewFlipper(Context* context,const AttributeSet& attrs);
    void setFlipInterval(int milliseconds);
    int  getFlipInterval()const;
    void startFlipping();
    void stopFlipping();
    bool isFlipping()const;
    void setAutoStart(bool autoStart);
    bool isAutoStart()const;
};
}//endof namespace

#endif
