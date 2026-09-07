#include <analogclock.h>

#include <R.h>

#include <content/Locale.h>
#include <content/dateformat.h>
#include <content/simpledateformat.h>
#include <core/systemclock.h>
#include <view/layoutinflater.h>
#include <view/view.h>
#include <drawable/colordrawable.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {

namespace {
constexpr int64_t SECOND_IN_MILLIS = 1000LL;
}

AnalogClock::AnalogClock(Context* context, const AttributeSet* attrs)
    : FrameLayout(context, attrs) {
    mTime = Calendar::getInstance();
    // (DateFormat.getTimeFormat(ctx) as SimpleDateFormat).toLocalizedPattern()
    mDescFormat = DateFormat::getBestDateTimePattern(Locale::getDefault(), "hma");

    mClockTick = [this]() {
        onTimeChanged();

        if (mEnableSeconds) {
            const int64_t now = SystemClock::currentTimeMillis();
            const int64_t delay = SECOND_IN_MILLIS - now % SECOND_IN_MILLIS;
            postDelayed(mClockTick, delay);
        }
    };

    // Must call mutate on these instances, otherwise the drawables will blur, because they're
    // sharing their size characteristics with the (smaller) world cities analog clocks.
    ImageView* dial = new ImageView(getContext());
    dial->setImageResource(R::drawable::clock_analog_dial);
    if (Drawable* d = dial->getDrawable()) d->mutate();
    addView(dial);

    mHourHand = new ImageView(getContext());
    mHourHand->setImageResource(R::drawable::clock_analog_hour);
    if (Drawable* d = mHourHand->getDrawable()) d->mutate();
    addView(mHourHand);

    mMinuteHand = new ImageView(getContext());
    mMinuteHand->setImageResource(R::drawable::clock_analog_minute);
    if (Drawable* d = mMinuteHand->getDrawable()) d->mutate();
    addView(mMinuteHand);

    mSecondHand = new ImageView(getContext());
    mSecondHand->setImageResource(R::drawable::clock_analog_second);
    if (Drawable* d = mSecondHand->getDrawable()) d->mutate();
    addView(mSecondHand);

    // The dial vector is a transparent ring upstream as well; AOSP shows the
    // (dark) window background through its center. cdroid windows carry no
    // background, so the moving hands smear over stale pixels instead — give
    // the clock an opaque plate.
    setBackgroundColor(0xFF000000);
}

void AnalogClock::onAttachedToWindow() {
    FrameLayout::onAttachedToWindow();

    // Refresh the calendar instance since the time zone may have changed while detached.
    mTime = Calendar::getInstance();
    mTimeZone = TimeZone::getDefault();
    mTime->setTimeZone(mTimeZone.getRawOffset() / 1000);
    onTimeChanged();

    // Tick every second.
    if (mEnableSeconds) {
        mClockTick();
    }
}

void AnalogClock::onDetachedFromWindow() {
    FrameLayout::onDetachedFromWindow();

    removeCallbacks(mClockTick);
}

void AnalogClock::onTimeChanged() {
    mTime->setTimeInMillis(SystemClock::currentTimeMillis());
    const float hourAngle = mTime->get(Calendar::HOUR) * 30.0f;
    mHourHand->setRotation(hourAngle);
    const float minuteAngle = mTime->get(Calendar::MINUTE) * 6.0f;
    mMinuteHand->setRotation(minuteAngle);
    if (mEnableSeconds) {
        const float secondAngle = mTime->get(Calendar::SECOND) * 6.0f;
        mSecondHand->setRotation(secondAngle);
    }
    SimpleDateFormat descFmt(mDescFormat, Locale::getDefault());
    setContentDescription(descFmt.format(mTime->getTimeInMillis()));
    invalidate();
}

void AnalogClock::setTimeZone(const std::string& id) {
    mTimeZone = TimeZone::getTimeZone(id);
    mTime->setTimeZone(mTimeZone.getRawOffset() / 1000);
    onTimeChanged();
}

void AnalogClock::enableSeconds(bool enable) {
    mEnableSeconds = enable;
    if (mEnableSeconds) {
        mSecondHand->setVisibility(View::VISIBLE);
        mClockTick();
    } else {
        mSecondHand->setVisibility(View::GONE);
    }
}

} // namespace deskclock

// Registered under the legacy invented key "DeskClockAnalogClock" and under
// the upstream FQCN: getInflater resolves a dotted tag exactly first, so the
// original tag now reaches this app class without colliding with the
// framework android.widget.AnalogClock (bare "AnalogClock") registration.
static const int _cdroid_act_reg_deskclock_AnalogClock =
    (LayoutInflater::registerInflater("DeskClockAnalogClock",
        [](Context* ctx, const AttributeSet& attr) -> View* {
            return new cdroid::deskclock::AnalogClock(ctx, &attr);
        }),
     LayoutInflater::registerInflater("com.android.deskclock.AnalogClock",
        [](Context* ctx, const AttributeSet& attr) -> View* {
            return new cdroid::deskclock::AnalogClock(ctx, &attr);
        }), 0);

} // namespace cdroid
