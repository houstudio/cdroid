#include <widgetEx/wear/circularprogressdrawable.h>
#include <widgetEx/wear/circularprogresslayoutcontroller.h>
namespace cdroid{

CircularProgressLayoutController::CircularProgressLayoutController(CircularProgressLayout* layout) {
    mLayout = layout;
    mTotalTime = 0;
    mInterval = 0;
}

/**
 * Returns the registered {@link CircularProgressLayout.OnTimerFinishedListener}.
 */
CircularProgressLayout::OnTimerFinishedListener CircularProgressLayoutController::getOnTimerFinishedListener() const{
    return mOnTimerFinishedListener;
}

/**
 * Sets the {@link CircularProgressLayout.OnTimerFinishedListener} to be notified when timer
 * countdown is finished.
 */
void CircularProgressLayoutController::setOnTimerFinishedListener(const CircularProgressLayout::OnTimerFinishedListener& listener) {
    mOnTimerFinishedListener = listener;
}

/** Returns true if the progress is shown as an indeterminate spinner. */
bool CircularProgressLayoutController::isIndeterminate() const{
    return mIsIndeterminate;
}

/** Returns true if timer is running. */
bool CircularProgressLayoutController::isTimerRunning() const{
    return mIsTimerRunning;
}

/** Sets if the progress should be shown as an indeterminate spinner. */
void CircularProgressLayoutController::setIndeterminate(bool indeterminate) {
    if (mIsIndeterminate == indeterminate) {
        return;
    }
    mIsIndeterminate = indeterminate;
    if (mIsIndeterminate) {
        if (mIsTimerRunning) {
            stopTimer();
        }
        mLayout->getProgressDrawable()->start();
    } else {
        mLayout->getProgressDrawable()->stop();
    }
}

void CircularProgressLayoutController::startTimer(long totalTime, long updateInterval) {
    reset();
    mIsTimerRunning = true;
    mTotalTime= totalTime;
    mInterval = updateInterval;
    mMillisUntilFinished = mTotalTime;
    //mTimer = new CircularProgressTimer(totalTime, updateInterval);
    //mTimer.start();
    mRunnable = std::bind(&CircularProgressLayoutController::onRunnableProc,this);
    mLayout->postDelayed(mRunnable,mInterval);
}

void CircularProgressLayoutController::stopTimer() {
    if (mIsTimerRunning) {
        //mTimer.cancel();
        mLayout->removeCallbacks(mRunnable);
        mIsTimerRunning = false;
        mLayout->getProgressDrawable()->setStartEndTrim(0.0f, 0.0f); // Reset the progress
    }
}

void CircularProgressLayoutController::reset() {
    setIndeterminate(false); // If showing indeterminate progress, stop it
    stopTimer(); // Stop the previous timer if there is one
    mLayout->getProgressDrawable()->setStartEndTrim(0.0f, 0.0f); // Reset the progress
}

void CircularProgressLayoutController::onRunnableProc(){
    mMillisUntilFinished -= mInterval;
    onTick(mMillisUntilFinished);
    if(mMillisUntilFinished<mInterval){
        onFinish();
    }else{
        mLayout->postDelayed(mRunnable,mInterval);
    }
}

void CircularProgressLayoutController::onTick(long millisUntilFinished) {
    mLayout->getProgressDrawable()
            ->setStartEndTrim(0.0f, 1.0f - (float) millisUntilFinished / (float) mTotalTime);
    mLayout->invalidate();
}

void CircularProgressLayoutController::onFinish() {
    mLayout->getProgressDrawable()->setStartEndTrim(0.0f, 1.0f);
    if (mOnTimerFinishedListener != nullptr) {
        mOnTimerFinishedListener/*->onTimerFinished*/(*mLayout);
    }
    mIsTimerRunning = false;
}
}/*endof namespace*/
