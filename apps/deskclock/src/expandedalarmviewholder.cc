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

Animator* ExpandedAlarmViewHolder::onAnimateChange(std::vector<Object*>* payloads,
        int fromLeft, int fromTop, int fromRight, int fromBottom, int64_t duration) {
    if (payloads == nullptr || payloads->empty()) {
        return nullptr;
    }
    bool hasRepeatDaysPayload = false;
    for (Object* payload : *payloads) {
        if (payload == AlarmItemHolder::ANIMATE_REPEAT_DAYS()) {
            hasRepeatDaysPayload = true;
            break;
        }
    }
    if (!hasRepeatDaysPayload) {
        return nullptr;
    }

    // The repeat-days row grows/shrinks in place; everything below it rides.
    const bool isExpansion = repeatDays->getVisibility() == View::VISIBLE;
    const int height = repeatDays->getHeight();
    if (isExpansion) {
        setTranslationY((float) -height, (float) -height);
    } else {
        setTranslationY(0.0f, (float) height);
    }
    repeatDays->setVisibility(View::VISIBLE);
    repeatDays->setAlpha(isExpansion ? 0.0f : 1.0f);

    std::vector<Animator*> animators = {
        AnimatorUtils::getBoundsAnimator(*itemView, fromLeft, fromTop, fromRight, fromBottom,
                itemView->getLeft(), itemView->getTop(), itemView->getRight(),
                itemView->getBottom()),
        ObjectAnimator::ofFloat(repeatDays, View::ALPHA, {isExpansion ? 1.0f : 0.0f}),
        ObjectAnimator::ofFloat(repeatDays, View::TRANSLATION_Y,
                {isExpansion ? 0.0f : (float) -height}),
        ObjectAnimator::ofFloat(ringtone, View::TRANSLATION_Y, {0.0f}),
        ObjectAnimator::ofFloat(vibrateCheckBox, View::TRANSLATION_Y, {0.0f}),
        ObjectAnimator::ofFloat(editLabel, View::TRANSLATION_Y, {0.0f}),
        ObjectAnimator::ofFloat(hairLine, View::TRANSLATION_Y, {0.0f}),
        ObjectAnimator::ofFloat(deleteView, View::TRANSLATION_Y, {0.0f}),
        ObjectAnimator::ofFloat(arrow, View::TRANSLATION_Y, {0.0f}),
    };
    auto* animatorSet = new AnimatorSet();
    animatorSet->playTogether(animators);
    Animator::AnimatorListener listener;
    listener.onAnimationEnd = [this, isExpansion](Animator&, bool) {
        setTranslationY(0.0f, 0.0f);
        repeatDays->setAlpha(1.0f);
        repeatDays->setVisibility(isExpansion ? View::VISIBLE : View::GONE);
        itemView->requestLayout();
    };
    animatorSet->addListener(listener);
    animatorSet->setDuration(duration);
    animatorSet->setInterpolator(AnimatorUtils::INTERPOLATOR_FAST_OUT_SLOW_IN());

    return animatorSet;
}

Animator* ExpandedAlarmViewHolder::onAnimateChange(RecyclerView::ViewHolder& oldHolder,
        RecyclerView::ViewHolder& newHolder, int64_t duration) {
    AlarmItemViewHolder* oldItemHolder = dynamic_cast<AlarmItemViewHolder*>(&oldHolder);
    AlarmItemViewHolder* newItemHolder = dynamic_cast<AlarmItemViewHolder*>(&newHolder);
    if (oldItemHolder == nullptr || newItemHolder == nullptr) {
        return nullptr;
    }

    const bool isExpanding = this == newItemHolder;
    AnimatorUtils::setBackgroundAlpha(*itemView, isExpanding ? 0 : 255);
    setChangingViewsAlpha(isExpanding ? 0.0f : 1.0f);

    Animator* changeAnimatorSet = isExpanding
            ? createExpandingAnimator(*oldItemHolder, duration)
            : createCollapsingAnimator(*newItemHolder, duration);
    Animator::AnimatorListener listener;
    listener.onAnimationEnd = [this](Animator& animator, bool) {
        animator.removeAllListeners();
        AnimatorUtils::setBackgroundAlpha(*itemView, 255);
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

Animator* ExpandedAlarmViewHolder::createCollapsingAnimator(AlarmItemViewHolder& newHolder,
        int64_t duration) {
    arrow->setVisibility(View::INVISIBLE);
    clock->setVisibility(View::INVISIBLE);
    onOff->setVisibility(View::INVISIBLE);

    const bool daysVisible = repeatDays->getVisibility() == View::VISIBLE;
    const int numberOfItems = countNumberOfItems();

    View* oldView = itemView;
    View* newView = newHolder.itemView;

    // The editor's background fades away while its bounds shrink to the row.
    std::vector<PropertyValuesHolder*> bgProps = {
        PropertyValuesHolder::ofInt(AnimatorUtils::BACKGROUND_ALPHA(), {255, 0}),
    };
    Animator* backgroundAnimator = ObjectAnimator::ofPropertyValuesHolder(oldView, bgProps);
    backgroundAnimator->setDuration(duration);

    Animator* boundsAnimator = AnimatorUtils::getBoundsAnimator(*oldView, *oldView, *newView);
    boundsAnimator->setDuration(duration);
    boundsAnimator->setInterpolator(AnimatorUtils::INTERPOLATOR_FAST_OUT_SLOW_IN());

    // Editor contents each fade out on a staggered delay.
    const int64_t shortDuration = (int64_t) (duration * ANIM_SHORT_DURATION_MULTIPLIER);
    auto* repeatAnimation = ObjectAnimator::ofFloat(repeat, View::ALPHA, {0.0f});
    repeatAnimation->setDuration(shortDuration);
    auto* editLabelAnimation = ObjectAnimator::ofFloat(editLabel, View::ALPHA, {0.0f});
    editLabelAnimation->setDuration(shortDuration);
    auto* repeatDaysAnimation = ObjectAnimator::ofFloat(repeatDays, View::ALPHA, {0.0f});
    repeatDaysAnimation->setDuration(shortDuration);
    auto* vibrateAnimation = ObjectAnimator::ofFloat(vibrateCheckBox, View::ALPHA, {0.0f});
    vibrateAnimation->setDuration(shortDuration);
    auto* ringtoneAnimation = ObjectAnimator::ofFloat(ringtone, View::ALPHA, {0.0f});
    ringtoneAnimation->setDuration(shortDuration);
    auto* deleteAnimation = ObjectAnimator::ofFloat(deleteView, View::ALPHA, {0.0f});
    deleteAnimation->setDuration(shortDuration);
    auto* hairLineAnimation = ObjectAnimator::ofFloat(hairLine, View::ALPHA, {0.0f});
    hairLineAnimation->setDuration(shortDuration);
    // (No preemptive-dismiss button in the cdroid layout subset.)

    // Set the staggered delays; use the first portion (duration * (1 - 1/4 - 1/6))
    // of the time, so the final animation, with a duration of 1/4 the total
    // duration, finishes exactly before the collapsed holder begins expanding.
    int64_t startDelay = 0;
    const int64_t delayIncrement =
            (int64_t) (duration * ANIM_LONG_DELAY_INCREMENT_MULTIPLIER) / (numberOfItems - 1);
    deleteAnimation->setStartDelay(startDelay);
    hairLineAnimation->setStartDelay(startDelay);
    startDelay += delayIncrement;
    editLabelAnimation->setStartDelay(startDelay);
    startDelay += delayIncrement;
    vibrateAnimation->setStartDelay(startDelay);
    ringtoneAnimation->setStartDelay(startDelay);
    startDelay += delayIncrement;
    if (daysVisible) {
        repeatDaysAnimation->setStartDelay(startDelay);
        startDelay += delayIncrement;
    }
    repeatAnimation->setStartDelay(startDelay);

    std::vector<Animator*> animators = {backgroundAnimator, boundsAnimator, repeatAnimation,
            repeatDaysAnimation, vibrateAnimation, ringtoneAnimation, editLabelAnimation,
            deleteAnimation, hairLineAnimation};
    auto* animatorSet = new AnimatorSet();
    animatorSet->playTogether(animators);
    return animatorSet;
}

Animator* ExpandedAlarmViewHolder::createExpandingAnimator(AlarmItemViewHolder& oldHolder,
        int64_t duration) {
    View* oldView = oldHolder.itemView;
    View* newView = itemView;
    Animator* boundsAnimator = AnimatorUtils::getBoundsAnimator(*newView, *oldView, *newView);
    boundsAnimator->setDuration(duration);
    boundsAnimator->setInterpolator(AnimatorUtils::INTERPOLATOR_FAST_OUT_SLOW_IN());

    std::vector<PropertyValuesHolder*> bgProps = {
        PropertyValuesHolder::ofInt(AnimatorUtils::BACKGROUND_ALPHA(), {0, 255}),
    };
    Animator* backgroundAnimator = ObjectAnimator::ofPropertyValuesHolder(newView, bgProps);
    backgroundAnimator->setDuration(duration);

    // The arrow rides from its old collapsed position up into the editor.
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

    const int64_t longDuration = (int64_t) (duration * ANIM_LONG_DURATION_MULTIPLIER);
    auto* repeatAnimation = ObjectAnimator::ofFloat(repeat, View::ALPHA, {1.0f});
    repeatAnimation->setDuration(longDuration);
    auto* repeatDaysAnimation = ObjectAnimator::ofFloat(repeatDays, View::ALPHA, {1.0f});
    repeatDaysAnimation->setDuration(longDuration);
    auto* ringtoneAnimation = ObjectAnimator::ofFloat(ringtone, View::ALPHA, {1.0f});
    ringtoneAnimation->setDuration(longDuration);
    auto* vibrateAnimation = ObjectAnimator::ofFloat(vibrateCheckBox, View::ALPHA, {1.0f});
    vibrateAnimation->setDuration(longDuration);
    auto* editLabelAnimation = ObjectAnimator::ofFloat(editLabel, View::ALPHA, {1.0f});
    editLabelAnimation->setDuration(longDuration);
    auto* hairLineAnimation = ObjectAnimator::ofFloat(hairLine, View::ALPHA, {1.0f});
    hairLineAnimation->setDuration(longDuration);
    auto* deleteAnimation = ObjectAnimator::ofFloat(deleteView, View::ALPHA, {1.0f});
    deleteAnimation->setDuration(longDuration);
    // (No preemptive-dismiss button in the cdroid layout subset.)
    auto* arrowAnimation = ObjectAnimator::ofFloat(arrow, View::TRANSLATION_Y, {0.0f});
    arrowAnimation->setDuration(duration);
    arrowAnimation->setInterpolator(AnimatorUtils::INTERPOLATOR_FAST_OUT_SLOW_IN());

    // Stagger the delays: delay the first by the amount of time it takes for
    // the collapse to complete, then stagger the expansion with the rest.
    int64_t startDelay = (int64_t) (duration * ANIM_STANDARD_DELAY_MULTIPLIER);
    const int numberOfItems = countNumberOfItems();
    const int64_t delayIncrement =
            (int64_t) (duration * ANIM_SHORT_DELAY_INCREMENT_MULTIPLIER) / (numberOfItems - 1);
    repeatAnimation->setStartDelay(startDelay);
    startDelay += delayIncrement;
    const bool daysVisible = repeatDays->getVisibility() == View::VISIBLE;
    if (daysVisible) {
        repeatDaysAnimation->setStartDelay(startDelay);
        startDelay += delayIncrement;
    }
    ringtoneAnimation->setStartDelay(startDelay);
    vibrateAnimation->setStartDelay(startDelay);
    startDelay += delayIncrement;
    editLabelAnimation->setStartDelay(startDelay);
    startDelay += delayIncrement;
    hairLineAnimation->setStartDelay(startDelay);
    deleteAnimation->setStartDelay(startDelay);

    std::vector<Animator*> animators = {boundsAnimator, backgroundAnimator, arrowAnimation,
            repeatAnimation, repeatDaysAnimation, ringtoneAnimation, vibrateAnimation,
            editLabelAnimation, hairLineAnimation, deleteAnimation};
    auto* animatorSet = new AnimatorSet();
    animatorSet->playTogether(animators);
    Animator::AnimatorListener listener;
    listener.onAnimationStart = [this](Animator&, bool) {
        AnimatorUtils::startDrawableAnimation(*arrow);
    };
    animatorSet->addListener(listener);
    return animatorSet;
}

void ExpandedAlarmViewHolder::setTranslationY(float repeatDaysTranslationY, float translationY) {
    repeatDays->setTranslationY(repeatDaysTranslationY);
    ringtone->setTranslationY(translationY);
    vibrateCheckBox->setTranslationY(translationY);
    editLabel->setTranslationY(translationY);
    hairLine->setTranslationY(translationY);
    deleteView->setTranslationY(translationY);
    arrow->setTranslationY(translationY);
}

void ExpandedAlarmViewHolder::setChangingViewsAlpha(float alpha) {
    repeat->setAlpha(alpha);
    editLabel->setAlpha(alpha);
    repeatDays->setAlpha(alpha);
    vibrateCheckBox->setAlpha(alpha);
    ringtone->setAlpha(alpha);
    hairLine->setAlpha(alpha);
    deleteView->setAlpha(alpha);
}

int ExpandedAlarmViewHolder::countNumberOfItems() const {
    // Always between 4 and 6 items (no preemptive-dismiss button: 4..5 here).
    int numberOfItems = 4;
    if (repeatDays->getVisibility() == View::VISIBLE) {
        numberOfItems++;
    }
    return numberOfItems;
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
            holder->notifyItemChanged(AlarmItemHolder::ANIMATE_REPEAT_DAYS());
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
    // DataModel.getRingtoneTitle(alarm.alert) (the uri stem placeholder retired
    // with the ringtone module).
    const std::string title =
            data::DataModel::getDataModel().getRingtoneTitle(alarm.alert);
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
