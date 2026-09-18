#ifndef __DESKCLOCK_STOPWATCH_H__
#define __DESKCLOCK_STOPWATCH_H__
/*********************************************************************************
 * Port of com.android.deskclock.data.{Stopwatch,Lap} — read-only domain objects
 * for the stopwatch; state transitions return copies.
 *********************************************************************************/
#include <cstdint>
#include <list>

namespace cdroid {
namespace deskclock {
namespace data {

/** A read-only domain object representing a stopwatch lap. */
struct Lap {
    /** The 1-based position of the lap. */
    int lapNumber;
    /** Elapsed time in ms since the lap was last started. */
    int64_t lapTime;
    /** Elapsed time in ms accumulated for all laps up to and including this one. */
    int64_t accumulatedTime;

    Lap() : lapNumber(0), lapTime(0), accumulatedTime(0) {}
    Lap(int lapNumber, int64_t lapTime, int64_t accumulatedTime)
        : lapNumber(lapNumber), lapTime(lapTime), accumulatedTime(accumulatedTime) {}
};

/** A read-only domain object representing a stopwatch. */
class Stopwatch {
public:
    enum class State { RESET, RUNNING, PAUSED };

    State state;
    /** Elapsed time in ms the stopwatch was last started; UNUSED if not running. */
    int64_t lastStartTime;
    /** The time since epoch at which the stopwatch was last started. */
    int64_t lastWallClockTime;
    /** Elapsed time in ms this stopwatch has accumulated while running. */
    int64_t accumulatedTime;

    Stopwatch(State state, int64_t lastStartTime, int64_t lastWallClockTime,
              int64_t accumulatedTime);

    bool isReset() const { return state == State::RESET; }
    bool isPaused() const { return state == State::PAUSED; }
    bool isRunning() const { return state == State::RUNNING; }

    /** @return the total amount of time accumulated up to this moment. */
    int64_t getTotalTime() const;

    /** @return a copy of this stopwatch that is running. */
    Stopwatch start() const;
    /** @return a copy of this stopwatch that is paused. */
    Stopwatch pause() const;
    /** @return a copy of this stopwatch that is reset. */
    Stopwatch reset() const;
    /** @return this if not running or an updated version based on wallclock time. */
    Stopwatch updateAfterReboot() const;
    /** @return this if not running or an updated version based on the realtime clock. */
    Stopwatch updateAfterTimeSet() const;

    static constexpr int64_t UNUSED = INT64_MIN;

    /** The single, immutable instance of a reset stopwatch. */
    static const Stopwatch& RESET_STOPWATCH();
};

} // namespace data
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_STOPWATCH_H__
