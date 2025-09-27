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
#ifndef __CIRCULAR_PROGRESS_LAYOUT_CONTROLLER_H__
#define __CIRCULAR_PROGRESS_LAYOUT_CONTROLLER_H__
#include <core/countdowntimer.h>
#include <widgetEx/wear/circularprogresslayout.h>
namespace cdroid{
class CircularProgressLayoutController {
private:
    CircularProgressLayout* mLayout;
    std::unique_ptr<CountDownTimer>mTimer;

    bool mIsIndeterminate;
    bool mIsTimerRunning;

    CircularProgressLayout::OnTimerFinishedListener mOnTimerFinishedListener;
private:
    void onRunnableProc();
    void onTick(long millisUntilFinished);
    void onFinish();
public:
    CircularProgressLayoutController(CircularProgressLayout* layout);

    CircularProgressLayout::OnTimerFinishedListener getOnTimerFinishedListener()const;
    void setOnTimerFinishedListener(const CircularProgressLayout::OnTimerFinishedListener& listener);

    /** Returns true if the progress is shown as an indeterminate spinner. */
    bool isIndeterminate() const;

    /** Returns true if timer is running. */
    bool isTimerRunning() const;

    /** Sets if the progress should be shown as an indeterminate spinner. */
    void setIndeterminate(bool indeterminate);

    void startTimer(int64_t totalTime, long updateInterval);
    void stopTimer();

    void reset();

};
}/*endof namespace*/
#endif/*__CIRCULAR_PROGRESS_LAYOUT_CONTROLLER_H__*/
