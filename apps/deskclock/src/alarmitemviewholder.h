#ifndef __DESKCLOCK_ALARMITEMVIEWHOLDER_H__
#define __DESKCLOCK_ALARMITEMVIEWHOLDER_H__
/*********************************************************************************
 * Port of com.android.deskclock.alarms.dataadapter.AlarmItemViewHolder — the
 * abstract ViewHolder for alarm time items (clock/on-off/arrow/dismiss button).
 *********************************************************************************/
#include <itemadapter.h>

#include <alarm.h>
#include <texttime.h>
#include <widget/compoundbutton.h>
#include <widget/imageview.h>
#include <widget/textview.h>

namespace cdroid {
namespace deskclock {
namespace alarms {

class AlarmItemHolder;
class AlarmTimeClickHandler;

class AlarmItemViewHolder : public ItemViewHolder {
public:
    TextTime* clock;
    CompoundButton* onOff;
    ImageView* arrow;
    // (preemptive_dismiss_button is not present in the cdroid layout subset; the
    //  bind method degrades to no-op when null.)

    explicit AlarmItemViewHolder(View* itemView);

protected:
    void onBindItemView(ItemHolder& itemHolder) override;

    void bindOnOffSwitch(const data::Alarm& alarm);
    void bindClock(const data::Alarm& alarm);
    bool bindPreemptiveDismissButton(Context& context, const data::Alarm& alarm,
                                     const data::Alarminstance* alarmInstance);

    AlarmTimeClickHandler* getClickHandler();

    static constexpr float CLOCK_ENABLED_ALPHA = 1.0f;
    static constexpr float CLOCK_DISABLED_ALPHA = 0.69f;
};

} // namespace alarms
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_ALARMITEMVIEWHOLDER_H__
