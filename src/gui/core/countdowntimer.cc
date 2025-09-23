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

CountDownTimer::CountDownTimer(int64_t millisInFuture, long countDownInterval) {
    mMillisInFuture = millisInFuture;
    mCountdownInterval = countDownInterval;
    mHandler = new TimerHandler(this);
}

CountDownTimer::~CountDownTimer(){
    delete mHandler;
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

}/*endof namespace*/;


