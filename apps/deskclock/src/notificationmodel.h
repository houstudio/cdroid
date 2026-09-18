#ifndef __DESKCLOCK_NOTIFICATIONMODEL_H__
#define __DESKCLOCK_NOTIFICATIONMODEL_H__
/*********************************************************************************
 * Port of com.android.deskclock.data.NotificationModel — notification ids and
 * the application-in-foreground flag. Notifications themselves are cut on
 * cdroid; the flag still gates notification-side model behavior.
 *********************************************************************************/
#include <string>

namespace cdroid {
namespace deskclock {
namespace data {

class NotificationModel {
private:
    bool mApplicationInForeground = false;

public:
    int getStopwatchNotificationId() const { return 1; }
    int getUnexpiredTimerNotificationId() const { return 2; }
    int getExpiredTimerNotificationId() const { return 3; }
    int getMissedTimerNotificationId() const { return 4; }

    std::string getStopwatchNotificationGroupKey() const { return "StopwatchNotification"; }
    std::string getTimerNotificationGroupKey() const { return "TimerNotification"; }
    std::string getTimerNotificationSortKey() const { return "T"; }
    std::string getTimerNotificationMissedSortKey() const { return "S"; }

    bool isApplicationInForeground() const { return mApplicationInForeground; }
    void setApplicationInForeground(bool inForeground) {
        mApplicationInForeground = inForeground;
    }
};

} // namespace data
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_NOTIFICATIONMODEL_H__
