#ifndef __DESKCLOCK_TEXTTIME_H__
#define __DESKCLOCK_TEXTTIME_H__
/*********************************************************************************
 * Port of com.android.deskclock.widget.TextTime — a TextView displaying a
 * constant (non-ticking) time of day. The ContentObserver on the system
 * time-format setting is not ported (no system settings surface); format is
 * chosen on (re)attach and format changes.
 *********************************************************************************/
#include <string>

#include <widget/textview.h>

namespace cdroid {
namespace deskclock {

class TextTime : public TextView {
private:
    std::string mFormat12;
    std::string mFormat24;
    std::string mFormat;

    bool mAttached = false;

    int mHour = 0;
    int mMinute = 0;

public:
    TextTime(Context* context, const AttributeSet* attrs);

    std::string getFormat12Hour() const { return mFormat12; }
    void setFormat12Hour(const std::string& format);
    std::string getFormat24Hour() const { return mFormat24; }
    void setFormat24Hour(const std::string& format);

    void onAttachedToWindow() override;
    void onDetachedFromWindow() override;

    void setTime(int hour, int minute);

    static const constexpr char* DEFAULT_FORMAT_12_HOUR = "h:mm a";
    static const constexpr char* DEFAULT_FORMAT_24_HOUR = "H:mm";

private:
    void chooseFormat();
    void updateTime();
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_TEXTTIME_H__
