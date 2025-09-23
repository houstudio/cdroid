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
#ifndef __COUNT_DOWN_TIMER_H__
#define __COUNT_DOWN_TIMER_H__
#include <core/handler.h>
namespace cdroid{
class CountDownTimer {
public:
    struct TimerListener{
        std::function<void(int64_t)>onTick;
        std::function<void()>onFinish;
    };
private:
    class TimerHandler;
    static constexpr int MSG = 1;
    /*Millis since epoch when alarm should stop.*/
    int64_t mMillisInFuture;
    int64_t mStopTimeInFuture;
    /*The interval in millis that the user receives callbacks*/
    long mCountdownInterval;
    Handler*mHandler;
    TimerListener mTimerListener;
    bool mCancelled = false;
public:
    /**
     * @param millisInFuture The number of millis in the future from the call
     *   to {@link #start()} until the countdown is done and {@link #onFinish()}
     *   is called.
     * @param countDownInterval The interval along the way to receive
     *   {@link #onTick(long)} callbacks.
     */
    CountDownTimer(int64_t millisInFuture, long countDownInterval);
    virtual ~CountDownTimer();
    void setTimerListener(const TimerListener&);
    /**
     * Cancel the countdown.
     */
    void cancel();

    /**
     * Start the countdown.
     */
    void start();


    /**
     * Callback fired on regular interval.
     * @param millisUntilFinished The amount of time until finished.
     */
    virtual void onTick(int64_t millisUntilFinished);

    /**
     * Callback fired when the time is up.
     */
    virtual void onFinish();
};
}/*endof namespace*/
#endif/*__COUNT_DOWN_TIMER_H__*/
