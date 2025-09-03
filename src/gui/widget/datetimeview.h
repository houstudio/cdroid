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
#ifndef __DATE_TIME_VIEW_H__
#define __DATE_TIME_VIEW_H__
#include <widget/textview.h>
namespace cdroid{

class DateTimeView: public TextView{
private:
    static constexpr int SHOW_TIME = 0;
    static constexpr int SHOW_MONTH_DAY_YEAR = 1;
private:
    int64_t mTimeMillis;
    // The LocalDateTime equivalent of mTimeMillis but truncated to minute, i.e. no seconds / nanos.
    //LocalDateTime mLocalTime;

    int mLastDisplay = -1;
    //DateFormat mLastFormat;

    int64_t mUpdateTimeMillis;
    //static final ThreadLocal<ReceiverInfo> sReceiverInfo = new ThreadLocal<ReceiverInfo>();
    std::string mNowText;
    bool mShowRelativeTime;
private:
    void updateRelativeTime();
    //int64_t computeNextMidnight(TimeZone timeZone);
protected:
    void onAttachedToWindow()override;
    void onDetachedFromWindow()override;
    void update();
public:
    DateTimeView(Context* context,const AttributeSet& attrs);
    void setTime(int64_t timeMillis);
    void setShowRelativeTime(bool showRelativeTime);
    View& setVisibility(int visibility)override;
    void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info)override;
};

}/*endof namespace*/
#endif
