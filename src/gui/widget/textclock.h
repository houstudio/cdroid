/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __TEXT_CLOCK_H__
#define __TEXT_CLOCK_H__
#include <core/calendar.h>
#include <widget/textview.h>
namespace cdroid{
class TextClock:public TextView {
public:
    static constexpr const char* const DEFAULT_FORMAT_12_HOUR = "h:mm a";
    static constexpr const char* const DEFAULT_FORMAT_24_HOUR = "H:mm";
private:
    class ClockEventDelegate;
    std::string mFormat12;
    std::string mFormat24;
    std::string mDescFormat12;
    std::string mDescFormat24;
    std::string mFormat;
    std::string mDescFormat;
    std::string mTimeZone;

    bool mHasSeconds;
    bool mRegistered;
    bool mShouldRunTicker;
    bool mShowCurrentUserTime;
    // Used by tests to stop time change events from triggering the text update
    bool mStopTicking;

    Calendar mTime;
    ClockEventDelegate* mClockEventDelegate;
    //ContentObserver mFormatChangeObserver;
    //BroadcastReceiver mIntentReceiver;
    Runnable mTicker;
private:
    void init();
    void doTick();
    void createTime(const std::string& timeZone);
    void chooseFormat();
    const std::string getBestDateTimePattern(const std::string& skeleton);
    void registerObserver();
    void unregisterObserver();
    void onTimeChanged();
protected:
    void onAttachedToWindow()override;
    void onDetachedFromWindow()override;
    //void encodeProperties(ViewHierarchyEncoder& stream)override;
public:
    TextClock(Context* context,const AttributeSet& attrs);
    
    std::string getFormat12Hour() const;
    void setFormat12Hour(const std::string& format);
    void setContentDescriptionFormat12Hour(const std::string& format);

    std::string getFormat24Hour() const;
    void setFormat24Hour(const std::string& format);
    void setContentDescriptionFormat24Hour(const std::string& format);

    /**
     * Sets whether this clock should always track the current user and not the user of the
     * current process. This is used for single instance processes like the systemUI who need
     * to display time for different users.
     */
    void setShowCurrentUserTime(bool showCurrentUserTime);
    void setClockEventDelegate(ClockEventDelegate* delegate);

    void refreshTime();
    bool is24HourModeEnabled() const;

    std::string getTimeZone() const;

    /**
     * Sets the specified time zone to use in this clock. When the time zone
     * is set through this method, system time zone changes (when the user
     * sets the time zone in settings for instance) will be ignored.
     *
     * @param timeZone The desired time zone's ID as specified in {@link TimeZone}
     *                 or null to user the time zone specified by the user
     *                 (system time zone)
     *
     * @see #getTimeZone()
     * @see java.util.TimeZone#getAvailableIDs()
     * @see TimeZone#getTimeZone(String)
     *
     * @attr ref android.R.styleable#TextClock_timeZone
     */
    void setTimeZone(const std::string& timeZone);

    /**
     * Returns the current format string. Always valid after constructor has
     * finished, and will never be {@code null}.
     */
    std::string getFormat() const;
    void onVisibilityAggregated(bool isVisible) override;
    void disableClockTick();
};

/*class TextClock::FormatChangeObserver:public ContentObserver {
public:
    FormatChangeObserver(Handler handler);
    void onChange(bool selfChange) override;
    void onChange(bool selfChange, Uri uri) override;
};

class TextClock::ClockEventDelegate {
private:
    Context* mContext;
public:
    ClockEventDelegate(Context* context);
    void registerTimeChangeReceiver(BroadcastReceiver receiver, Handler handler);
    void unregisterTimeChangeReceiver(BroadcastReceiver receiver);
    void registerFormatChangeObserver(ContentObserver observer, int userHandle);
    void unregisterFormatChangeObserver(ContentObserver observer);
};*/
}/*endof namespace*/
#endif/*__TEXT_CLOCK_H__*/
