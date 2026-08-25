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
#include <widget/internal_R.h>
#include <widget/daypickerview.h>
#include <widget/simplemonthview.h>
#include <widget/calendarview.h>
#include <widget/framework_styleable.h>
#include <core/typedarray.h>
#include <porting/cdlog.h>
#include <utils/mathutils.h>

namespace cdroid{
using namespace cdroid::internal;

DECLARE_WIDGET(DayPickerView);

DayPickerView::DayPickerView(Context*ctx)
    :DayPickerView(ctx,nullptr){}

// AOSP DayPickerView(Context, AttributeSet) chains to defStyleAttr
// R.attr.calendarViewStyle, so Widget.Material.CalendarView's text appearances
// and day selector color apply from the theme.
DayPickerView::DayPickerView(Context* context,const AttributeSet* attrs)
    :DayPickerView(context, attrs, cdroid::internal::R::attr::calendarViewStyle){}

DayPickerView::DayPickerView(Context* context,const AttributeSet* pAttrs,int defStyleAttr)
    :DayPickerView(context,pAttrs,defStyleAttr,0){}

DayPickerView::DayPickerView(Context* context,const AttributeSet* pAttrs,int defStyleAttr,int defStyleRes)
    :ViewGroup(context, pAttrs, defStyleAttr, defStyleRes){

    Calendar tempDate;
    auto a = mContext->obtainStyledAttributes(pAttrs, R::styleable::CalendarView, defStyleAttr, defStyleRes);
    const int firstDayOfWeek = a ? a->getInt(R::styleable::CalendarView_firstDayOfWeek, tempDate.getFirstDayOfWeek()) : tempDate.getFirstDayOfWeek();

    const std::string minDate = a ? a->getString(R::styleable::CalendarView_minDate) : std::string();
    const std::string maxDate = a ? a->getString(R::styleable::CalendarView_maxDate) : std::string();

    // AOSP DayPickerView defaults these to the material calendar text
    // appearances when the styleable doesn't carry them; without the defaults
    // every text falls back to Paint's plain black (invisible on dark themes).
    const int monthTextAppearanceResId = a ? a->getResourceId(
            R::styleable::CalendarView_monthTextAppearance,
            R::style::TextAppearance_Material_Widget_Calendar_Month) : 0;
    const int dayOfWeekTextAppearanceResId = a ? a->getResourceId(
            R::styleable::CalendarView_weekDayTextAppearance,
            R::style::TextAppearance_Material_Widget_Calendar_DayOfWeek) : 0;
    const int dayTextAppearanceResId = a ? a->getResourceId(
            R::styleable::CalendarView_dateTextAppearance,
            R::style::TextAppearance_Material_Widget_Calendar_Day) : 0;

    auto daySelectorColor = a ? a->getColorStateList(R::styleable::CalendarView_daySelectorColor) : nullptr;

    // Set up adapter.
    mAdapter = new DayPickerPagerAdapter(context, R::layout::date_picker_month_item_material, R::id::month_view);
    mAdapter->setMonthTextAppearance(monthTextAppearanceResId);
    mAdapter->setDayOfWeekTextAppearance(dayOfWeekTextAppearanceResId);
    mAdapter->setDayTextAppearance(dayTextAppearanceResId);
    mAdapter->setDaySelectorColor(daySelectorColor);

    LayoutInflater* inflater = LayoutInflater::from(context);
    ViewGroup* content = (ViewGroup*) inflater->inflate(R::layout::day_picker_content_material, this, false);

    // Transfer all children from content to here.
    while (content->getChildCount() > 0) {
        View* child = content->getChildAt(0);
        content->removeViewAt(0);
        addView(child);
    }
    // attachToRoot=false leaves the inflated root owned by this caller; the
    // emptied shell must be freed (no GC — otherwise it leaks per DayPickerView).
    delete content;

    mPrevButton = (ImageButton*)findViewById(R::id::prev);
    auto clickListener =[this](View&view){onButtonClick(view);};
    mPrevButton->setOnClickListener(clickListener);

    mNextButton = (ImageButton*)findViewById(R::id::next);
    mNextButton->setOnClickListener(clickListener);

    mViewPager = (ViewPager*)findViewById(R::id::day_picker_view_pager);
    mViewPager->setAdapter(mAdapter);

    ViewPager::OnPageChangeListener pcl;
    pcl.onPageScrolled=[this](int position, float positionOffset, int positionOffsetPixels){
        const float alpha = std::abs(0.5f - positionOffset) * 2.0f;
        mPrevButton->setAlpha(alpha);
        mNextButton->setAlpha(alpha);
    };
    pcl.onPageSelected=[this](int position) {
        updateButtonVisibility(position);
    };
    mViewPager->addOnPageChangeListener(pcl);

    // Proxy the month text color into the previous and next buttons.
    if (monthTextAppearanceResId != 0) {
        // AOSP: mPrevNextButtonColor = textAppearance.getTextColor(); resolve the
        // style's textColor through its own typed resolution.
        static const uint32_t kTextColor[] = { R::attr::textColor, 0 };
        auto taStyle = mContext->obtainStyledAttributes(monthTextAppearanceResId, R::styleable::TextAppearance);
        auto monthColor = taStyle ? taStyle->getColorStateList(R::styleable::TextAppearance_textColor) : nullptr;
        if (monthColor == nullptr) {
            auto ta = mContext->obtainStyledAttributes(kTextColor);
            monthColor = ta ? ta->getColorStateList(0) : nullptr;
        }
        if (monthColor != nullptr) {
            mPrevButton->setImageTintList(monthColor);
            mNextButton->setImageTintList(monthColor);
        }
    }

    // Set up min and max dates.
    if (!CalendarView::parseDate(minDate, tempDate)) {
        tempDate.set(DEFAULT_START_YEAR, Calendar::JANUARY, 1);
    }
    const int64_t minDateMillis = tempDate.getTimeInMillis();

    if (!CalendarView::parseDate(maxDate, tempDate)) {
        tempDate.set(DEFAULT_END_YEAR, Calendar::DECEMBER, 31);
    }
    const int64_t maxDateMillis = tempDate.getTimeInMillis();

    if (maxDateMillis < minDateMillis) {
        throw std::invalid_argument("maxDate must be >= minDate");
    }

    const int64_t setDateMillis = MathUtils::constrain(SystemClock::currentTimeMillis(), minDateMillis, maxDateMillis);

    setFirstDayOfWeek(firstDayOfWeek);
    setMinDate(minDateMillis);
    setMaxDate(maxDateMillis);
    setDate(setDateMillis, false);
    // Proxy selection callbacks to our own listener.
    DayPickerPagerAdapter::OnDaySelectedListener  dsl=[this](DayPickerPagerAdapter& adapter, Calendar& day){
        if (mOnDaySelectedListener != nullptr) {
            mOnDaySelectedListener(*this, day);
        }
    };
    mAdapter->setOnDaySelectedListener(dsl);
}

DayPickerView::~DayPickerView(){
    delete mAdapter;
}

void DayPickerView::onButtonClick(View&v){
    int direction;
    if (&v == mPrevButton) {
        direction = -1;
    } else if (&v == mNextButton) {
        direction = 1;
    } else {
        return;
    }
    const bool animate = true;//!mAccessibilityManager.isEnabled();
    const int nextItem = mViewPager->getCurrentItem() + direction;
    mViewPager->setCurrentItem(nextItem, animate);
}

void DayPickerView::updateButtonVisibility(int position) {
    bool hasPrev = position > 0;
    bool hasNext = position < (mAdapter->getCount() - 1);
    mPrevButton->setVisibility(hasPrev ? View::VISIBLE : View::INVISIBLE);
    mNextButton->setVisibility(hasNext ? View::VISIBLE : View::INVISIBLE);
}

void DayPickerView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    ViewPager* viewPager = mViewPager;
    measureChild(viewPager, widthMeasureSpec, heightMeasureSpec);

    int measuredWidthAndState = viewPager->getMeasuredWidthAndState();
    int measuredHeightAndState = viewPager->getMeasuredHeightAndState();
    setMeasuredDimension(measuredWidthAndState, measuredHeightAndState);

    int pagerWidth = viewPager->getMeasuredWidth();
    int pagerHeight = viewPager->getMeasuredHeight();
    int buttonWidthSpec = MeasureSpec::makeMeasureSpec(pagerWidth, MeasureSpec::AT_MOST);
    int buttonHeightSpec = MeasureSpec::makeMeasureSpec(pagerHeight, MeasureSpec::AT_MOST);
    mPrevButton->measure(buttonWidthSpec, buttonHeightSpec);
    mNextButton->measure(buttonWidthSpec, buttonHeightSpec);
}

void DayPickerView::onLayout(bool changed, int left, int top, int width, int height) {
    ImageButton* leftButton;
    ImageButton* rightButton;
    if (isLayoutRtl()) {
        leftButton = mNextButton;
        rightButton = mPrevButton;
    } else {
        leftButton = mPrevButton;
        rightButton = mNextButton;
    }

    mViewPager->layout(0, 0, width, height);

    SimpleMonthView* monthView = (SimpleMonthView*) mViewPager->getChildAt(0);
    int monthHeight = monthView->getMonthHeight();
    int cellWidth = monthView->getCellWidth();

    // Vertically center the previous/next buttons within the month
    // header, horizontally center within the day cell.
    // NOTE: CDROID View::layout takes (left, top, width, height), unlike
    // AOSP's (left, top, right, bottom) — pass sizes, not edges.
    int leftDW = leftButton->getMeasuredWidth();
    int leftDH = leftButton->getMeasuredHeight();
    int leftIconTop = monthView->getPaddingTop() + (monthHeight - leftDH) / 2;
    int leftIconLeft = monthView->getPaddingLeft() + (cellWidth - leftDW) / 2;
        leftButton->layout(leftIconLeft, leftIconTop, leftDW, leftDH);

    int rightDW = rightButton->getMeasuredWidth();
    int rightDH = rightButton->getMeasuredHeight();
    int rightIconTop = monthView->getPaddingTop() + (monthHeight - rightDH) / 2;
    int rightIconRight = width - monthView->getPaddingRight() - (cellWidth - rightDW) / 2;
    rightButton->layout(rightIconRight - rightDW, rightIconTop, rightDW, rightDH);
}

void DayPickerView::setDayOfWeekTextAppearance(int resId) {
    mAdapter->setDayOfWeekTextAppearance(resId);
}

int DayPickerView::getDayOfWeekTextAppearance() {
    return mAdapter->getDayOfWeekTextAppearance();
}

void DayPickerView::setDayTextAppearance(int resId) {
    mAdapter->setDayTextAppearance(resId);
}

int DayPickerView::getDayTextAppearance() {
    return mAdapter->getDayTextAppearance();
}

void DayPickerView::setDate(int64_t timeInMillis) {
    setDate(timeInMillis, false);
}

void DayPickerView::setDate(int64_t timeInMillis, bool animate) {
    setDate(timeInMillis, animate, true);
}

void DayPickerView::setDate(int64_t timeInMillis, bool animate, bool setSelected) {
    bool dateClamped = false;
    // Clamp the target day in milliseconds to the min or max if outside the range.
    if (timeInMillis < mMinDate.getTimeInMillis()) {
        timeInMillis = mMinDate.getTimeInMillis();
        dateClamped = true;
    } else if (timeInMillis > mMaxDate.getTimeInMillis()) {
        timeInMillis = mMaxDate.getTimeInMillis();
        dateClamped = true;
    }

    getTempCalendarForTime(timeInMillis);

    if (setSelected || dateClamped) {
        mSelectedDay.setTimeInMillis(timeInMillis);
    }

    int position = getPositionFromDay(timeInMillis);
    if (position != mViewPager->getCurrentItem()) {
        mViewPager->setCurrentItem(position, animate);
    }

    mAdapter->setSelectedDay(&mTempCalendar);
}

int64_t DayPickerView::getDate() {
    return mSelectedDay.getTimeInMillis();
}

bool DayPickerView::getBoundsForDate(int64_t timeInMillis,Rect& outBounds) {
    int position = getPositionFromDay(timeInMillis);
    if (position != mViewPager->getCurrentItem()) {
        return false;
    }

    mTempCalendar.setTimeInMillis(timeInMillis);
    return mAdapter->getBoundsForDate(mTempCalendar, outBounds);
}

void DayPickerView::setFirstDayOfWeek(int firstDayOfWeek) {
    mAdapter->setFirstDayOfWeek(firstDayOfWeek);
}

int DayPickerView::getFirstDayOfWeek() {
    return mAdapter->getFirstDayOfWeek();
}

void DayPickerView::setMinDate(int64_t timeInMillis) {
    mMinDate.setTimeInMillis(timeInMillis);
    onRangeChanged();
}

int64_t DayPickerView::getMinDate() {
    return mMinDate.getTimeInMillis();
}

void DayPickerView::setMaxDate(int64_t timeInMillis) {
    mMaxDate.setTimeInMillis(timeInMillis);
    onRangeChanged();
}

int64_t DayPickerView::getMaxDate() {
    return mMaxDate.getTimeInMillis();
}

void DayPickerView::onRangeChanged() {
    mAdapter->setRange(mMinDate, mMaxDate);

    // Changing the min/max date changes the selection position since we
    // don't really have stable IDs. Jumps immediately to the new position.
    setDate(mSelectedDay.getTimeInMillis(), false, false);

    updateButtonVisibility(mViewPager->getCurrentItem());
}

void DayPickerView::setOnDaySelectedListener(const OnDaySelectedListener& listener) {
    mOnDaySelectedListener = listener;
}

int DayPickerView::getDiffMonths(Calendar& start, Calendar& end) {
    int diffYears = end.get(Calendar::YEAR) - start.get(Calendar::YEAR);
    return end.get(Calendar::MONTH) - start.get(Calendar::MONTH) + 12 * diffYears;
}

int DayPickerView::getPositionFromDay(int64_t timeInMillis) {
    Calendar ctmp=getTempCalendarForTime(timeInMillis);
    int diffMonthMax = getDiffMonths(mMinDate, mMaxDate);
    int diffMonth = getDiffMonths(mMinDate, ctmp);
    return MathUtils::constrain(diffMonth, 0, diffMonthMax);
}

Calendar DayPickerView::getTempCalendarForTime(int64_t timeInMillis) {
    mTempCalendar.setTimeInMillis(timeInMillis);
    return mTempCalendar;
}

/**
 * Gets the position of the view that is most prominently displayed within the list view.
 */
int DayPickerView::getMostVisiblePosition() {
    return mViewPager->getCurrentItem();
}

void DayPickerView::setPosition(int position) {
    mViewPager->setCurrentItem(position, false);
}

}//namespace
