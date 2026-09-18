#include <stopwatch.h>

#include <algorithm>
#include <climits>

#include <utils.h>

namespace cdroid {
namespace deskclock {
namespace data {

Stopwatch::Stopwatch(State state, int64_t lastStartTime, int64_t lastWallClockTime,
                     int64_t accumulatedTime)
    : state(state), lastStartTime(lastStartTime), lastWallClockTime(lastWallClockTime),
      accumulatedTime(accumulatedTime) {
}

int64_t Stopwatch::getTotalTime() const {
    if (state != State::RUNNING) {
        return accumulatedTime;
    }

    // In practice, "now" can be any value due to device reboots. When the real-time clock
    // is reset, there is no more guarantee that "now" falls after the last start time. To
    // ensure the stopwatch is monotonically increasing, normalize negative time segments to 0
    const int64_t timeSinceStart = Utils::now() - lastStartTime;
    return accumulatedTime + std::max((int64_t) 0, timeSinceStart);
}

Stopwatch Stopwatch::start() const {
    if (state == State::RUNNING) {
        return *this;
    }
    return Stopwatch(State::RUNNING, Utils::now(), Utils::wallClock(), getTotalTime());
}

Stopwatch Stopwatch::pause() const {
    if (state != State::RUNNING) {
        return *this;
    }
    return Stopwatch(State::PAUSED, UNUSED, UNUSED, getTotalTime());
}

Stopwatch Stopwatch::reset() const {
    return RESET_STOPWATCH();
}

Stopwatch Stopwatch::updateAfterReboot() const {
    if (state != State::RUNNING) {
        return *this;
    }
    const int64_t timeSinceBoot = Utils::now();
    const int64_t wallClockTime = Utils::wallClock();
    // Avoid negative time deltas. They can happen in practice, but they can't be used. Simply
    // update the recorded times and proceed with no change in accumulated time.
    const int64_t delta = std::max((int64_t) 0, wallClockTime - lastWallClockTime);
    return Stopwatch(state, timeSinceBoot, wallClockTime, accumulatedTime + delta);
}

Stopwatch Stopwatch::updateAfterTimeSet() const {
    if (state != State::RUNNING) {
        return *this;
    }
    const int64_t timeSinceBoot = Utils::now();
    const int64_t wallClockTime = Utils::wallClock();
    const int64_t delta = timeSinceBoot - lastStartTime;
    if (delta < 0) {
        // Avoid negative time deltas. They typically happen following reboots when TIME_SET is
        // broadcast before BOOT_COMPLETED. Simply ignore the time update and hope
        // updateAfterReboot() can successfully correct the data at a later time.
        return *this;
    }
    return Stopwatch(state, timeSinceBoot, wallClockTime, accumulatedTime + delta);
}

const Stopwatch& Stopwatch::RESET_STOPWATCH() {
    static const Stopwatch RESET_STOPWATCH(State::RESET, UNUSED, UNUSED, 0);
    return RESET_STOPWATCH;
}

} // namespace data
} // namespace deskclock
} // namespace cdroid
