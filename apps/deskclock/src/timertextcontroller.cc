#include <timertextcontroller.h>

#include <utils.h>

namespace cdroid {
namespace deskclock {

static constexpr int64_t SECOND_IN_MILLIS = 1000;
static constexpr int64_t MINUTE_IN_MILLIS = 60 * SECOND_IN_MILLIS;
static constexpr int64_t HOUR_IN_MILLIS = 60 * MINUTE_IN_MILLIS;

void TimerTextController::setTimeString(int64_t remainingTime) {
    int64_t variableRemainingTime = remainingTime;
    bool isNegative = false;
    if (variableRemainingTime < 0) {
        variableRemainingTime = -variableRemainingTime;
        isNegative = true;
    }

    int hours = (int) (variableRemainingTime / HOUR_IN_MILLIS);
    int64_t remainder = variableRemainingTime % HOUR_IN_MILLIS;

    int minutes = (int) (remainder / MINUTE_IN_MILLIS);
    remainder = remainder % MINUTE_IN_MILLIS;

    int seconds = (int) (remainder / SECOND_IN_MILLIS);
    remainder = remainder % SECOND_IN_MILLIS;

    // Round up to the next second.
    if (!isNegative && remainder != 0) {
        seconds++;
        if (seconds == 60) {
            seconds = 0;
            minutes++;
            if (minutes == 60) {
                minutes = 0;
                hours++;
            }
        }
    }

    std::string time = Utils::getTimeString(*mTextView.getContext(), hours, minutes, seconds);
    if (isNegative && !(hours == 0 && minutes == 0 && seconds == 0)) {
        time = "−" + time;
    }

    mTextView.setText(time);
}

} // namespace deskclock
} // namespace cdroid
