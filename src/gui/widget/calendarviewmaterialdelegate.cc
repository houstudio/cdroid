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

CalendarViewMaterialDelegate::CalendarViewMaterialDelegate(CalendarView* delegator, Context* context,const AttributeSet* attrs)
    :CalendarView::AbstractCalendarViewDelegate(delegator,context){
    mDayPickerView = new DayPickerView(context, attrs);
    mDayPickerView->setId(View::NO_ID); 
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

// CalendarView's public header face is string-keyed ("@[pkg:]style/name");
// DayPickerView takes ids — bridge here (parse + arsc getIdentifier).
static int resolveStyleResId(Context* ctx, const std::string& ref) {
    std::string s = (!ref.empty() && ref[0]=='@') ? ref.substr(1) : ref;
    const size_t slash = s.rfind('/');
    if (slash == std::string::npos) return 0;
    const size_t colon = s.rfind(':');
    const size_t typeStart = (colon==std::string::npos)?0:colon+1;
    return ctx->getResources().getIdentifier(s.substr(slash+1),
        s.substr(typeStart, slash-typeStart),
        (colon==std::string::npos)?std::string():s.substr(0,colon));
}

void CalendarViewMaterialDelegate::setWeekDayTextAppearance(const std::string& resId){
    mDayPickerView->setDayOfWeekTextAppearance(resolveStyleResId(mDelegator->getContext(), resId));
}

std::string CalendarViewMaterialDelegate::getWeekDayTextAppearance() const{
    return mDelegator->getContext()->getResourceName((uint32_t)mDayPickerView->getDayOfWeekTextAppearance());
}

void CalendarViewMaterialDelegate::setDateTextAppearance(const std::string&resId){
    mDayPickerView->setDayTextAppearance(resolveStyleResId(mDelegator->getContext(), resId));
}

std::string CalendarViewMaterialDelegate::getDateTextAppearance() const{
    return mDelegator->getContext()->getResourceName((uint32_t)mDayPickerView->getDayTextAppearance());
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
        // CDROID Rect is width/height: top += offset already shifts bottom.
        //outBounds.bottom += extraVerticalOffset;
        return true;
    }
    return false;
}
}/*endof name space*/
