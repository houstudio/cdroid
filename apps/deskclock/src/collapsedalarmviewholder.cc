#include <collapsedalarmviewholder.h>

#include <R.h>

#include <animation/animatorset.h>
#include <animation/objectanimator.h>
#include <view/layoutinflater.h>
#include <view/view.h>
#include <view/viewgroup.h>
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
    AlarmItemViewHolder* oldItemHolder = dynamic_cast<AlarmItemViewHolder*>(&oldHolder);
    AlarmItemViewHolder* newItemHolder = dynamic_cast<AlarmItemViewHolder*>(&newHolder);
    if (oldItemHolder == nullptr || newItemHolder == nullptr) {
        return nullptr;
    }

    const bool isCollapsing = this == newItemHolder;
    setChangingViewsAlpha(isCollapsing ? 0.0f : 1.0f);

    Animator* changeAnimatorSet = isCollapsing
            ? createCollapsingAnimator(*oldItemHolder, duration)
            : createExpandingAnimator(*newItemHolder, duration);
    Animator::AnimatorListener listener;
    listener.onAnimationEnd = [this](Animator& animator, bool) {
        animator.removeAllListeners();
        clock->setVisibility(View::VISIBLE);
        onOff->setVisibility(View::VISIBLE);
        arrow->setVisibility(View::VISIBLE);
        arrow->setTranslationY(0.0f);
        setChangingViewsAlpha(1.0f);
        arrow->jumpDrawablesToCurrentState();
    };
    changeAnimatorSet->addListener(listener);
    return changeAnimatorSet;
}

Animator* CollapsedAlarmViewHolder::onAnimateChange(std::vector<Object*>* /*payloads*/,
        int /*fromLeft*/, int /*fromTop*/, int /*fromRight*/, int /*fromBottom*/,
        int64_t /*duration*/) {
    /* There are no possible partial animations for collapsed view holders. */
    return nullptr;
}

Animator* CollapsedAlarmViewHolder::createExpandingAnimator(AlarmItemViewHolder& newHolder,
        int64_t duration) {
    clock->setVisibility(View::INVISIBLE);
    onOff->setVisibility(View::INVISIBLE);
    arrow->setVisibility(View::INVISIBLE);

    // (The cdroid layout subset has no preemptive-dismiss button.)
    std::vector<Animator*> alphaAnimators = {
        ObjectAnimator::ofFloat(alarmLabel, View::ALPHA, {0.0f}),
        ObjectAnimator::ofFloat(daysOfWeekView, View::ALPHA, {0.0f}),
        ObjectAnimator::ofFloat(upcomingInstanceLabel, View::ALPHA, {0.0f}),
        ObjectAnimator::ofFloat(hairLine, View::ALPHA, {0.0f}),
    };
    auto* alphaAnimatorSet = new AnimatorSet();
    alphaAnimatorSet->playTogether(alphaAnimators);
    alphaAnimatorSet->setDuration(
            (int64_t) (duration * ANIM_SHORT_DURATION_MULTIPLIER));

    View* oldView = itemView;
    View* newView = newHolder.itemView;
    Animator* boundsAnimator = AnimatorUtils::getBoundsAnimator(*oldView, *oldView, *newView);
    boundsAnimator->setDuration(duration);
    boundsAnimator->setInterpolator(AnimatorUtils::INTERPOLATOR_FAST_OUT_SLOW_IN());

    std::vector<Animator*> animators = {alphaAnimatorSet, boundsAnimator};
    auto* animatorSet = new AnimatorSet();
    animatorSet->playTogether(animators);
    return animatorSet;
}

Animator* CollapsedAlarmViewHolder::createCollapsingAnimator(AlarmItemViewHolder& oldHolder,
        int64_t duration) {
    std::vector<Animator*> alphaAnimators = {
        ObjectAnimator::ofFloat(alarmLabel, View::ALPHA, {1.0f}),
        ObjectAnimator::ofFloat(daysOfWeekView, View::ALPHA, {1.0f}),
        ObjectAnimator::ofFloat(upcomingInstanceLabel, View::ALPHA, {1.0f}),
        ObjectAnimator::ofFloat(hairLine, View::ALPHA, {1.0f}),
    };
    auto* alphaAnimatorSet = new AnimatorSet();
    alphaAnimatorSet->playTogether(alphaAnimators);
    const int64_t standardDelay = (int64_t) (duration * ANIM_STANDARD_DELAY_MULTIPLIER);
    alphaAnimatorSet->setDuration(standardDelay);
    alphaAnimatorSet->setStartDelay(duration - standardDelay);

    View* oldView = oldHolder.itemView;
    View* newView = itemView;
    Animator* boundsAnimator = AnimatorUtils::getBoundsAnimator(*newView, *oldView, *newView);
    boundsAnimator->setDuration(duration);
    boundsAnimator->setInterpolator(AnimatorUtils::INTERPOLATOR_FAST_OUT_SLOW_IN());

    // The arrow rides from its old editor position down to the collapsed row.
    View* oldArrow = oldHolder.arrow;
    Rect oldArrowRect;   // cdroid Rect is l/t/w/h
    oldArrowRect.set(0, 0, oldArrow->getWidth(), oldArrow->getHeight());
    Rect newArrowRect;
    newArrowRect.set(0, 0, arrow->getWidth(), arrow->getHeight());
    ((ViewGroup*) newView)->offsetDescendantRectToMyCoords(arrow, newArrowRect);
    ((ViewGroup*) oldView)->offsetDescendantRectToMyCoords(oldArrow, oldArrowRect);
    const float arrowTranslationY =
            (float) (oldArrowRect.bottom() - newArrowRect.bottom());
    arrow->setTranslationY(arrowTranslationY);
    arrow->setVisibility(View::VISIBLE);
    clock->setVisibility(View::VISIBLE);
    onOff->setVisibility(View::VISIBLE);

    Animator* arrowAnimation =
            ObjectAnimator::ofFloat(arrow, View::TRANSLATION_Y, {0.0f});
    arrowAnimation->setDuration(duration);
    arrowAnimation->setInterpolator(AnimatorUtils::INTERPOLATOR_FAST_OUT_SLOW_IN());

    std::vector<Animator*> animators = {alphaAnimatorSet, boundsAnimator, arrowAnimation};
    auto* animatorSet = new AnimatorSet();
    animatorSet->playTogether(animators);
    Animator::AnimatorListener listener;
    listener.onAnimationStart = [this](Animator&, bool) {
        AnimatorUtils::startDrawableAnimation(*arrow);
    };
    animatorSet->addListener(listener);
    return animatorSet;
}

void CollapsedAlarmViewHolder::setChangingViewsAlpha(float alpha) {
    alarmLabel->setAlpha(alpha);
    daysOfWeekView->setAlpha(alpha);
    upcomingInstanceLabel->setAlpha(alpha);
    hairLine->setAlpha(alpha);
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
