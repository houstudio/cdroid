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
#ifndef __DAYPICKER_PAGERADAPTER_H__
#define __DAYPICKER_PAGERADAPTER_H__
#include <widget/adapter.h>
#include <widget/simplemonthview.h>

namespace cdroid{

class DayPickerPagerAdapter:public PagerAdapter{
public:
    DECLARE_UIEVENT(void,OnDaySelectedListener,DayPickerPagerAdapter& view,Calendar& day);
private:
    static constexpr int MONTHS_IN_YEAR=12;
    class ViewHolder {
    public:
        int position;
        View* container;
        SimpleMonthView* calendar;
        ViewHolder(int position, View* container, SimpleMonthView* calendar);
    };
    Calendar mMinDate;
    Calendar mMaxDate;
    SparseArray<ViewHolder*>mItems;
    LayoutInflater* mInflater;
    std::string mLayoutResId;
    int mCalendarViewId;
    
    Calendar* mSelectedDay;

    std::string mMonthTextAppearance;
    std::string mDayOfWeekTextAppearance;
    std::string mDayTextAppearance;

    const ColorStateList* mCalendarTextColor;
    const ColorStateList* mDaySelectorColor;
    const ColorStateList* mDayHighlightColor;

    SimpleMonthView::OnDayClickListener mOnDayClickListener;
    OnDaySelectedListener mOnDaySelectedListener;

    int mCount;
    int mFirstDayOfWeek;
private:
    int getMonthForPosition(int position);
    int getYearForPosition(int position);
    int getPositionForDay(Calendar* day);
public:
    DayPickerPagerAdapter(Context* context,const std::string&layoutResId,int calendarViewId);
    ~DayPickerPagerAdapter();
    void setRange(Calendar& min,Calendar& max);
    void setFirstDayOfWeek(int weekStart);
    int getFirstDayOfWeek()const;
    bool getBoundsForDate(Calendar& day, Rect& outBounds);
    void setSelectedDay(Calendar* day);
    void setOnDaySelectedListener(const OnDaySelectedListener& listener);
    void setCalendarTextColor(const ColorStateList* calendarTextColor);
    void setDaySelectorColor(const ColorStateList* selectorColor);

    void setMonthTextAppearance(const std::string& resId);
    void setDayOfWeekTextAppearance(const std::string& resId);
    std::string getDayOfWeekTextAppearance();

    void setDayTextAppearance(const std::string&resId);
    std::string getDayTextAppearance();

    int getCount()override;
    bool isViewFromObject(View* view,void*object)override;
    void* instantiateItem(ViewGroup* container, int position)override;
    void destroyItem(ViewGroup* container, int position,void* object)override;
    int getItemPosition(void* object)override;
    std::string getPageTitle(int position)override;

    SimpleMonthView* getView(void* object);
};
}//namespace
#endif
