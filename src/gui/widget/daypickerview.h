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
#ifndef __DAY_PICKER_VIEW_H__
#define __DAY_PICKER_VIEW_H__
#include <view/viewgroup.h>
#include <widget/imagebutton.h>
#include <widget/adapter.h>
#include <widget/simplemonthview.h>
#include <widget/viewpager.h>
#include <widget/daypickerpageradapter.h>
#include <sparsearray.h>
#include <calendar.h>

namespace cdroid{

class DayPickerView:public ViewGroup{
public:
    DECLARE_UIEVENT(void,OnDaySelectedListener,DayPickerView&,Calendar&);
private:
    static constexpr int DEFAULT_LAYOUT =0;// R.layout.day_picker_content_material;
    static constexpr int DEFAULT_START_YEAR = 1900;
    static constexpr int DEFAULT_END_YEAR   = 2100;
    
    Calendar mSelectedDay;
    Calendar mMinDate ;
    Calendar mMaxDate ;
    /** Temporary calendar used for date calculations. */
    Calendar mTempCalendar;

    ViewPager* mViewPager;
    ImageButton* mPrevButton;
    ImageButton* mNextButton;

    class DayPickerPagerAdapter* mAdapter;
    OnDaySelectedListener mOnDaySelectedListener;
private:
    void onButtonClick(View&v);
    void updateButtonVisibility(int position);
    void setDate(int64_t timeInMillis, bool animate, bool setSelected);
    int getDiffMonths(Calendar& start, Calendar& end);
    int getPositionFromDay(int64_t timeInMillis);
    Calendar getTempCalendarForTime(int64_t timeInMillis);
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int left, int top, int width, int height)override;

public:
    DayPickerView(Context* context,const AttributeSet&atts);
    ~DayPickerView()override;
    void setDayOfWeekTextAppearance(const std::string& resId);
    const std::string getDayOfWeekTextAppearance();
    void  setDayTextAppearance(const std::string& resId);
    const std::string getDayTextAppearance();
    void setDate(int64_t timeInMillis);
    void setDate(int64_t timeInMillis, bool animate);
    int64_t getDate();
    bool getBoundsForDate(int64_t timeInMillis,Rect& outBounds);
    void setFirstDayOfWeek(int firstDayOfWeek);
    int getFirstDayOfWeek();
    void setMinDate(int64_t timeInMillis);
    int64_t getMinDate();
    void setMaxDate(int64_t timeInMillis);
    int64_t getMaxDate();
    void onRangeChanged();
    void setOnDaySelectedListener(const OnDaySelectedListener& listener);
    int getMostVisiblePosition();
    void setPosition(int position);

};

}//endof namespace
#endif/*__DAY_PICKER_VIEW_H__*/
