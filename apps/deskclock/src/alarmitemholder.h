#ifndef __DESKCLOCK_ALARMITEMHOLDER_H__
#define __DESKCLOCK_ALARMITEMHOLDER_H__
/*********************************************************************************
 * Port of com.android.deskclock.alarms.dataadapter.AlarmItemHolder.
 *********************************************************************************/
#include <itemadapter.h>

#include <alarm.h>
#include <alarminstance.h>
#include <alarmtimeclickhandler.h>

namespace cdroid {
namespace deskclock {
namespace alarms {

/** View types are the layout ids upstream; aliased here since the factories map them. */
int alarmCollapsedViewType();
int alarmExpandedViewType();

class AlarmItemHolder : public TypedItemHolder<data::Alarm> {
public:
    /** The firing instance, or nullptr when the alarm cannot preemptively dismiss. */
    const bool hasAlarmInstance;
    const data::Alarminstance alarmInstance;
    AlarmTimeClickHandler* const alarmTimeClickHandler;

    AlarmItemHolder(const data::Alarm& alarm, const data::Alarminstance* instance,
                    AlarmTimeClickHandler* clickHandler)
        : TypedItemHolder(alarm, alarm.id), hasAlarmInstance(instance != nullptr),
          alarmInstance(instance != nullptr ? *instance : data::Alarminstance()),
          alarmTimeClickHandler(clickHandler) {}

    bool isExpanded = false;

    int getItemViewType() const override {
        return isExpanded ? alarmExpandedViewType() : alarmCollapsedViewType();
    }

    void expand() {
        if (!isExpanded) {
            isExpanded = true;
            notifyItemChanged();
        }
    }

    void collapse() {
        if (isExpanded) {
            isExpanded = false;
            notifyItemChanged();
        }
    }

    /** Payload marker for the repeat-days animation (upstream ANIMATE_REPEAT_DAYS). */
    static constexpr int ANIMATE_REPEAT_DAYS = 1000;
};

} // namespace alarms
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_ALARMITEMHOLDER_H__
