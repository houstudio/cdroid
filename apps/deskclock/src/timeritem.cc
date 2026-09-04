#include <timeritem.h>

#include <R.h>
#include <widget/internal_R.h>

#include <drawable/colorstatelist.h>
#include <core/context.h>
#include <core/systemclock.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {

TimerItem::TimerItem(Context* context, const AttributeSet* attrs)
    : LinearLayout(context, attrs) {
}

TimerItem::~TimerItem() {
    delete mTimerTextController;
}

void TimerItem::onFinishInflate() {
    LinearLayout::onFinishInflate();
    mLabelView = (TextView*) findViewById(R::id::timer_label);
    mResetAddButton = (Button*) findViewById(R::id::reset_add);
    mCircleView = (TimerCircleView*) findViewById(R::id::timer_time);
    mTimerText = (TextView*) findViewById(R::id::timer_time_text);
    mTimerTextController = new TimerTextController(*mTimerText);

    Context& c = *getContext();
    int colorAccent = 0xFFDA4336;
    int textColorPrimary = 0xFFFFFFFF;
    {
        // Same as StopwatchFragment: Theme.resolveAttribute flattens a
        // color-selector reference to a pool value (TypedValue.data with
        // resourceId=0) — as an ARGB int that is effectively transparent, so
        // the countdown digits never rasterize. obtainStyledAttributes is the
        // ResTable path that chases the reference.
        const uint32_t attrs[] = {0x01010435 /* android:colorAccent */,
                                  0x01010036 /* android:textColorPrimary */, 0};
        auto ta = c.obtainStyledAttributes(attrs);
        if (ta != nullptr) {
            colorAccent = (int) ta->getColor(0, (uint32_t) colorAccent);
            textColorPrimary = (int) ta->getColor(1, (uint32_t) textColorPrimary);
        }
    }
    auto colors = std::make_shared<ColorStateList>(
            std::vector<std::vector<int>>{
                    {-internal::R::attr::state_activated, -internal::R::attr::state_pressed}, {}},
            std::vector<int>{textColorPrimary, colorAccent});
    mTimerText->setTextColor(colors);
}

void TimerItem::update(const data::Timer& timer) {
    // Update the time.
    mTimerTextController->setTimeString(timer.getRemainingTime());

    // Update the label if it changed. (Upstream compares against the view's text;
    // cdroid CharSequence::toString() returns an owned String*, so track the last
    // label set instead of reading it back.)
    const std::string& label = timer.label;
    if (label != mLastLabel) {
        mLastLabel = label;
        mLabelView->setText(label);
    }

    // Update visibility of things that may blink.
    const bool blinkOff = SystemClock::elapsedRealtime() % 1000 < 500;
    if (mCircleView != nullptr) {
        const bool hideCircle = (timer.isExpired() || timer.isMissed()) && blinkOff;
        mCircleView->setVisibility(hideCircle ? View::INVISIBLE : View::VISIBLE);

        if (!hideCircle) {
            // Update the progress of the circle.
            mCircleView->update(timer);
        }
    }
    if (!timer.isPaused() || !blinkOff || mTimerText->isPressed()) {
        mTimerText->setAlpha(1.0f);
    } else {
        mTimerText->setAlpha(0.0f);
    }

    // Update some potentially expensive areas of the user interface only on state changes.
    if (timer.state != mLastState) {
        mLastState = timer.state;
        Context& context = *getContext();
        switch (mLastState) {
            case data::Timer::State::RESET:
            case data::Timer::State::PAUSED:
                mResetAddButton->setText(R::string::timer_reset);
                mResetAddButton->setContentDescription(std::string());
                mTimerText->setClickable(true);
                mTimerText->setActivated(false);
                // Upstream attaches ClickAccessibilityDelegate("start timer") here; the
                // a11y delegate face is not ported — the content description carries it.
                break;
            case data::Timer::State::RUNNING: {
                const std::string addTimeDesc = context.getString(R::string::timer_plus_one);
                mResetAddButton->setText(R::string::timer_add_minute);
                mResetAddButton->setContentDescription(addTimeDesc);
                mTimerText->setClickable(true);
                mTimerText->setActivated(false);
                break;
            }
            case data::Timer::State::EXPIRED:
            case data::Timer::State::MISSED: {
                const std::string addTimeDesc = context.getString(R::string::timer_plus_one);
                mResetAddButton->setText(R::string::timer_add_minute);
                mResetAddButton->setContentDescription(addTimeDesc);
                mTimerText->setClickable(false);
                mTimerText->setActivated(true);
                break;
            }
        }
    }
}

} // namespace deskclock

DECLARE_WIDGET3(cdroid::deskclock::TimerItem, TimerItem, 0);

} // namespace cdroid
