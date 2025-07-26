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

#ifndef __CALENDAR_VIEW_MATERIAL_DELEGATE_H__
#define __CALENDAR_VIEW_MATERIAL_DELEGATE_H__
#include <widget/calendarview.h>
#include <widget/daypickerview.h>
namespace cdroid{

class CalendarViewMaterialDelegate:public CalendarView::AbstractCalendarViewDelegate {
private:
    DayPickerView* mDayPickerView;
    CalendarView::OnDateChangeListener mOnDateChangeListener;
public:
    CalendarViewMaterialDelegate(CalendarView* delegator, Context* context,const AttributeSet& attrs)
        :CalendarView::AbstractCalendarViewDelegate(delegator,context){
        mDayPickerView = new DayPickerView(context, attrs);

        mDayPickerView->setOnDaySelectedListener([this](DayPickerView& view, Calendar& day){
            if (mOnDateChangeListener) {
                int year  = day.get(Calendar::YEAR);
                int month = day.get(Calendar::MONTH);
                int dayOfMonth = day.get(Calendar::DAY_OF_MONTH);
                mOnDateChangeListener(*mDelegator, year, month, dayOfMonth);
            }
        });

        delegator->addView(mDayPickerView);
    }

    void setWeekDayTextAppearance(const std::string& resId) override{
        mDayPickerView->setDayOfWeekTextAppearance(resId);
    }

    const std::string getWeekDayTextAppearance() override{
        return mDayPickerView->getDayOfWeekTextAppearance();
    }

    void setDateTextAppearance(const std::string&resId) override{
        mDayPickerView->setDayTextAppearance(resId);
    }

    const std::string getDateTextAppearance() override{
        return mDayPickerView->getDayTextAppearance();
    }

    void setMinDate(int64_t minDate) override{
        mDayPickerView->setMinDate(minDate);
    }

    int64_t getMinDate() override{
        return mDayPickerView->getMinDate();
    }

    void setMaxDate(int64_t maxDate) override{
        mDayPickerView->setMaxDate(maxDate);
    }

    int64_t getMaxDate() override{
        return mDayPickerView->getMaxDate();
    }

    void setFirstDayOfWeek(int firstDayOfWeek) override{
        mDayPickerView->setFirstDayOfWeek(firstDayOfWeek);
    }

    int getFirstDayOfWeek() override{
        return mDayPickerView->getFirstDayOfWeek();
    }

    void setDate(int64_t date) {
        mDayPickerView->setDate(date, true);
    }

    void setDate(int64_t date, bool animate, bool center) override{
        mDayPickerView->setDate(date, animate);
    }

    int64_t getDate() {
        return mDayPickerView->getDate();
    }

    void setOnDateChangeListener(const CalendarView::OnDateChangeListener& listener) {
        mOnDateChangeListener = listener;
    }

    bool getBoundsForDate(int64_t date, Rect& outBounds) override{
        bool result = mDayPickerView->getBoundsForDate(date, outBounds);
        if (result) {
            // Found the date in the current picker. Now need to offset vertically to return correct
            // bounds in the coordinate system of the entire layout
            int dayPickerPositionOnScreen[2];
            int delegatorPositionOnScreen[2];
            mDayPickerView->getLocationOnScreen(dayPickerPositionOnScreen);
            mDelegator->getLocationOnScreen(delegatorPositionOnScreen);
            const int extraVerticalOffset =  dayPickerPositionOnScreen[1] - delegatorPositionOnScreen[1];
            outBounds.top += extraVerticalOffset;
            //outBounds.bottom += extraVerticalOffset;
            return true;
        }
        return false;
    }
};
}/*endof name space*/
#endif/*__CALENDAR_VIEW_MATERIAL_DELEGATE_H__*/
