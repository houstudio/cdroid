#ifndef __DESKCLOCK_STOPWATCHMODEL_H__
#define __DESKCLOCK_STOPWATCHMODEL_H__
/*********************************************************************************
 * Port of com.android.deskclock.data.StopwatchModel — stopwatch state, laps,
 * and listener notification. Notification updates are no-ops on cdroid.
 *********************************************************************************/
#include <vector>

#include <datalisteners.h>
#include <stopwatch.h>

namespace cdroid {

class Context;
class SharedPreferences;

namespace deskclock {
namespace data {

class NotificationModel;

class StopwatchModel {
private:
    Context& mContext;
    SharedPreferences& mPrefs;
    NotificationModel& mNotificationModel;

    std::vector<StopwatchListener> mStopwatchListeners;

    bool mStopwatchLoaded = false;
    Stopwatch mStopwatch = Stopwatch::RESET_STOPWATCH();

    bool mLapsLoaded = false;
    std::vector<Lap> mLaps;

public:
    StopwatchModel(Context& context, SharedPreferences& prefs,
                   NotificationModel& notificationModel);

    void addStopwatchListener(const StopwatchListener& stopwatchListener);
    void removeStopwatchListener(const StopwatchListener& stopwatchListener);

    const Stopwatch& getStopwatch();
    Stopwatch setStopwatch(const Stopwatch& stopwatch);

    const std::vector<Lap>& getLaps();

    /** @return the newly added lap, or false if no lap can be added. */
    bool addLap(Lap& outLap);

    void clearLaps();

    bool canAddMoreLaps() const;

    int64_t getLongestLapTime();

    int64_t getCurrentLapTime(int64_t time) const;

private:
    void updateNotification();
};

} // namespace data
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_STOPWATCHMODEL_H__
