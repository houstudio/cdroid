#include <stopwatchtextcontroller.h>

#include <climits>

#include <core/context.h>

#include <uidata.h>
#include <utils.h>

namespace cdroid {
namespace deskclock {

void StopwatchTextController::setTimeString(int64_t accumulatedTime) {
    constexpr int64_t HOUR_IN_MILLIS = 60LL * 60 * 1000;
    constexpr int64_t MINUTE_IN_MILLIS = 60LL * 1000;
    constexpr int64_t SECOND_IN_MILLIS = 1000LL;

    // Since time is only displayed to centiseconds, if there is a change at the milliseconds
    // level but not the centiseconds level, we can avoid unnecessary work.
    if (mLastTime / 10 == accumulatedTime / 10) {
        return;
    }

    int hours = (int) (accumulatedTime / HOUR_IN_MILLIS);
    int64_t remainder = accumulatedTime % HOUR_IN_MILLIS;

    int minutes = (int) (remainder / MINUTE_IN_MILLIS);
    remainder = remainder % MINUTE_IN_MILLIS;

    int seconds = (int) (remainder / SECOND_IN_MILLIS);
    remainder = remainder % SECOND_IN_MILLIS;

    mHundredthsTextView.setText(uidata::UiDataModel::getUiDataModel()
            .getFormattedNumber((int) (remainder / 10), 2));

    // Avoid unnecessary computations and garbage creation if seconds have not changed since
    // last layout pass.
    if (mLastTime / SECOND_IN_MILLIS != accumulatedTime / SECOND_IN_MILLIS) {
        Context& context = *mMainTextView.getContext();
        const std::string time = Utils::getTimeString(context, hours, minutes, seconds);
        mMainTextView.setText(time);
    }
    mLastTime = accumulatedTime;
}

} // namespace deskclock
} // namespace cdroid
