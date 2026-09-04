#include <timerdao.h>

#include <content/sharedpreferences.h>

namespace cdroid {
namespace deskclock {
namespace data {

namespace {
// Keys/prefixes mirroring the Kotlin TimerDAO.
constexpr const char* TIMER_IDS = "timers_list";
constexpr const char* NEXT_TIMER_ID = "next_timer_id";
constexpr const char* STATE = "timer_state_";
constexpr const char* LENGTH = "timer_setup_timet_";
constexpr const char* TOTAL_LENGTH = "timer_original_timet_";
constexpr const char* LAST_START_TIME = "timer_start_time_";
constexpr const char* LAST_WALL_CLOCK_TIME = "timer_wall_clock_time_";
constexpr const char* REMAINING_TIME = "timer_time_left_";
constexpr const char* LABEL = "timer_label_";
constexpr const char* DELETE_AFTER_USE = "delete_after_use_";

std::string key(const char* prefix, int id) {
    return std::string(prefix) + std::to_string(id);
}
} // namespace

std::vector<Timer> TimerDAO::getTimers(SharedPreferences& prefs) {
    // Read the set of timer ids.
    const std::set<std::string> timerIds = prefs.getStringSet(TIMER_IDS, {});
    std::vector<Timer> timers;

    // Build a timer using the data associated with each timer id.
    for (const std::string& timerId : timerIds) {
        const int id = atoi(timerId.c_str());
        const int stateValue = prefs.getInt(key(STATE, id), (int) Timer::State::RESET);
        Timer::State state;
        if (!Timer::stateFromValue(stateValue, state)) {
            // Timer state may be null when migrating timers from prior releases which defined a
            // "deleted" state. Such a state is no longer required.
            continue;
        }

        const int64_t length = prefs.getLong(key(LENGTH, id), INT64_MIN);
        const int64_t totalLength = prefs.getLong(key(TOTAL_LENGTH, id), INT64_MIN);
        const int64_t lastStartTime = prefs.getLong(key(LAST_START_TIME, id), Timer::UNUSED);
        const int64_t lastWallClockTime = prefs.getLong(key(LAST_WALL_CLOCK_TIME, id), Timer::UNUSED);
        const int64_t remainingTime = prefs.getLong(key(REMAINING_TIME, id), totalLength);
        const std::string label = prefs.getString(key(LABEL, id), "");
        const bool deleteAfterUse = prefs.getBoolean(key(DELETE_AFTER_USE, id), false);
        timers.push_back(Timer(id, state, length, totalLength, lastStartTime,
                lastWallClockTime, remainingTime, label, deleteAfterUse));
    }

    return timers;
}

Timer TimerDAO::addTimer(SharedPreferences& prefs, const Timer& timer) {
    SharedPreferences::Editor& editor = prefs.edit();

    // Fetch the next timer id.
    const int id = prefs.getInt(NEXT_TIMER_ID, 0);
    editor.putInt(NEXT_TIMER_ID, id + 1);

    // Add the new timer id to the set of all timer ids.
    std::set<std::string> timerIds = getTimerIds(prefs);
    timerIds.insert(std::to_string(id));
    editor.putStringSet(TIMER_IDS, timerIds);

    // Record the fields of the timer.
    editor.putInt(key(STATE, id), (int) timer.state);
    editor.putLong(key(LENGTH, id), timer.length);
    editor.putLong(key(TOTAL_LENGTH, id), timer.totalLength);
    editor.putLong(key(LAST_START_TIME, id), timer.lastStartTime);
    editor.putLong(key(LAST_WALL_CLOCK_TIME, id), timer.lastWallClockTime);
    editor.putLong(key(REMAINING_TIME, id), timer.getRemainingTime());
    editor.putString(key(LABEL, id), timer.label);
    editor.putBoolean(key(DELETE_AFTER_USE, id), timer.deleteAfterUse);

    editor.apply();

    // Return a new timer with the generated timer id present.
    return Timer(id, timer.state, timer.length, timer.totalLength, timer.lastStartTime,
            timer.lastWallClockTime, timer.getRemainingTime(), timer.label, timer.deleteAfterUse);
}

void TimerDAO::updateTimer(SharedPreferences& prefs, const Timer& timer) {
    SharedPreferences::Editor& editor = prefs.edit();

    // Record the fields of the timer.
    const int id = timer.id;
    editor.putInt(key(STATE, id), (int) timer.state);
    editor.putLong(key(LENGTH, id), timer.length);
    editor.putLong(key(TOTAL_LENGTH, id), timer.totalLength);
    editor.putLong(key(LAST_START_TIME, id), timer.lastStartTime);
    editor.putLong(key(LAST_WALL_CLOCK_TIME, id), timer.lastWallClockTime);
    editor.putLong(key(REMAINING_TIME, id), timer.getRemainingTime());
    editor.putString(key(LABEL, id), timer.label);
    editor.putBoolean(key(DELETE_AFTER_USE, id), timer.deleteAfterUse);

    editor.apply();
}

void TimerDAO::removeTimer(SharedPreferences& prefs, const Timer& timer) {
    SharedPreferences::Editor& editor = prefs.edit();
    const int id = timer.id;

    // Remove the timer id from the set of all timer ids.
    std::set<std::string> timerIds = getTimerIds(prefs);
    timerIds.erase(std::to_string(id));
    if (timerIds.empty()) {
        editor.remove(TIMER_IDS);
        editor.remove(NEXT_TIMER_ID);
    } else {
        editor.putStringSet(TIMER_IDS, timerIds);
    }

    // Record the fields of the timer.
    editor.remove(key(STATE, id));
    editor.remove(key(LENGTH, id));
    editor.remove(key(TOTAL_LENGTH, id));
    editor.remove(key(LAST_START_TIME, id));
    editor.remove(key(LAST_WALL_CLOCK_TIME, id));
    editor.remove(key(REMAINING_TIME, id));
    editor.remove(key(LABEL, id));
    editor.remove(key(DELETE_AFTER_USE, id));

    editor.apply();
}

std::set<std::string> TimerDAO::getTimerIds(SharedPreferences& prefs) {
    return prefs.getStringSet(TIMER_IDS, {});
}

} // namespace data
} // namespace deskclock
} // namespace cdroid
