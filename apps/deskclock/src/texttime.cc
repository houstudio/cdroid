#include <texttime.h>

#include <content/Locale.h>
#include <content/simpledateformat.h>

#include <datamodel.h>
#include <utils.h>

namespace cdroid {
namespace deskclock {

using data::DataModel;

TextTime::TextTime(Context* context, const AttributeSet* attrs)
    : TextView(context, attrs)
    , mFormat12( /* Utils.get12ModeFormat(0.3f, false) pattern form */
            "h:mm a")
    , mFormat24(Utils::get24ModeFormat(false)) {
    chooseFormat();
}

void TextTime::setFormat12Hour(const std::string& format) {
    mFormat12 = format;
    chooseFormat();
    updateTime();
}

void TextTime::setFormat24Hour(const std::string& format) {
    mFormat24 = format;
    chooseFormat();
    updateTime();
}

void TextTime::chooseFormat() {
    const bool format24Requested = DataModel::getDataModel().is24HourFormat();
    if (format24Requested) {
        mFormat = mFormat24.empty() ? DEFAULT_FORMAT_24_HOUR : mFormat24;
    } else {
        mFormat = mFormat12.empty() ? DEFAULT_FORMAT_12_HOUR : mFormat12;
    }
}

void TextTime::onAttachedToWindow() {
    TextView::onAttachedToWindow();
    if (!mAttached) {
        mAttached = true;
        // Upstream registers a ContentObserver on Settings.System here; cdroid
        // has no system settings surface, so the format only refreshes on attach.
        updateTime();
    }
}

void TextTime::onDetachedFromWindow() {
    TextView::onDetachedFromWindow();
    mAttached = false;
}

void TextTime::setTime(int hour, int minute) {
    mHour = hour;
    mMinute = minute;
    updateTime();
}

void TextTime::updateTime() {
    // Format the time relative to UTC to ensure hour and minute are not adjusted for DST.
    std::unique_ptr<Calendar> calendar = DataModel::getDataModel().getCalendar();
    calendar->setTimeZone(0 /* UTC */);
    calendar->set(Calendar::HOUR_OF_DAY, mHour);
    calendar->set(Calendar::MINUTE, mMinute);
    SimpleDateFormat format(mFormat, Locale::getDefault());
    setText(format.format(calendar->getTimeInMillis()));
    // Strip away the spans from text so talkback is not confused
    setContentDescription(format.format(calendar->getTimeInMillis()));
}

} // namespace deskclock

typedef cdroid::deskclock::TextTime TextTime;
DECLARE_WIDGET2(TextTime, "TextTime");

} // namespace cdroid
