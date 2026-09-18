#ifndef __DESKCLOCK_ANALOGCLOCK_H__
#define __DESKCLOCK_ANALOGCLOCK_H__
/*********************************************************************************
 * Port of com.android.deskclock.AnalogClock — a FrameLayout of four ImageViews
 * (dial + hour/minute/second hands) rotated to the current time, ticking every
 * second. CDROID has no system TIME_TICK/TIMEZONE_CHANGED broadcasts: the
 * ticker Runnable (posted each second) drives onTimeChanged; zone changes
 * re-resolve on attach or setTimeZone.
 *
 * Registered as "DeskClockAnalogClock": cdroid's inflater truncates dotted
 * tags to the last segment, colliding with the framework AnalogClock.
 *********************************************************************************/
#include <memory>

#include <core/calendar.h>
#include <widget/framelayout.h>
#include <widget/imageview.h>

#include <timezone.h>

namespace cdroid {
namespace deskclock {

class AnalogClock : public FrameLayout {
private:
    Runnable mClockTick;

    ImageView* mHourHand;
    ImageView* mMinuteHand;
    ImageView* mSecondHand;

    std::unique_ptr<Calendar> mTime;
    std::string mDescFormat;
    TimeZone mTimeZone = TimeZone::getDefault();
    bool mEnableSeconds = true;

public:
    AnalogClock(Context* context, const AttributeSet* attrs);

    void onAttachedToWindow() override;
    void onDetachedFromWindow() override;

    void setTimeZone(const std::string& id);
    void enableSeconds(bool enable);

private:
    void onTimeChanged();
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_ANALOGCLOCK_H__
