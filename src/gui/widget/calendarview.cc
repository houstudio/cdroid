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
#include <widget/calendarview.h>
#include <widget/calendarviewlegacydelegate.h>
#include <widget/calendarviewmaterialdelegate.h>
namespace cdroid{

CalendarView::CalendarView(int w,int h):FrameLayout(w,h){
}

CalendarView::CalendarView(Context*context,const AttributeSet&attrs)
  :FrameLayout(context,attrs){
    const int mode = attrs.getInt("calendarViewMode",std::unordered_map<std::string,int>{

            }, MODE_HOLO);
    switch (mode) {
    case MODE_HOLO:
        mDelegate = new CalendarViewLegacyDelegate(this, context, attrs);
        break;
    case MODE_MATERIAL:
        mDelegate = new CalendarViewMaterialDelegate(this, context, attrs);
        break;
    default:
        throw std::invalid_argument("invalid calendarViewMode attribute");
    }
}

CalendarView::~CalendarView(){
    delete mDelegate;
}

void CalendarView::setShownWeekCount(int count) {
        mDelegate->setShownWeekCount(count);
}

int CalendarView::getShownWeekCount() const{
    return mDelegate->getShownWeekCount();
}

void CalendarView::setSelectedWeekBackgroundColor(int color) {
    mDelegate->setSelectedWeekBackgroundColor(color);
}

int CalendarView::getSelectedWeekBackgroundColor() const{
    return mDelegate->getSelectedWeekBackgroundColor();
}

void CalendarView::setFocusedMonthDateColor(int color) {
    mDelegate->setFocusedMonthDateColor(color);
}

int CalendarView::getFocusedMonthDateColor() const{
    return mDelegate->getFocusedMonthDateColor();
}

void CalendarView::setUnfocusedMonthDateColor(int color) {
    mDelegate->setUnfocusedMonthDateColor(color);
}

int CalendarView::getUnfocusedMonthDateColor() const{
    return mDelegate->getUnfocusedMonthDateColor();
}

void CalendarView::setWeekNumberColor(int color) {
    mDelegate->setWeekNumberColor(color);
}

int CalendarView::getWeekNumberColor() const{
    return mDelegate->getWeekNumberColor();
}

void CalendarView::setWeekSeparatorLineColor(int color) {
    mDelegate->setWeekSeparatorLineColor(color);
}

int CalendarView::getWeekSeparatorLineColor() const{
    return mDelegate->getWeekSeparatorLineColor();
}

void CalendarView::setSelectedDateVerticalBar(const std::string& resourceId) {
    mDelegate->setSelectedDateVerticalBar(resourceId);
}

void CalendarView::setSelectedDateVerticalBar(Drawable* drawable) {
    mDelegate->setSelectedDateVerticalBar(drawable);
}

Drawable* CalendarView::getSelectedDateVerticalBar() const{
    return mDelegate->getSelectedDateVerticalBar();
}

void CalendarView::setWeekDayTextAppearance(const std::string& resourceId) {
    mDelegate->setWeekDayTextAppearance(resourceId);
}

std::string CalendarView::getWeekDayTextAppearance() const{
    return mDelegate->getWeekDayTextAppearance();
}

void CalendarView::setDateTextAppearance(const std::string& resourceId) {
    mDelegate->setDateTextAppearance(resourceId);
}

std::string CalendarView::getDateTextAppearance() const{
    return mDelegate->getDateTextAppearance();
}

int64_t CalendarView::getMinDate() const{
    return mDelegate->getMinDate();
}

void CalendarView::setMinDate(int64_t minDate) {
    mDelegate->setMinDate(minDate);
}

int64_t CalendarView::getMaxDate() const{
    return mDelegate->getMaxDate();
}

void CalendarView::setMaxDate(int64_t maxDate) {
    mDelegate->setMaxDate(maxDate);
}

void CalendarView::setShowWeekNumber(bool showWeekNumber) {
    mDelegate->setShowWeekNumber(showWeekNumber);
}

bool CalendarView::getShowWeekNumber() const{
    return mDelegate->getShowWeekNumber();
}

int CalendarView::getFirstDayOfWeek() const{
    return mDelegate->getFirstDayOfWeek();
}

void CalendarView::setFirstDayOfWeek(int firstDayOfWeek) {
    mDelegate->setFirstDayOfWeek(firstDayOfWeek);
}

void CalendarView::setOnDateChangeListener(const OnDateChangeListener& listener) {
    mDelegate->setOnDateChangeListener(listener);
}

int64_t CalendarView::getDate(){
    return mDelegate->getDate();
}

void CalendarView::setDate(int64_t date) {
    mDelegate->setDate(date);
}

void CalendarView::setDate(int64_t date, bool animate, bool center) {
    mDelegate->setDate(date, animate, center);
}

bool CalendarView::getBoundsForDate(int64_t date, Rect& outBounds) {
    return mDelegate->getBoundsForDate(date, outBounds);
}

/*void CalendarView::onConfigurationChanged(Configuration newConfig {
    FrameLayout::onConfigurationChanged(newConfig);
    mDelegate->onConfigurationChanged(newConfig);
}*/

std::string CalendarView::getAccessibilityClassName() const{
    return "CalendarView";
}

bool CalendarView::parseDate(const std::string& date, Calendar& outDate){
    return true;
}

}//namespace
