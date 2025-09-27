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
#include <core/countdowntimer.h>
#include <core/systemclock.h>

namespace cdroid{

class CountDownTimer::TimerHandler:public Handler {
private:
    CountDownTimer*mCountDownTimer;
public:
    TimerHandler(CountDownTimer*timer):mCountDownTimer(timer){}
    void handleMessage(Message& msg) override{
        if (mCountDownTimer->mCancelled) {
            return;
        }
        const int64_t millisLeft = mCountDownTimer->mStopTimeInFuture - SystemClock::elapsedRealtime();

        if (millisLeft <= 0) {
            mCountDownTimer->onFinish();
        } else {
            int64_t lastTickStart = SystemClock::elapsedRealtime();
            mCountDownTimer->onTick(millisLeft);

            // take into account user's onTick taking time to execute
            int64_t lastTickDuration = SystemClock::elapsedRealtime() - lastTickStart;
            int64_t delay;

            if (millisLeft < mCountDownTimer->mCountdownInterval) {
                // just delay until done
                delay = millisLeft - lastTickDuration;

                // special case: user's onTick took more than interval to
                // complete, trigger onFinish without delay
                if (delay < 0) delay = 0;
            } else {
                delay = mCountDownTimer->mCountdownInterval - lastTickDuration;

                // special case: user's onTick took more than interval to
                // complete, skip to next interval
                while (delay < 0) delay += mCountDownTimer->mCountdownInterval;
            }
            sendMessageDelayed(obtainMessage(MSG), delay);
        }
    }
};

CountDownTimer::CountDownTimer(int64_t millisInFuture, int64_t countDownInterval) {
    mMillisInFuture = millisInFuture;
    mCountdownInterval = countDownInterval;
    mHandler = new TimerHandler(this);
}

CountDownTimer::~CountDownTimer(){
    delete mHandler;
}

void CountDownTimer::setTimerListener(const TimerListener&listener){
    mTimerListener =listener;
}

void CountDownTimer::cancel() {
    mCancelled = true;
    mHandler->removeMessages(MSG);
}

void CountDownTimer::start() {
    mCancelled = false;
    if (mMillisInFuture <= 0) {
        onFinish();
        return;
    }
    mStopTimeInFuture = SystemClock::elapsedRealtime() + mMillisInFuture;
    mHandler->sendMessage(mHandler->obtainMessage(MSG));
}

void CountDownTimer::onTick(int64_t millisUntilFinished){
    if(mTimerListener.onTick){
        mTimerListener.onTick(millisUntilFinished);
    }
}

void CountDownTimer::onFinish(){
    if(mTimerListener.onFinish){
        mTimerListener.onFinish();
    }
}

}/*endof namespace*/;


