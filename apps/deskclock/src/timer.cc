#include <timer.h>

#include <algorithm>
#include <stdexcept>
#include <climits>

#include <utils.h>

namespace cdroid {
namespace deskclock {
namespace data {

namespace {
constexpr int64_t HOUR_IN_MILLIS = 60LL * 60 * 1000;
constexpr int64_t MINUTE_IN_MILLIS = 60LL * 1000;
constexpr int64_t SECOND_IN_MILLIS = 1000LL;
} // namespace

Timer::Timer(int id, State state, int64_t length, int64_t totalLength, int64_t lastStartTime,
             int64_t lastWallClockTime, int64_t lastRemainingTime, const std::string& label,
             bool deleteAfterUse)
    : id(id), state(state), length(length), totalLength(totalLength),
      lastStartTime(lastStartTime), lastWallClockTime(lastWallClockTime),
      lastRemainingTime(lastRemainingTime), label(label), deleteAfterUse(deleteAfterUse) {
}

bool Timer::stateFromValue(int value, State& outState) {
    switch (value) {
        case 1: outState = State::RUNNING; return true;
        case 2: outState = State::PAUSED; return true;
        case 3: outState = State::EXPIRED; return true;
        case 4: outState = State::RESET; return true;
        case 5: outState = State::MISSED; return true;
        default: return false;
    }
}

int64_t Timer::getRemainingTime() const {
    if (state == State::PAUSED || state == State::RESET) {
        return lastRemainingTime;
    }

    // In practice, "now" can be any value due to device reboots. When the real-time clock
    // is reset, there is no more guarantee that "now" falls after the last start time. To
    // ensure the timer is monotonically decreasing, normalize negative time segments to 0,
    const int64_t timeSinceStart = Utils::now() - lastStartTime;
    return lastRemainingTime - std::max((int64_t) 0, timeSinceStart);
}

int64_t Timer::getExpirationTime() const {
    if (state != State::RUNNING && state != State::EXPIRED && state != State::MISSED) {
        throw std::logic_error("cannot compute expiration time in state " + std::to_string((int) state));
    }
    return lastStartTime + lastRemainingTime;
}

int64_t Timer::getWallClockExpirationTime() const {
    if (state != State::RUNNING && state != State::EXPIRED && state != State::MISSED) {
        throw std::logic_error("cannot compute expiration time in state " + std::to_string((int) state));
    }
    return lastWallClockTime + lastRemainingTime;
}

Timer Timer::start() const {
    if (state == State::RUNNING || state == State::EXPIRED || state == State::MISSED) {
        return *this;
    }
    return Timer(id, State::RUNNING, length, totalLength,
                 Utils::now(), Utils::wallClock(), lastRemainingTime, label, deleteAfterUse);
}

Timer Timer::pause() const {
    if (state == State::PAUSED || state == State::RESET) {
        return *this;
    } else if (state == State::EXPIRED || state == State::MISSED) {
        return reset();
    }

    const int64_t remainingTime = getRemainingTime();
    return Timer(id, State::PAUSED, length, totalLength, UNUSED, UNUSED, remainingTime, label,
                 deleteAfterUse);
}

Timer Timer::expire() const {
    if (state == State::EXPIRED || state == State::RESET || state == State::MISSED) {
        return *this;
    }

    const int64_t remainingTime = std::min((int64_t) 0, lastRemainingTime);
    return Timer(id, State::EXPIRED, length, 0LL, Utils::now(),
                 Utils::wallClock(), remainingTime, label, deleteAfterUse);
}

Timer Timer::miss() const {
    if (state == State::RESET || state == State::MISSED) {
        return *this;
    }

    const int64_t remainingTime = std::min((int64_t) 0, lastRemainingTime);
    return Timer(id, State::MISSED, length, 0LL, Utils::now(),
                 Utils::wallClock(), remainingTime, label, deleteAfterUse);
}

Timer Timer::reset() const {
    if (state == State::RESET) {
        return *this;
    }
    return Timer(id, State::RESET, length, length, UNUSED, UNUSED, length, label, deleteAfterUse);
}

Timer Timer::updateAfterReboot() const {
    if (state == State::RESET || state == State::PAUSED) {
        return *this;
    }
    const int64_t timeSinceBoot = Utils::now();
    const int64_t wallClockTime = Utils::wallClock();
    // Avoid negative time deltas. They can happen in practice, but they can't be used. Simply
    // update the recorded times and proceed with no change in accumulated time.
    const int64_t delta = std::max((int64_t) 0, wallClockTime - lastWallClockTime);
    const int64_t remainingTime = lastRemainingTime - delta;
    return Timer(id, state, length, totalLength, timeSinceBoot, wallClockTime,
                 remainingTime, label, deleteAfterUse);
}

Timer Timer::updateAfterTimeSet() const {
    if (state == State::RESET || state == State::PAUSED) {
        return *this;
    }
    const int64_t timeSinceBoot = Utils::now();
    const int64_t wallClockTime = Utils::wallClock();
    const int64_t delta = timeSinceBoot - lastStartTime;
    const int64_t remainingTime = lastRemainingTime - delta;
    if (delta < 0) {
        // Avoid negative time deltas. They typically happen following reboots when TIME_SET is
        // broadcast before BOOT_COMPLETED. Simply ignore the time update and hope
        // updateAfterReboot() can successfully correct the data at a later time.
        return *this;
    }
    return Timer(id, state, length, totalLength, timeSinceBoot, wallClockTime,
                 remainingTime, label, deleteAfterUse);
}

Timer Timer::setLabel(const std::string& label) const {
    if (this->label == label) {
        return *this;
    }
    return Timer(id, state, length, totalLength, lastStartTime,
                 lastWallClockTime, lastRemainingTime, label, deleteAfterUse);
}

Timer Timer::setLength(int64_t length) const {
    if (this->length == length || length <= MIN_LENGTH) {
        return *this;
    }

    int64_t totalLength;
    int64_t remainingTime;
    if (state == State::RESET) {
        totalLength = length;
        remainingTime = length;
    } else {
        totalLength = this->totalLength;
        remainingTime = lastRemainingTime;
    }

    return Timer(id, state, length, totalLength, lastStartTime,
                 lastWallClockTime, remainingTime, label, deleteAfterUse);
}

Timer Timer::setRemainingTime(int64_t remainingTime) const {
    // Do not change the remaining time of a reset timer.
    if (lastRemainingTime == remainingTime || state == State::RESET) {
        return *this;
    }

    const int64_t delta = remainingTime - lastRemainingTime;
    const int64_t totalLength = this->totalLength + delta;

    int64_t lastStartTime;
    int64_t lastWallClockTime;
    State state;
    if (remainingTime > 0 && (this->state == State::EXPIRED || this->state == State::MISSED)) {
        state = State::RUNNING;
        lastStartTime = Utils::now();
        lastWallClockTime = Utils::wallClock();
    } else {
        state = this->state;
        lastStartTime = this->lastStartTime;
        lastWallClockTime = this->lastWallClockTime;
    }

    return Timer(id, state, length, totalLength, lastStartTime,
                 lastWallClockTime, remainingTime, label, deleteAfterUse);
}

Timer Timer::addMinute() const {
    if (state == State::EXPIRED || state == State::MISSED) {
        // Expired and missed timers restart with 60 seconds of remaining time.
        return setRemainingTime(MINUTE_IN_MILLIS);
    }
    // Otherwise try to add a minute to the remaining time.
    return setRemainingTime(lastRemainingTime + MINUTE_IN_MILLIS);
}

bool Timer::ID_COMPARATOR(const Timer& timer1, const Timer& timer2) {
    return timer2.id < timer1.id; // timer2.id.compareTo(timer1.id) < 0
}

bool Timer::EXPIRY_COMPARATOR(const Timer& timer1, const Timer& timer2) {
    static const State STATE_EXPIRY_ORDER[] = {
        State::MISSED, State::EXPIRED, State::RUNNING, State::PAUSED, State::RESET};

    auto stateIndex = [](State s) {
        for (int i = 0; i < 5; i++) {
            if (STATE_EXPIRY_ORDER[i] == s) return i;
        }
        return 5;
    };

    const int stateIndex1 = stateIndex(timer1.state);
    const int stateIndex2 = stateIndex(timer2.state);

    if (stateIndex1 != stateIndex2) return stateIndex1 < stateIndex2;

    if (timer1.state == State::RESET) {
        return timer1.length < timer2.length;
    }
    return timer1.lastRemainingTime < timer2.lastRemainingTime;
}

} // namespace data
} // namespace deskclock
} // namespace cdroid
