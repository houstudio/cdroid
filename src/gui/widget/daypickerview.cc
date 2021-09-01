#include <widget/daypickerview.h>
#include <widget/simplemonthview.h>

namespace cdroid{

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
    int leftDW = leftButton->getMeasuredWidth();
    int leftDH = leftButton->getMeasuredHeight();
    int leftIconTop = monthView->getPaddingTop() + (monthHeight - leftDH) / 2;
    int leftIconLeft = monthView->getPaddingLeft() + (cellWidth - leftDW) / 2;
        leftButton->layout(leftIconLeft, leftIconTop, leftIconLeft + leftDW, leftIconTop + leftDH);

    int rightDW = rightButton->getMeasuredWidth();
    int rightDH = rightButton->getMeasuredHeight();
    int rightIconTop = monthView->getPaddingTop() + (monthHeight - rightDH) / 2;
    int rightIconRight = width - monthView->getPaddingRight() - (cellWidth - rightDW) / 2;
    rightButton->layout(rightIconRight - rightDW, rightIconTop,
                rightIconRight, rightIconTop + rightDH);
}

void DayPickerView::setDate(long timeInMillis) {
    setDate(timeInMillis, false);
}

void DayPickerView::setDate(long timeInMillis, bool animate) {
    setDate(timeInMillis, animate, true);
}

void DayPickerView::setDate(long timeInMillis, bool animate, bool setSelected) {
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

    mAdapter->setSelectedDay(mTempCalendar);
}

long DayPickerView::getDate() {
    return mSelectedDay.getTimeInMillis();
}

bool DayPickerView::getBoundsForDate(long timeInMillis,Rect& outBounds) {
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

void DayPickerView::setMinDate(long timeInMillis) {
    mMinDate.setTimeInMillis(timeInMillis);
    onRangeChanged();
}

long DayPickerView::getMinDate() {
    return mMinDate.getTimeInMillis();
}

void DayPickerView::setMaxDate(long timeInMillis) {
    mMaxDate.setTimeInMillis(timeInMillis);
    onRangeChanged();
}

long DayPickerView::getMaxDate() {
    return mMaxDate.getTimeInMillis();
}

/**
 * Handles changes to date range.
 */
void DayPickerView::onRangeChanged() {
    mAdapter->setRange(mMinDate, mMaxDate);

    // Changing the min/max date changes the selection position since we
    // don't really have stable IDs. Jumps immediately to the new position.
    setDate(mSelectedDay.getTimeInMillis(), false, false);

    updateButtonVisibility(mViewPager->getCurrentItem());
}

/**
 * Sets the listener to call when the user selects a day.
 *
 * @param listener The listener to call.
 */
void DayPickerView::setOnDaySelectedListener(OnDaySelectedListener listener) {
    mOnDaySelectedListener = listener;
}

int DayPickerView::getDiffMonths(Calendar& start, Calendar& end) {
    int diffYears = end.get(Calendar::YEAR) - start.get(Calendar::YEAR);
    return end.get(Calendar::MONTH) - start.get(Calendar::MONTH) + 12 * diffYears;
}

static int constrain(int amount, int low, int high) {//get the
    return amount < low ? low : (amount > high ? high : amount);
}

int DayPickerView::getPositionFromDay(long timeInMillis) {
    Calendar ctmp=getTempCalendarForTime(timeInMillis);
    int diffMonthMax = getDiffMonths(mMinDate, mMaxDate);
    int diffMonth = getDiffMonths(mMinDate, ctmp);
    return constrain(diffMonth, 0, diffMonthMax);
}

Calendar DayPickerView::getTempCalendarForTime(long timeInMillis) {
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void DayPickerPagerAdapter::setRange(Calendar&min,Calendar&max){
    mMinDate.setTimeInMillis(min.getTimeInMillis());
    mMaxDate.setTimeInMillis(max.getTimeInMillis());

    int diffYear = mMaxDate.get(Calendar::YEAR) - mMinDate.get(Calendar::YEAR);
    int diffMonth = mMaxDate.get(Calendar::MONTH) - mMinDate.get(Calendar::MONTH);
    mCount = diffMonth + MONTHS_IN_YEAR * diffYear + 1;

    // Positions are now invalid, clear everything and start over.
    notifyDataSetChanged();
}

void* DayPickerPagerAdapter::instantiateItem(ViewGroup* container, int position){
}

void DayPickerPagerAdapter::destroyItem(ViewGroup* container, int position, void* object){
}

int DayPickerPagerAdapter::getItemPosition(void* object){
    ViewHolder* holder = (ViewHolder*) object;
    return holder->position;
}

std::string DayPickerPagerAdapter::getPageTitle(int position){
    SimpleMonthView* v = mItems.get(position)->calendar;
    if (v) return v->getMonthYearLabel();
    return std::string();
}
void DayPickerPagerAdapter::setFirstDayOfWeek(int weekStart){
    mFirstDayOfWeek = weekStart;

    // Update displayed views.
    int count = mItems.size();
    for (int i = 0; i < count; i++) {
        SimpleMonthView* monthView = mItems.valueAt(i)->calendar;
        monthView->setFirstDayOfWeek(weekStart);
    }
}

int DayPickerPagerAdapter::getFirstDayOfWeek() {
    return mFirstDayOfWeek;
}

bool DayPickerPagerAdapter::getBoundsForDate(Calendar& day, Rect& outBounds){
    int position = getPositionForDay(&day);
    ViewHolder* monthView = mItems.get(position,nullptr);
    if (monthView == nullptr) {
        return false;
    } else {
        int dayOfMonth = day.get(Calendar::DAY_OF_MONTH);
        return monthView->calendar->getBoundsForDay(dayOfMonth, outBounds);
    }
}

void DayPickerPagerAdapter::setSelectedDay(Calendar& day) {
    int oldPosition = getPositionForDay(mSelectedDay);
    int newPosition = getPositionForDay(&day);

    // Clear the old position if necessary.
    if (oldPosition != newPosition && oldPosition >= 0) {
        ViewHolder* oldMonthView = mItems.get(oldPosition,nullptr);
        if (oldMonthView != nullptr) {
            oldMonthView->calendar->setSelectedDay(-1);
        }
    }

    // Set the new position.
    if (newPosition >= 0) {
        ViewHolder* newMonthView = mItems.get(newPosition,nullptr);
        if (newMonthView != nullptr) {
            int dayOfMonth = day.get(Calendar::DAY_OF_MONTH);
            newMonthView->calendar->setSelectedDay(dayOfMonth);
        }
    }
    mSelectedDay = &day;
}

void DayPickerPagerAdapter::setOnDaySelectedListener(OnDaySelectedListener listener) {
    mOnDaySelectedListener = listener;
}

void DayPickerPagerAdapter::setCalendarTextColor(ColorStateList* calendarTextColor) {
    mCalendarTextColor = calendarTextColor;
    notifyDataSetChanged();
}

void DayPickerPagerAdapter::setDaySelectorColor(ColorStateList* selectorColor) {
    mDaySelectorColor = selectorColor;
    notifyDataSetChanged();
}
#if 0
void DayPickerPagerAdapter::setMonthTextAppearance(int resId) {
    mMonthTextAppearance = resId;
    notifyDataSetChanged();
}

void DayPickerPagerAdapter::setDayOfWeekTextAppearance(int resId) {
    mDayOfWeekTextAppearance = resId;
    notifyDataSetChanged();
}

int DayPickerPagerAdapter::getDayOfWeekTextAppearance() {
    return mDayOfWeekTextAppearance;
}

void DayPickerPagerAdapter::setDayTextAppearance(int resId) {
    mDayTextAppearance = resId;
    notifyDataSetChanged();
}

int DayPickerPagerAdapter::getDayTextAppearance() {
    return mDayTextAppearance;
}
#endif
bool DayPickerPagerAdapter::isViewFromObject(View* view, void* object) {
    ViewHolder* holder = (ViewHolder*) object;
    return view == holder->container;
}

int DayPickerPagerAdapter::getMonthForPosition(int position) {
    return (position + mMinDate.get(Calendar::MONTH)) % MONTHS_IN_YEAR;
}

int DayPickerPagerAdapter::getYearForPosition(int position) {
    int yearOffset = (position + mMinDate.get(Calendar::MONTH)) / MONTHS_IN_YEAR;
    return yearOffset + mMinDate.get(Calendar::YEAR);
}

int DayPickerPagerAdapter::getPositionForDay(Calendar* day) {
    if (day == nullptr)  return -1;

    int yearOffset  = day->get(Calendar::YEAR ) - mMinDate.get(Calendar::YEAR);
    int monthOffset = day->get(Calendar::MONTH) - mMinDate.get(Calendar::MONTH);
    int position = yearOffset * MONTHS_IN_YEAR + monthOffset;
    return position;
}


}//namespace
