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

#include <widget/calendarviewmaterialdelegate.h>
namespace cdroid{

CalendarViewMaterialDelegate::CalendarViewMaterialDelegate(CalendarView* delegator, Context* context,const AttributeSet& attrs)
    :CalendarView::AbstractCalendarViewDelegate(delegator,context){
    mDayPickerView = new DayPickerView(context, attrs);

    DayPickerView::OnDaySelectedListener dsl= [this](DayPickerView& view, Calendar& day){
        if (mOnDateChangeListener) {
            const int year  = day.get(Calendar::YEAR);
            const int month = day.get(Calendar::MONTH);
            const int dayOfMonth = day.get(Calendar::DAY_OF_MONTH);
            mOnDateChangeListener(*mDelegator, year, month, dayOfMonth);
        }
    };
    mDayPickerView->setOnDaySelectedListener(dsl);

    delegator->addView(mDayPickerView);
}

void CalendarViewMaterialDelegate::setWeekDayTextAppearance(const std::string& resId){
    mDayPickerView->setDayOfWeekTextAppearance(resId);
}

std::string CalendarViewMaterialDelegate::getWeekDayTextAppearance() const{
    return mDayPickerView->getDayOfWeekTextAppearance();
}

void CalendarViewMaterialDelegate::setDateTextAppearance(const std::string&resId){
    mDayPickerView->setDayTextAppearance(resId);
}

std::string CalendarViewMaterialDelegate::getDateTextAppearance() const{
    return mDayPickerView->getDayTextAppearance();
}

void CalendarViewMaterialDelegate::setMinDate(int64_t minDate){
    mDayPickerView->setMinDate(minDate);
}

int64_t CalendarViewMaterialDelegate::getMinDate(){
    return mDayPickerView->getMinDate();
}

void CalendarViewMaterialDelegate::setMaxDate(int64_t maxDate){
    mDayPickerView->setMaxDate(maxDate);
}

int64_t CalendarViewMaterialDelegate::getMaxDate(){
    return mDayPickerView->getMaxDate();
}

void CalendarViewMaterialDelegate::setFirstDayOfWeek(int firstDayOfWeek){
    mDayPickerView->setFirstDayOfWeek(firstDayOfWeek);
}

int CalendarViewMaterialDelegate::getFirstDayOfWeek() const{
    return mDayPickerView->getFirstDayOfWeek();
}

void CalendarViewMaterialDelegate::setDate(int64_t date) {
    mDayPickerView->setDate(date, true);
}

void CalendarViewMaterialDelegate::setDate(int64_t date, bool animate, bool center) {
    mDayPickerView->setDate(date, animate);
}

int64_t CalendarViewMaterialDelegate::getDate() {
    return mDayPickerView->getDate();
}

void CalendarViewMaterialDelegate::setOnDateChangeListener(const CalendarView::OnDateChangeListener& listener) {
    mOnDateChangeListener = listener;
}

bool CalendarViewMaterialDelegate::getBoundsForDate(int64_t date, Rect& outBounds){
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
}/*endof name space*/
