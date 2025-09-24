#ifndef __CIRCULAR_PROGRESS_LAYOUT_CONTROLLER_H__
#define __CIRCULAR_PROGRESS_LAYOUT_CONTROLLER_H__
#include <widgetEx/wear/circularprogresslayout.h>
namespace cdroid{
class CircularProgressLayoutController {

    CircularProgressLayout* mLayout;
    //CountDownTimer mTimer;
    Runnable mRunnable;
    long mTotalTime;
    long mInterval;
    long mMillisUntilFinished;

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

    void startTimer(long totalTime, long updateInterval);
    void stopTimer();

    void reset();

};
}/*endof namespace*/
#endif/*__CIRCULAR_PROGRESS_LAYOUT_CONTROLLER_H__*/
