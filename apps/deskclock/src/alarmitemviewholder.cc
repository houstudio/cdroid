#include <alarmitemviewholder.h>

#include <R.h>

#include <core/context.h>
#include <text/String.h>

#include <alarmitemholder.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace alarms {

AlarmItemViewHolder::AlarmItemViewHolder(View* itemView) : ItemViewHolder(itemView) {
    clock = (TextTime*) itemView->findViewById(R::id::digital_clock);
    onOff = (CompoundButton*) itemView->findViewById(R::id::onoff);
    arrow = (ImageView*) itemView->findViewById(R::id::arrow);

    onOff->setOnCheckedChangeListener([this](CompoundButton&, bool isChecked) {
        AlarmItemHolder* holder = dynamic_cast<AlarmItemHolder*>(itemHolder);
        if (holder != nullptr && holder->alarmTimeClickHandler != nullptr) {
            holder->alarmTimeClickHandler->setAlarmEnabled(holder->item, isChecked);
        }
    });
}

AlarmTimeClickHandler* AlarmItemViewHolder::getClickHandler() {
    AlarmItemHolder* holder = dynamic_cast<AlarmItemHolder*>(itemHolder);
    return holder != nullptr ? holder->alarmTimeClickHandler : nullptr;
}

void AlarmItemViewHolder::onBindItemView(ItemHolder& itemHolder) {
    AlarmItemHolder* holder = dynamic_cast<AlarmItemHolder*>(&itemHolder);
    const data::Alarm& alarm = holder->item;
    bindOnOffSwitch(alarm);
    bindClock(alarm);
    Context& context = *itemView->getContext();
    // String::toString() returns an owned copy (caller deletes).
    String* clockText = clock->getText().toString();
    itemView->setContentDescription(clockText->str() + " "
            + alarm.getLabelOrDefault(context));
    delete clockText;
}

void AlarmItemViewHolder::bindOnOffSwitch(const data::Alarm& alarm) {
    if (onOff->isChecked() != alarm.enabled) {
        onOff->setChecked(alarm.enabled);
    }
}

void AlarmItemViewHolder::bindClock(const data::Alarm& alarm) {
    clock->setTime(alarm.hour, alarm.minutes);
    clock->setAlpha(alarm.enabled ? CLOCK_ENABLED_ALPHA : CLOCK_DISABLED_ALPHA);
}

bool AlarmItemViewHolder::bindPreemptiveDismissButton(Context& context,
                                                       const data::Alarm& alarm,
                                                       const data::Alarminstance* alarmInstance) {
    // preemptive_dismiss_button is absent from the cdroid layout subset; nothing to bind.
    return alarm.canPreemptivelyDismiss() && alarmInstance != nullptr;
}

} // namespace alarms
} // namespace deskclock
} // namespace cdroid
