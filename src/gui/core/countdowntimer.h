#ifndef __COUNT_DOWN_TIMER_H__
#define __COUNT_DOWN_TIMER_H__
#include <core/handler.h>
namespace cdroid{
class CountDownTimer {
private:
    class TimerHandler;
    static constexpr int MSG = 1;
    /**
     * Millis since epoch when alarm should stop.
     */
    int64_t mMillisInFuture;

    /**
     * The interval in millis that the user receives callbacks
     */
    long mCountdownInterval;
    long mStopTimeInFuture;
    Handler*mHandler;
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
    virtual void onTick(long millisUntilFinished)=0;

    /**
     * Callback fired when the time is up.
     */
    virtual void onFinish()=0;
};
}/*endof namespace*/
#endif/*__COUNT_DOWN_TIMER_H__*/
