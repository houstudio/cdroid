#include <expandedalarmviewholder.h>

#include <R.h>

#include <animation/animatorset.h>
#include <animation/objectanimator.h>
#include <view/view.h>

#include <collapsedalarmviewholder.h>

#include <view/layoutinflater.h>
#include <core/context.h>

#include <alarmitemholder.h>
#include <datamodel.h>
#include <uidata.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace alarms {

Animator* ExpandedAlarmViewHolder::onAnimateChange(RecyclerView::ViewHolder& oldHolder,
        RecyclerView::ViewHolder& newHolder, int64_t duration) {
    // Upstream ExpandedAlarmItemHolder: only the expand FROM a collapsed row
    // animates (staggered cross-fade); other rebinds stay unanimated.
    if (dynamic_cast<CollapsedAlarmViewHolder*>(&oldHolder) == nullptr) return nullptr;
    View* oldView = oldHolder.itemView;
    View* newView = newHolder.itemView;
    const float prevOldAlpha = oldView->getAlpha();
    const float prevNewAlpha = newView->getAlpha();
    newView->setAlpha(0.0f);
    std::vector<Animator*> animators;
    animators.push_back(ObjectAnimator::ofFloat(oldView, View::ALPHA, {0.0f}));
    animators.push_back(ObjectAnimator::ofFloat(newView, View::ALPHA, {1.0f}));
    AnimatorSet* set = new AnimatorSet();
    set->playTogether(animators);
    set->setDuration(duration);
    Animator::AnimatorListener listener;
    listener.onAnimationEnd = [oldView, newView, prevOldAlpha, prevNewAlpha]
            (Animator& animator, bool) {
        animator.removeAllListeners();
        oldView->setAlpha(prevOldAlpha);
        newView->setAlpha(prevNewAlpha);
    };
    set->addListener(listener);
    return set;
}

Animator* ExpandedAlarmViewHolder::onAnimateChange(std::vector<Object*>* /*payloads*/,
        int /*fromLeft*/, int /*fromTop*/, int /*fromRight*/, int /*fromBottom*/,
        int64_t /*duration*/) {
    // In-place payload animations (repeat-days) arrive with the payload pass.
    return nullptr;
}

ExpandedAlarmViewHolder::ExpandedAlarmViewHolder(View* itemView)
    : AlarmItemViewHolder(itemView) {
    Context& context = *this->itemView->getContext();
    repeat = (CheckBox*) itemView->findViewById(R::id::repeat_onoff);
    editLabel = (TextView*) itemView->findViewById(R::id::edit_label);
    repeatDays = (LinearLayout*) itemView->findViewById(R::id::repeat_days);
    vibrateCheckBox = (CheckBox*) itemView->findViewById(R::id::vibrate_onoff);
    ringtone = (TextView*) itemView->findViewById(R::id::choose_ringtone);
    deleteView = (TextView*) itemView->findViewById(R::id::delete_);
    hairLine = itemView->findViewById(R::id::hairline);

    // Build a button for each day.
    LayoutInflater* inflater = LayoutInflater::from(&context);
    const std::vector<int>& weekdays =
            data::Weekdays::Order::calendarDays(data::DataModel::getDataModel().getWeekdayOrder());
    for (int i = 0; i < 7; i++) {
        View* dayButtonFrame = inflater->inflate(R::layout::day_button, repeatDays, false);
        CompoundButton* dayButton =
                (CompoundButton*) dayButtonFrame->findViewById(R::id::day_button_box);
        const int weekday = weekdays[i];
        dayButton->setText(uidata::UiDataModel::getUiDataModel().getShortWeekday(weekday));
        dayButton->setContentDescription(
                uidata::UiDataModel::getUiDataModel().getLongWeekday(weekday));
        repeatDays->addView(dayButtonFrame);
        dayButtons[i] = dayButton;
    }

    // Collapse handler.
    itemView->setOnClickListener([this](View&) {
        AlarmItemHolder* holder = dynamic_cast<AlarmItemHolder*>(itemHolder);
        if (holder != nullptr) holder->collapse();
    });
    arrow->setOnClickListener([this](View&) {
        AlarmItemHolder* holder = dynamic_cast<AlarmItemHolder*>(itemHolder);
        if (holder != nullptr) holder->collapse();
    });
    // Edit time handler.
    clock->setOnClickListener([this](View&) {
        AlarmItemHolder* holder = dynamic_cast<AlarmItemHolder*>(itemHolder);
        if (holder != nullptr && holder->alarmTimeClickHandler != nullptr) {
            holder->alarmTimeClickHandler->onClockClicked(holder->item);
        }
    });
    // Edit label handler.
    editLabel->setOnClickListener([this](View&) {
        AlarmItemHolder* holder = dynamic_cast<AlarmItemHolder*>(itemHolder);
        if (holder != nullptr && holder->alarmTimeClickHandler != nullptr) {
            holder->alarmTimeClickHandler->onEditLabelClicked(holder->item);
        }
    });
    // Vibrator checkbox handler.
    vibrateCheckBox->setOnClickListener([this](View& view) {
        AlarmItemHolder* holder = dynamic_cast<AlarmItemHolder*>(itemHolder);
        if (holder != nullptr && holder->alarmTimeClickHandler != nullptr) {
            holder->alarmTimeClickHandler->setAlarmVibrationEnabled(holder->item,
                    ((CheckBox*) &view)->isChecked());
        }
    });
    // Ringtone editor handler.
    ringtone->setOnClickListener([this](View&) {
        AlarmItemHolder* holder = dynamic_cast<AlarmItemHolder*>(itemHolder);
        if (holder != nullptr && holder->alarmTimeClickHandler != nullptr) {
            holder->alarmTimeClickHandler->onRingtoneClicked(*this->itemView->getContext(),
                    holder->item);
        }
    });
    // Delete alarm handler.
    deleteView->setOnClickListener([this](View& view) {
        AlarmItemHolder* holder = dynamic_cast<AlarmItemHolder*>(itemHolder);
        if (holder != nullptr && holder->alarmTimeClickHandler != nullptr) {
            holder->alarmTimeClickHandler->onDeleteClicked(holder);
            view.announceForAccessibility(
                    view.getContext()->getString(R::string::alarm_deleted));
        }
    });
    // Repeat checkbox handler.
    repeat->setOnClickListener([this](View& view) {
        AlarmItemHolder* holder = dynamic_cast<AlarmItemHolder*>(itemHolder);
        const bool checked = ((CheckBox*) &view)->isChecked();
        if (holder != nullptr && holder->alarmTimeClickHandler != nullptr) {
            holder->alarmTimeClickHandler->setAlarmRepeatEnabled(holder->item, checked);
        }
    });
    // Day buttons handler.
    for (int i = 0; i < 7; i++) {
        dayButtons[i]->setOnClickListener([this, i](View& view) {
            AlarmItemHolder* holder = dynamic_cast<AlarmItemHolder*>(itemHolder);
            const bool isChecked = ((CompoundButton*) &view)->isChecked();
            if (holder != nullptr && holder->alarmTimeClickHandler != nullptr) {
                holder->alarmTimeClickHandler->setDayOfWeekEnabled(holder->item, isChecked, i);
            }
        });
    }
    itemView->setImportantForAccessibility(View::IMPORTANT_FOR_ACCESSIBILITY_NO);
}

void ExpandedAlarmViewHolder::onBindItemView(ItemHolder& itemHolder) {
    AlarmItemViewHolder::onBindItemView(itemHolder);
    AlarmItemHolder* holder = dynamic_cast<AlarmItemHolder*>(&itemHolder);
    const data::Alarm& alarm = holder->item;
    Context& context = *this->itemView->getContext();
    bindEditLabel(context, alarm);
    bindDaysOfWeekButtons(alarm, context);
    bindVibrator(alarm);
    bindRingtone(context, alarm);
    bindPreemptiveDismissButton(context, alarm,
            holder->hasAlarmInstance ? &holder->alarmInstance : nullptr);
}

void ExpandedAlarmViewHolder::bindRingtone(Context& context, const data::Alarm& alarm) {
    // Ringtone titles load with the ringtone module (#11); show the uri stem.
    std::string title = alarm.alert;
    const size_t slash = title.rfind('/');
    if (slash != std::string::npos) title = title.substr(slash + 1);
    if (title.empty()) title = context.getString(R::string::silent_ringtone_title);
    ringtone->setText(title);
    ringtone->setContentDescription(context.getString(R::string::ringtone_description)
            + " " + title);
}

void ExpandedAlarmViewHolder::bindDaysOfWeekButtons(const data::Alarm& alarm, Context& context) {
    const std::vector<int>& weekdays =
            data::Weekdays::Order::calendarDays(data::DataModel::getDataModel().getWeekdayOrder());
    for (size_t i = 0; i < weekdays.size() && i < 7; i++) {
        CompoundButton* dayButton = dayButtons[i];
        if (alarm.getWeekdays().isBitOn(weekdays[i])) {
            dayButton->setChecked(true);
            // windowBackground color for the checked day (ThemeUtils resolveColor).
            int windowBackground = 0xFF212121;
            TypedValue value;
            if (context.getTheme().resolveAttribute(0x01010001 /* windowBackground */,
                    &value, true)) {
                windowBackground = value.data;
            }
            dayButton->setTextColor(windowBackground);
        } else {
            dayButton->setChecked(false);
            dayButton->setTextColor(0xFFFFFFFF);
        }
    }
    if (alarm.getWeekdays().isRepeating()) {
        repeat->setChecked(true);
        repeatDays->setVisibility(View::VISIBLE);
    } else {
        repeat->setChecked(false);
        repeatDays->setVisibility(View::GONE);
    }
}

void ExpandedAlarmViewHolder::bindEditLabel(Context& context, const data::Alarm& alarm) {
    editLabel->setText(alarm.label);
    editLabel->setContentDescription(!alarm.label.empty()
            ? context.getString(R::string::label_description) + " " + alarm.label
            : context.getString(R::string::no_label_specified));
}

void ExpandedAlarmViewHolder::bindVibrator(const data::Alarm& alarm) {
    // cdroid targets have no vibrator service (upstream hides the row); keep the
    // row visible-but-set to honor the layout.
    vibrateCheckBox->setChecked(alarm.vibrate);
}

ItemViewHolder* ExpandedAlarmViewHolder::createViewHolder(LayoutInflater* inflater,
                                                          ViewGroup* parent, int viewType) {
    (void) viewType;
    return new ExpandedAlarmViewHolder(inflater->inflate(
            R::layout::alarm_time_expanded, parent, false));
}

} // namespace alarms
} // namespace deskclock
} // namespace cdroid
