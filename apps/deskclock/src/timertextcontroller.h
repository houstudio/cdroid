#ifndef __DESKCLOCK_TIMERTEXTCONTROLLER_H__
#define __DESKCLOCK_TIMERTEXTCONTROLLER_H__
/*********************************************************************************
 * Port of com.android.deskclock.TimerTextController — formats a time in millis
 * (possibly negative for expired timers) into a single TextView.
 *********************************************************************************/
#include <widget/textview.h>

namespace cdroid {
namespace deskclock {

class TimerTextController {
private:
    TextView& mTextView;

public:
    explicit TimerTextController(TextView& textView) : mTextView(textView) {}

    void setTimeString(int64_t remainingTime);
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_TIMERTEXTCONTROLLER_H__
