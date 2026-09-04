#include <collapsedalarmviewholder.h>

#include <R.h>

#include <animation/animatorset.h>
#include <animation/objectanimator.h>
#include <view/layoutinflater.h>
#include <view/view.h>
#include <core/context.h>
#include <core/calendar.h>

#include <alarmitemholder.h>
#include <datamodel.h>
#include <expandedalarmviewholder.h>
#include <utils.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace alarms {

Animator* CollapsedAlarmViewHolder::onAnimateChange(RecyclerView::ViewHolder& oldHolder,
        RecyclerView::ViewHolder& newHolder, int64_t duration) {
    // Upstream CollapsedAlarmItemHolder: only the collapse FROM the expanded
    // editor animates (staggered cross-fade); other rebinds stay unanimated.
    if (dynamic_cast<ExpandedAlarmViewHolder*>(&oldHolder) == nullptr) return nullptr;
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

Animator* CollapsedAlarmViewHolder::onAnimateChange(std::vector<Object*>* /*payloads*/,
        int /*fromLeft*/, int /*fromTop*/, int /*fromRight*/, int /*fromBottom*/,
        int64_t /*duration*/) {
    // In-place payload animations (repeat-days) arrive with the payload pass.
    return nullptr;
}

CollapsedAlarmViewHolder::CollapsedAlarmViewHolder(View* itemView)
    : AlarmItemViewHolder(itemView) {
    alarmLabel = (TextView*) itemView->findViewById(R::id::label);
    daysOfWeekView = (TextView*) itemView->findViewById(R::id::days_of_week);
    upcomingInstanceLabel = (TextView*) itemView->findViewById(R::id::upcoming_instance_label);
    hairLine = itemView->findViewById(R::id::hairline);

    // Expand handler.
    itemView->setOnClickListener([this](View&) {
        AlarmItemHolder* holder = dynamic_cast<AlarmItemHolder*>(itemHolder);
        if (holder != nullptr) holder->expand();
    });
    alarmLabel->setOnClickListener([this](View&) {
        AlarmItemHolder* holder = dynamic_cast<AlarmItemHolder*>(itemHolder);
        if (holder != nullptr) holder->expand();
    });
    arrow->setOnClickListener([this](View&) {
        AlarmItemHolder* holder = dynamic_cast<AlarmItemHolder*>(itemHolder);
        if (holder != nullptr) holder->expand();
    });
    // Edit time handler.
    clock->setOnClickListener([this](View&) {
        AlarmItemHolder* holder = dynamic_cast<AlarmItemHolder*>(itemHolder);
        if (holder != nullptr && holder->alarmTimeClickHandler != nullptr) {
            holder->alarmTimeClickHandler->onClockClicked(holder->item);
            holder->expand();
        }
    });

    itemView->setImportantForAccessibility(View::IMPORTANT_FOR_ACCESSIBILITY_NO);
}

void CollapsedAlarmViewHolder::onBindItemView(ItemHolder& itemHolder) {
    AlarmItemViewHolder::onBindItemView(itemHolder);
    AlarmItemHolder* holder = dynamic_cast<AlarmItemHolder*>(&itemHolder);
    const data::Alarm& alarm = holder->item;
    Context& context = *itemView->getContext();
    bindRepeatText(context, alarm);
    bindReadOnlyLabel(context, alarm);
    bindUpcomingInstance(context, alarm);
    bindPreemptiveDismissButton(context, alarm,
            holder->hasAlarmInstance ? &holder->alarmInstance : nullptr);
}

void CollapsedAlarmViewHolder::bindReadOnlyLabel(Context& context, const data::Alarm& alarm) {
    if (!alarm.label.empty()) {
        alarmLabel->setText(alarm.label);
        alarmLabel->setVisibility(View::VISIBLE);
        alarmLabel->setContentDescription(
                context.getString(R::string::label_description) + " " + alarm.label);
    } else {
        alarmLabel->setVisibility(View::GONE);
    }
}

void CollapsedAlarmViewHolder::bindRepeatText(Context& context, const data::Alarm& alarm) {
    if (alarm.getWeekdays().isRepeating()) {
        const data::Weekdays::Order::Value weekdayOrder =
                data::DataModel::getDataModel().getWeekdayOrder();
        const std::string daysOfWeekText = alarm.getWeekdays().toString(context, weekdayOrder);
        daysOfWeekView->setText(daysOfWeekText);
        daysOfWeekView->setVisibility(View::VISIBLE);
    } else {
        daysOfWeekView->setVisibility(View::GONE);
    }
}

void CollapsedAlarmViewHolder::bindUpcomingInstance(Context& context, const data::Alarm& alarm) {
    if (alarm.getWeekdays().isRepeating()) {
        upcomingInstanceLabel->setVisibility(View::GONE);
    } else {
        upcomingInstanceLabel->setVisibility(View::VISIBLE);
        auto now = Calendar::getInstance();
        const std::string labelText = data::Alarm::isTomorrow(alarm, *now)
                ? context.getString(R::string::alarm_tomorrow)
                : context.getString(R::string::alarm_today);
        upcomingInstanceLabel->setText(labelText);
    }
}

ItemViewHolder* CollapsedAlarmViewHolder::createViewHolder(LayoutInflater* inflater,
                                                           ViewGroup* parent, int viewType) {
    (void) viewType;
    return new CollapsedAlarmViewHolder(inflater->inflate(
            R::layout::alarm_time_collapsed, parent, false));
}

} // namespace alarms
} // namespace deskclock
} // namespace cdroid
