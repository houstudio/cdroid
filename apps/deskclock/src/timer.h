#ifndef __DESKCLOCK_TIMER_H__
#define __DESKCLOCK_TIMER_H__
/*********************************************************************************
 * Port of com.android.deskclock.data.Timer — a read-only domain object
 * representing a countdown timer; state transitions return copies.
 *********************************************************************************/
#include <string>

namespace cdroid {
namespace deskclock {
namespace data {

class Timer {
public:
    enum class State {
        RUNNING = 1, PAUSED = 2, EXPIRED = 3, RESET = 4, MISSED = 5
    };

    /** A unique identifier for the timer. */
    int id;
    /** The current state of the timer. */
    State state;
    /** The original length of the timer in milliseconds when it was created. */
    int64_t length;
    /** The length of the timer in milliseconds including additional time added by the user. */
    int64_t totalLength;
    /** The time at which the timer was last started; UNUSED when not running. */
    int64_t lastStartTime;
    /** The time since epoch at which the timer was last started. */
    int64_t lastWallClockTime;
    /** The time remaining before expiry; negative if it is already expired. */
    int64_t lastRemainingTime;
    /** A message describing the meaning of the timer. */
    std::string label;
    /** A flag indicating the timer should be deleted when it is reset. */
    bool deleteAfterUse;

    Timer(int id, State state, int64_t length, int64_t totalLength, int64_t lastStartTime,
          int64_t lastWallClockTime, int64_t lastRemainingTime, const std::string& label,
          bool deleteAfterUse);

    /** Default (RESET, no time) — lets locals/out-params default-construct. */
    Timer() : Timer(-1, State::RESET, 0, 0, UNUSED, UNUSED, 0, "", false) {}

    /** @return the state corresponding to the given value; false if none. */
    static bool stateFromValue(int value, State& outState);

    bool isReset() const { return state == State::RESET; }
    bool isRunning() const { return state == State::RUNNING; }
    bool isPaused() const { return state == State::PAUSED; }
    bool isExpired() const { return state == State::EXPIRED; }
    bool isMissed() const { return state == State::MISSED; }

    /** @return total time remaining up to this moment; expired/missed timers go negative. */
    int64_t getRemainingTime() const;

    /** @return the elapsed realtime at which this timer will or did expire. */
    int64_t getExpirationTime() const;

    /** @return the wall clock time at which this timer will or did expire. */
    int64_t getWallClockExpirationTime() const;

    /** @return the total amount of time elapsed up to this moment. */
    int64_t getElapsedTime() const { return totalLength - getRemainingTime(); }

    /** @return a copy of this timer that is running, expired or missed. */
    Timer start() const;
    /** @return a copy of this timer that is paused or reset. */
    Timer pause() const;
    /** @return a copy of this timer that is expired, missed or reset. */
    Timer expire() const;
    /** @return a copy of this timer that is missed or reset. */
    Timer miss() const;
    /** @return a copy of this timer that is reset. */
    Timer reset() const;
    /** @return a copy of this timer that has its times adjusted after a reboot. */
    Timer updateAfterReboot() const;
    /** @return a copy of this timer that has its times adjusted after time has been set. */
    Timer updateAfterTimeSet() const;
    /** @return a copy of this timer with the given label. */
    Timer setLabel(const std::string& label) const;
    /** @return a copy of this timer with the given length, or this if not legally adjustable. */
    Timer setLength(int64_t length) const;
    /** @return a copy of this timer with the given remainingTime (or this if not adjustable). */
    Timer setRemainingTime(int64_t remainingTime) const;
    /** @return a copy with an additional minute added (expired/missed restart with 60s). */
    Timer addMinute() const;

    bool operator==(const Timer& other) const { return id == other.id; }
    bool operator!=(const Timer& other) const { return id != other.id; }

    /** The minimum duration of a timer. */
    static constexpr int64_t MIN_LENGTH = 1000LL;

    /** The maximum duration of a new timer created via the user interface. */
    static constexpr int64_t MAX_LENGTH =
            99LL * 3600000 + 99LL * 60000 + 99LL * 1000;

    static constexpr int64_t UNUSED = INT64_MIN;

    /** Orders timers by their IDs (newest at the top). */
    static bool ID_COMPARATOR(const Timer& timer1, const Timer& timer2);

    /** Orders timers by expected/actual expiration: MISSED, EXPIRED, RUNNING, PAUSED, RESET. */
    static bool EXPIRY_COMPARATOR(const Timer& timer1, const Timer& timer2);
};

} // namespace data
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_TIMER_H__
