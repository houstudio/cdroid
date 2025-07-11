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

#ifndef __CHRONOMETER_H__
#define __CHRONOMETER_H__
#include <widget/textview.h>
namespace cdroid{
class Chronometer:public TextView{
public:
    DECLARE_UIEVENT(void,OnChronometerTickListener,Chronometer&);
    DECLARE_UIEVENT(const std::string,Formatter,int64_t);
private:
    int64_t mBase;
    int64_t mNow; // the currently displayed time
    bool mVisible;
    bool mStarted;
    bool mRunning;
    bool mLogged;
    bool mColonBlinking;
    Runnable mTickRunnable;
    std::string mFormat;
    Formatter mFormatter;
    OnChronometerTickListener mOnChronometerTickListener;
    bool mCountDown;
private:
    void init();
    void tickRunner();
    void updateText(int64_t now);
    void updateRunning();
    static std::string formatDuration(int64_t ms);
protected:
    void dispatchChronometerTick();
public:
    Chronometer(int w,int h);
    Chronometer(Context*ctx,const AttributeSet&);
    void setCountDown(bool countDown);
    bool isCountDown()const;
    bool isTheFinalCountDown()const;
    void setBase(int64_t base);
    int64_t getBase()const;
    void setFormat(const std::string& format);
    std::string getFormat()const;
    void setOnChronometerTickListener(const OnChronometerTickListener& listener);
    OnChronometerTickListener getOnChronometerTickListener()const;
    void start();
    void stop();
    void setStarted(bool started);
    std::string getContentDescription()const override;
    std::string getAccessibilityClassName()const override;
};
}//namespace


#endif
