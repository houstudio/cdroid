#include <stopwatchdao.h>

#include <algorithm>

#include <content/sharedpreferences.h>

namespace cdroid {
namespace deskclock {
namespace data {

namespace {
constexpr const char* STATE = "sw_state";
constexpr const char* LAST_START_TIME = "sw_start_time";
constexpr const char* LAST_WALL_CLOCK_TIME = "sw_wall_clock_time";
constexpr const char* ACCUMULATED_TIME = "sw_accum_time";
constexpr const char* LAP_COUNT = "sw_lap_num";
constexpr const char* LAP_ACCUMULATED_TIME = "sw_lap_time_";
} // namespace

Stopwatch StopwatchDAO::getStopwatch(SharedPreferences& prefs) {
    const int stateIndex = prefs.getInt(STATE, (int) Stopwatch::State::RESET);
    const Stopwatch::State state = (Stopwatch::State) stateIndex;
    const int64_t lastStartTime = prefs.getLong(LAST_START_TIME, Stopwatch::UNUSED);
    const int64_t lastWallClockTime = prefs.getLong(LAST_WALL_CLOCK_TIME, Stopwatch::UNUSED);
    const int64_t accumulatedTime = prefs.getLong(ACCUMULATED_TIME, 0);
    Stopwatch s(state, lastStartTime, lastWallClockTime, accumulatedTime);

    // If the stopwatch reports an illegal (negative) amount of time, remove the bad data.
    if (s.getTotalTime() < 0) {
        s = s.reset();
        setStopwatch(prefs, s);
    }
    return s;
}

void StopwatchDAO::setStopwatch(SharedPreferences& prefs, const Stopwatch& stopwatch) {
    SharedPreferences::Editor& editor = prefs.edit();

    if (stopwatch.isReset()) {
        editor.remove(STATE)
              .remove(LAST_START_TIME)
              .remove(LAST_WALL_CLOCK_TIME)
              .remove(ACCUMULATED_TIME);
    } else {
        editor.putInt(STATE, (int) stopwatch.state)
              .putLong(LAST_START_TIME, stopwatch.lastStartTime)
              .putLong(LAST_WALL_CLOCK_TIME, stopwatch.lastWallClockTime)
              .putLong(ACCUMULATED_TIME, stopwatch.accumulatedTime);
    }

    editor.apply();
}

std::vector<Lap> StopwatchDAO::getLaps(SharedPreferences& prefs) {
    // Prepare the container to be filled with laps.
    const int lapCount = prefs.getInt(LAP_COUNT, 0);
    std::vector<Lap> laps;

    int64_t prevAccumulatedTime = 0;

    // Lap numbers are 1-based and so the are corresponding shared preference keys.
    for (int lapNumber = 1; lapNumber <= lapCount; lapNumber++) {
        // Look up the accumulated time for the lap.
        const std::string lapAccumulatedTimeKey =
                std::string(LAP_ACCUMULATED_TIME) + std::to_string(lapNumber);
        const int64_t accumulatedTime = prefs.getLong(lapAccumulatedTimeKey, 0);

        // Lap time is the delta between accumulated time of this lap and prior lap.
        const int64_t lapTime = accumulatedTime - prevAccumulatedTime;

        // Create the lap instance from the data.
        laps.push_back(Lap(lapNumber, lapTime, accumulatedTime));

        // Update the accumulated time of the previous lap.
        prevAccumulatedTime = accumulatedTime;
    }

    // Laps are stored in the order they were recorded; display order is the reverse.
    std::reverse(laps.begin(), laps.end());

    return laps;
}

void StopwatchDAO::addLap(SharedPreferences& prefs, int newLapCount, int64_t accumulatedTime) {
    prefs.edit()
            .putInt(LAP_COUNT, newLapCount)
            .putLong(std::string(LAP_ACCUMULATED_TIME) + std::to_string(newLapCount),
                     accumulatedTime)
            .apply();
}

void StopwatchDAO::clearLaps(SharedPreferences& prefs) {
    SharedPreferences::Editor& editor = prefs.edit();

    const int lapCount = prefs.getInt(LAP_COUNT, 0);
    for (int lapNumber = 1; lapNumber <= lapCount; lapNumber++) {
        editor.remove(std::string(LAP_ACCUMULATED_TIME) + std::to_string(lapNumber));
    }
    editor.remove(LAP_COUNT);

    editor.apply();
}

} // namespace data
} // namespace deskclock
} // namespace cdroid
