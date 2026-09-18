#include <stopwatchmodel.h>

#include <algorithm>

#include <content/sharedpreferences.h>
#include <core/context.h>

#include <notificationmodel.h>
#include <stopwatchdao.h>

namespace cdroid {
namespace deskclock {
namespace data {

StopwatchModel::StopwatchModel(Context& context, SharedPreferences& prefs,
                               NotificationModel& notificationModel)
    : mContext(context), mPrefs(prefs), mNotificationModel(notificationModel) {
}

void StopwatchModel::addStopwatchListener(const StopwatchListener& stopwatchListener) {
    mStopwatchListeners.push_back(stopwatchListener);
}

void StopwatchModel::removeStopwatchListener(const StopwatchListener& stopwatchListener) {
    for (auto it = mStopwatchListeners.begin(); it != mStopwatchListeners.end(); ++it) {
        if (*it == stopwatchListener) {
            mStopwatchListeners.erase(it);
            return;
        }
    }
}

const Stopwatch& StopwatchModel::getStopwatch() {
    if (!mStopwatchLoaded) {
        mStopwatch = StopwatchDAO::getStopwatch(mPrefs);
        mStopwatchLoaded = true;
    }
    return mStopwatch;
}

Stopwatch StopwatchModel::setStopwatch(const Stopwatch& stopwatch) {
    const Stopwatch& before = getStopwatch();
    if (!(before.state == stopwatch.state
            && before.lastStartTime == stopwatch.lastStartTime
            && before.accumulatedTime == stopwatch.accumulatedTime)) {
        StopwatchDAO::setStopwatch(mPrefs, stopwatch);
        mStopwatch = stopwatch;

        // Refresh the stopwatch notification to reflect the latest stopwatch state.
        if (!mNotificationModel.isApplicationInForeground()) {
            updateNotification();
        }

        // Resetting the stopwatch implicitly clears the recorded laps.
        if (stopwatch.isReset()) {
            clearLaps();
        }

        // Notify listeners of the stopwatch change.
        for (StopwatchListener& stopwatchListener : mStopwatchListeners) {
            if (stopwatchListener.stopwatchUpdated) stopwatchListener.stopwatchUpdated(before, stopwatch);
        }
    }

    return stopwatch;
}

const std::vector<Lap>& StopwatchModel::getLaps() {
    if (!mLapsLoaded) {
        mLaps = StopwatchDAO::getLaps(mPrefs);
        mLapsLoaded = true;
    }
    return mLaps;
}

bool StopwatchModel::addLap(Lap& outLap) {
    if (!mStopwatchLoaded || !mStopwatch.isRunning() || !canAddMoreLaps()) {
        return false;
    }

    const int64_t totalTime = getStopwatch().getTotalTime();
    std::vector<Lap>& laps = const_cast<std::vector<Lap>&>(getLaps());

    const int lapNumber = (int) laps.size() + 1;
    StopwatchDAO::addLap(mPrefs, lapNumber, totalTime);

    const int64_t prevAccumulatedTime = laps.empty() ? 0 : laps[0].accumulatedTime;
    const int64_t lapTime = totalTime - prevAccumulatedTime;

    const Lap lap(lapNumber, lapTime, totalTime);
    laps.insert(laps.begin(), lap);
    outLap = lap;

    // Refresh the stopwatch notification to reflect the latest stopwatch state.
    if (!mNotificationModel.isApplicationInForeground()) {
        updateNotification();
    }

    // Notify listeners of the new lap.
    for (StopwatchListener& stopwatchListener : mStopwatchListeners) {
        if (stopwatchListener.lapAdded) stopwatchListener.lapAdded(lap);
    }

    return true;
}

void StopwatchModel::clearLaps() {
    StopwatchDAO::clearLaps(mPrefs);
    mLaps.clear();
    mLapsLoaded = true;
}

bool StopwatchModel::canAddMoreLaps() const {
    return mLaps.size() < 98;
}

int64_t StopwatchModel::getLongestLapTime() {
    int64_t maxLapTime = 0;

    const std::vector<Lap>& laps = getLaps();
    if (!laps.empty()) {
        // Compute the maximum lap time across all recorded laps.
        for (const Lap& lap : laps) {
            maxLapTime = std::max(maxLapTime, lap.lapTime);
        }

        // Compare with the maximum lap time for the current lap.
        const Stopwatch& stopwatch = getStopwatch();
        const int64_t currentLapTime = stopwatch.getTotalTime() - laps[0].accumulatedTime;
        maxLapTime = std::max(maxLapTime, currentLapTime);
    }

    return maxLapTime;
}

int64_t StopwatchModel::getCurrentLapTime(int64_t time) const {
    const Lap& previousLap = mLaps[0];
    const int64_t currentLapTime = time - previousLap.accumulatedTime;
    return std::max((int64_t) 0, currentLapTime);
}

void StopwatchModel::updateNotification() {
    // No notification surface on cdroid.
}

} // namespace data
} // namespace deskclock
} // namespace cdroid
