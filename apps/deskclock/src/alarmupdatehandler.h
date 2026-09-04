#ifndef __DESKCLOCK_ALARMUPDATEHANDLER_H__
#define __DESKCLOCK_ALARMUPDATEHANDLER_H__
/*********************************************************************************
 * Port of com.android.deskclock.alarms.AlarmUpdateHandler — API for mutating a
 * single alarm. Upstream mutates via AsyncTask on a background thread; cdroid
 * performs the same steps synchronously on the caller thread (single-process,
 * prefs-backed storage; recorded deviation).
 *********************************************************************************/
#include <view/viewgroup.h>

#include <alarm.h>
#include <alarminstance.h>

namespace cdroid {

namespace deskclock {

namespace alarms {

/** Port of alarms/ScrollHandler. */
class ScrollHandler {
public:
    virtual ~ScrollHandler() = default;
    virtual void smoothScrollTo(int position) = 0;
    virtual void setSmoothScrollStableId(int64_t stableId) = 0;
};

class AlarmUpdateHandler {
private:
    Context* mContext;
    ScrollHandler* mScrollHandler;
    ViewGroup* mSnackbarAnchor;

    /** For undo. */
    data::Alarm mDeletedAlarm;
    bool mHasDeletedAlarm = false;

public:
    AlarmUpdateHandler(Context* context, ScrollHandler* scrollHandler, ViewGroup* snackbarAnchor);

    /** Adds a new alarm. */
    void asyncAddAlarm(const data::Alarm& alarm);

    /**
     * Modifies an alarm, optionally popping a toast when done.
     * @param minorUpdate if true, don't affect any currently snoozed instances.
     */
    void asyncUpdateAlarm(const data::Alarm& alarm, bool popToast, bool minorUpdate);

    /** Deletes an alarm and offers undo. */
    void asyncDeleteAlarm(const data::Alarm& alarm);

    /** (Snackbar/toast faces are no-op stubs on cdroid.) */
    void showPredismissToast(const data::Alarminstance& instance);
    void hideUndoBar();

private:
    void showUndoBar();
    data::Alarminstance setupAlarmInstance(const data::Alarm& alarm);
};

} // namespace alarms
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_ALARMUPDATEHANDLER_H__
