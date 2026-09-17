#ifndef __DESKCLOCK_STOPWATCHTEXTCONTROLLER_H__
#define __DESKCLOCK_STOPWATCHTEXTCONTROLLER_H__
/*********************************************************************************
 * Port of com.android.deskclock.StopwatchTextController — formats accumulated
 * time into the main time + hundredths TextView pair, skipping no-op updates.
 *********************************************************************************/
#include <cstdint>   // int64_t / INT64_MIN (self-sufficient header)
#include <widget/textview.h>

namespace cdroid {
namespace deskclock {

class StopwatchTextController {
private:
    TextView& mMainTextView;
    TextView& mHundredthsTextView;

    int64_t mLastTime = INT64_MIN;

public:
    StopwatchTextController(TextView& mainTextView, TextView& hundredthsTextView)
        : mMainTextView(mainTextView), mHundredthsTextView(hundredthsTextView) {}

    void setTimeString(int64_t accumulatedTime);
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_STOPWATCHTEXTCONTROLLER_H__
