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
    CalendarViewMaterialDelegate(CalendarView* delegator, Context* context,const AttributeSet& attrs);

    void setWeekDayTextAppearance(const std::string& resId) override;
    std::string getWeekDayTextAppearance() const override;

    void setDateTextAppearance(const std::string&resId) override;
    std::string getDateTextAppearance() const override;

    void setMinDate(int64_t minDate) override;
    int64_t getMinDate() override;

    void setMaxDate(int64_t maxDate) override;
    int64_t getMaxDate() override;

    void setFirstDayOfWeek(int firstDayOfWeek) override;
    int getFirstDayOfWeek() const override;

    void setDate(int64_t date);
    void setDate(int64_t date, bool animate, bool center) override;
    int64_t getDate();

    void setOnDateChangeListener(const CalendarView::OnDateChangeListener& listener);
    bool getBoundsForDate(int64_t date, Rect& outBounds) override;
};
}/*endof name space*/
#endif/*__CALENDAR_VIEW_MATERIAL_DELEGATE_H__*/
