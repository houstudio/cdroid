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
#include <widget/calendarviewlegacydelegate.h>
#include <widget/listview.h>
#include <widget/calendarview.h>
#include <core/systemclock.h>
#include <widget/R.h>
namespace cdroid{

CalendarViewLegacyDelegate::CalendarViewLegacyDelegate(CalendarView* delegator, Context* context,const AttributeSet& attrs)
    :CalendarView::AbstractCalendarViewDelegate(delegator,context){
    mDelegator = delegator;
    mScrollStateChangedRunnable = new ScrollStateRunnable(this);
    mShowWeekNumber= attrs.getBoolean("showWeekNumber",DEFAULT_SHOW_WEEK_NUMBER);
    Calendar cal;
    mFirstDayOfWeek= attrs.getInt("firstDayOfWeek",cal.getFirstDayOfWeek());
    const std::string minDate = attrs.getString("minDate");
    if (!CalendarView::parseDate(minDate, mMinDate)) {
        CalendarView::parseDate(DEFAULT_MIN_DATE, mMinDate);
    }
    const std::string maxDate = attrs.getString("maxDate");
    if (!CalendarView::parseDate(maxDate, mMaxDate)) {
        CalendarView::parseDate(DEFAULT_MAX_DATE, mMaxDate);
    }
    if (mMaxDate.before(mMinDate)) {
        throw std::invalid_argument("Max date cannot be before min date.");
    }
    mShownWeekCount = attrs.getInt("shownWeekCount", DEFAULT_SHOWN_WEEK_COUNT);
    mSelectedWeekBackgroundColor = attrs.getColor("selectedWeekBackgroundColor", 0);
    mFocusedMonthDateColor = attrs.getColor("focusedMonthDateColor", 0);
    mUnfocusedMonthDateColor = attrs.getColor("unfocusedMonthDateColor", 0);
    mWeekSeparatorLineColor = attrs.getColor("weekSeparatorLineColor", 0);
    mWeekNumberColor = attrs.getColor("weekNumberColor", 0);
    mSelectedDateVerticalBar = attrs.getDrawable("selectedDateVerticalBar");

    mDateTextAppearanceResId = attrs.getString("dateTextAppearance", "cdroid:attr/TextAppearance_Small");
    updateDateTextSize();

    mWeekDayTextAppearanceResId = attrs.getString("weekDayTextAppearance");//,DEFAULT_WEEK_DAY_TEXT_APPEARANCE_RES_ID);

    DisplayMetrics displayMetrics = mDelegator->getContext()->getDisplayMetrics();
    mWeekMinVisibleHeight = UNSCALED_WEEK_MIN_VISIBLE_HEIGHT;
    //(int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP,UNSCALED_WEEK_MIN_VISIBLE_HEIGHT, displayMetrics);
    mListScrollTopOffset = UNSCALED_LIST_SCROLL_TOP_OFFSET;
    //(int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, UNSCALED_LIST_SCROLL_TOP_OFFSET, displayMetrics);
    mBottomBuffer = UNSCALED_BOTTOM_BUFFER;//(int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, UNSCALED_BOTTOM_BUFFER, displayMetrics);
    mSelectedDateVerticalBarWidth = UNSCALED_SELECTED_DATE_VERTICAL_BAR_WIDTH;
    //(int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, UNSCALED_SELECTED_DATE_VERTICAL_BAR_WIDTH, displayMetrics);
    mWeekSeparatorLineWidth = UNSCALED_WEEK_SEPARATOR_LINE_WIDTH;
    // (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, UNSCALED_WEEK_SEPARATOR_LINE_WIDTH, displayMetrics);

    LayoutInflater* layoutInflater = LayoutInflater::from(mContext);
    View* content = layoutInflater->inflate("cdroid:layout/calendar_view", nullptr, false);
    mDelegator->addView(content);

    mListView = (ListView*)mDelegator->findViewById(R::id::list);
    mDayNamesHeader = (ViewGroup*)content->findViewById(R::id::day_names);
    mMonthName = (TextView*)content->findViewById(R::id::month_name);

    setUpHeader();
    setUpListView();
    setUpAdapter();

    // go to today or whichever is close to today min or max date
    mTempDate.setTimeInMillis(SystemClock::currentTimeMillis());
    if (mTempDate.before(mMinDate)) {
        goTo(mMinDate, false, true, true);
    } else if (mMaxDate.before(mTempDate)) {
        goTo(mMaxDate, false, true, true);
    } else {
        goTo(mTempDate, false, true, true);
    }

    mDelegator->invalidate();
}

CalendarViewLegacyDelegate::~CalendarViewLegacyDelegate(){
    delete mScrollStateChangedRunnable;
}

void CalendarViewLegacyDelegate::setShownWeekCount(int count) {
    if (mShownWeekCount != count) {
        mShownWeekCount = count;
        mDelegator->invalidate();
    }
}

int CalendarViewLegacyDelegate::getShownWeekCount() const{
    return mShownWeekCount;
}

void CalendarViewLegacyDelegate::setSelectedWeekBackgroundColor(int color) {
    if (mSelectedWeekBackgroundColor != color) {
        mSelectedWeekBackgroundColor = color;
        const int childCount = mListView->getChildCount();
        for (int i = 0; i < childCount; i++) {
            WeekView* weekView = (WeekView*) mListView->getChildAt(i);
            if (weekView->mHasSelectedDay) {
                weekView->invalidate();
            }
        }
    }
}

int CalendarViewLegacyDelegate::getSelectedWeekBackgroundColor() const{
    return mSelectedWeekBackgroundColor;
}

void CalendarViewLegacyDelegate::setFocusedMonthDateColor(int color) {
    if (mFocusedMonthDateColor != color) {
        mFocusedMonthDateColor = color;
        const int childCount = mListView->getChildCount();
        for (int i = 0; i < childCount; i++) {
            WeekView* weekView = (WeekView*) mListView->getChildAt(i);
            if (weekView->mHasFocusedDay) {
                weekView->invalidate();
            }
        }
    }
}

int CalendarViewLegacyDelegate::getFocusedMonthDateColor() const{
    return mFocusedMonthDateColor;
}

void CalendarViewLegacyDelegate::setUnfocusedMonthDateColor(int color) {
    if (mUnfocusedMonthDateColor != color) {
        mUnfocusedMonthDateColor = color;
        const int childCount = mListView->getChildCount();
        for (int i = 0; i < childCount; i++) {
            WeekView* weekView = (WeekView*) mListView->getChildAt(i);
            if (weekView->mHasUnfocusedDay) {
                weekView->invalidate();
            }
        }
    }
}

int CalendarViewLegacyDelegate::getUnfocusedMonthDateColor() const{
    return mUnfocusedMonthDateColor;
}

void CalendarViewLegacyDelegate::setWeekNumberColor(int color) {
    if (mWeekNumberColor != color) {
        mWeekNumberColor = color;
        if (mShowWeekNumber) {
            invalidateAllWeekViews();
        }
    }
}

int CalendarViewLegacyDelegate::getWeekNumberColor() const{
    return mWeekNumberColor;
}

void CalendarViewLegacyDelegate::setWeekSeparatorLineColor(int color) {
    if (mWeekSeparatorLineColor != color) {
        mWeekSeparatorLineColor = color;
        invalidateAllWeekViews();
    }
}

int CalendarViewLegacyDelegate::getWeekSeparatorLineColor() const{
    return mWeekSeparatorLineColor;
}

void CalendarViewLegacyDelegate::setSelectedDateVerticalBar(const std::string& resourceId) {
    Drawable* drawable = mDelegator->getContext()->getDrawable(resourceId);
    setSelectedDateVerticalBar(drawable);
}

void CalendarViewLegacyDelegate::setSelectedDateVerticalBar(Drawable* drawable) {
    if (mSelectedDateVerticalBar != drawable) {
        mSelectedDateVerticalBar = drawable;
        const int childCount = mListView->getChildCount();
        for (int i = 0; i < childCount; i++) {
            WeekView* weekView = (WeekView*) mListView->getChildAt(i);
            if (weekView->mHasSelectedDay) {
                weekView->invalidate();
            }
        }
    }
}

Drawable* CalendarViewLegacyDelegate::getSelectedDateVerticalBar() const{
    return mSelectedDateVerticalBar;
}

void CalendarViewLegacyDelegate::setWeekDayTextAppearance(const std::string& resourceId) {
    if (mWeekDayTextAppearanceResId != resourceId) {
        mWeekDayTextAppearanceResId = resourceId;
        setUpHeader();
    }
}

std::string CalendarViewLegacyDelegate::getWeekDayTextAppearance() const{
    return mWeekDayTextAppearanceResId;
}

void CalendarViewLegacyDelegate::setDateTextAppearance(const std::string& resourceId) {
    if (mDateTextAppearanceResId != resourceId) {
        mDateTextAppearanceResId = resourceId;
        updateDateTextSize();
        invalidateAllWeekViews();
    }
}

std::string CalendarViewLegacyDelegate::getDateTextAppearance() const{
    return mDateTextAppearanceResId;
}

void CalendarViewLegacyDelegate::setMinDate(int64_t minDate) {
    mTempDate.setTimeInMillis(minDate);
    if (isSameDate(mTempDate, mMinDate)) {
        return;
    }
    mMinDate.setTimeInMillis(minDate);
    // make sure the current date is not earlier than
    // the new min date since the latter is used for
    // calculating the indices in the adapter thus
    // avoiding out of bounds error
    Calendar date = mAdapter->mSelectedDate;
    if (date.before(mMinDate)) {
        mAdapter->setSelectedDay(mMinDate);
    }
    // reinitialize the adapter since its range depends on min date
    mAdapter->init();
    if (date.before(mMinDate)) {
        setDate(mTempDate.getTimeInMillis());
    } else {
        // we go to the current date to force the ListView to query its
        // adapter for the shown views since we have changed the adapter
        // range and the base from which the later calculates item indices
        // note that calling setDate will not work since the date is the same
        goTo(date, false, true, false);
    }
}

int64_t CalendarViewLegacyDelegate::getMinDate(){
    return mMinDate.getTimeInMillis();
}

void CalendarViewLegacyDelegate::setMaxDate(int64_t maxDate) {
    mTempDate.setTimeInMillis(maxDate);
    if (isSameDate(mTempDate, mMaxDate)) {
        return;
    }
    mMaxDate.setTimeInMillis(maxDate);
    // reinitialize the adapter since its range depends on max date
    mAdapter->init();
    Calendar date = mAdapter->mSelectedDate;
    if (date.after(mMaxDate)) {
        setDate(mMaxDate.getTimeInMillis());
    } else {
        // we go to the current date to force the ListView to query its
        // adapter for the shown views since we have changed the adapter
        // range and the base from which the later calculates item indices
        // note that calling setDate will not work since the date is the same
        goTo(date, false, true, false);
    }
}

int64_t CalendarViewLegacyDelegate::getMaxDate(){
    return mMaxDate.getTimeInMillis();
}

void CalendarViewLegacyDelegate::setShowWeekNumber(bool showWeekNumber) {
    if (mShowWeekNumber == showWeekNumber) {
        return;
    }
    mShowWeekNumber = showWeekNumber;
    mAdapter->notifyDataSetChanged();
    setUpHeader();
}

bool CalendarViewLegacyDelegate::getShowWeekNumber() const{
    return mShowWeekNumber;
}

void CalendarViewLegacyDelegate::setFirstDayOfWeek(int firstDayOfWeek) {
    if (mFirstDayOfWeek == firstDayOfWeek) {
        return;
    }
    mFirstDayOfWeek = firstDayOfWeek;
    mAdapter->init();
    mAdapter->notifyDataSetChanged();
    setUpHeader();
}

int CalendarViewLegacyDelegate::getFirstDayOfWeek() const{
    return mFirstDayOfWeek;
}

void CalendarViewLegacyDelegate::setDate(int64_t date) {
    setDate(date, false, false);
}

void CalendarViewLegacyDelegate::setDate(int64_t date, bool animate, bool center) {
    mTempDate.setTimeInMillis(date);
    if (isSameDate(mTempDate, mAdapter->mSelectedDate)) {
        return;
    }
    goTo(mTempDate, animate, true, center);
}

int64_t CalendarViewLegacyDelegate::getDate(){
    return mAdapter->mSelectedDate.getTimeInMillis();
}

void CalendarViewLegacyDelegate::setOnDateChangeListener(const CalendarView::OnDateChangeListener& listener) {
    mOnDateChangeListener = listener;
}

bool CalendarViewLegacyDelegate::getBoundsForDate(int64_t date, Rect& outBounds) {
    Calendar calendarDate;// = Calendar.getInstance();
    calendarDate.setTimeInMillis(date);
    const int listViewEntryCount = mListView->getCount();
    for (int i = 0; i < listViewEntryCount; i++) {
        WeekView* currWeekView = (WeekView*) mListView->getChildAt(i);
        if (currWeekView->getBoundsForDate(calendarDate, outBounds)) {
            // Found the date in this week. Now need to offset vertically to return correct
            // bounds in the coordinate system of the entire layout
            int weekViewPositionOnScreen[2];
            int delegatorPositionOnScreen[2];
            currWeekView->getLocationOnScreen(weekViewPositionOnScreen);
            mDelegator->getLocationOnScreen(delegatorPositionOnScreen);
            const int extraVerticalOffset =  weekViewPositionOnScreen[1] - delegatorPositionOnScreen[1];
            outBounds.top += extraVerticalOffset;
            //outBounds.bottom += extraVerticalOffset;
            return true;
        }
    }
    return false;
}

void CalendarViewLegacyDelegate::onConfigurationChanged(int newConfig) {
    //setCurrentLocale(newConfig.locale);
}

/*void CalendarViewLegacyDelegate::setCurrentLocale(Locale locale) {
    super.setCurrentLocale(locale);

    mTempDate = getCalendarForLocale(mTempDate, locale);
    mFirstDayOfMonth = getCalendarForLocale(mFirstDayOfMonth, locale);
    mMinDate = getCalendarForLocale(mMinDate, locale);
    mMaxDate = getCalendarForLocale(mMaxDate, locale);
}*/

void CalendarViewLegacyDelegate::updateDateTextSize() {
    AttributeSet attr = mDelegator->getContext()->obtainStyledAttributes(mDateTextAppearanceResId);//, "cdroid:attr/TextAppearance");
    mDateTextSize = attr.getDimensionPixelSize("textSize", DEFAULT_DATE_TEXT_SIZE);
}

void CalendarViewLegacyDelegate::invalidateAllWeekViews() {
    const int childCount = mListView->getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* view = mListView->getChildAt(i);
        view->invalidate();
    }
}
#if 0
Calendar CalendarViewLegacyDelegate::getCalendarForLocale(Calendar& oldCalendar, Locale& locale) {
    /*if (oldCalendar == null) {
        return Calendar.getInstance(locale);
    } else */{
        const long currentTimeMillis = oldCalendar.getTimeInMillis();
        Calendar newCalendar;// = Calendar.getInstance(locale);
        newCalendar.setTimeInMillis(currentTimeMillis);
        return newCalendar;
    }
}
#endif
bool CalendarViewLegacyDelegate::isSameDate(Calendar& firstDate, Calendar& secondDate) {
    return (firstDate.get(Calendar::DAY_OF_YEAR) == secondDate.get(Calendar::DAY_OF_YEAR)
            && firstDate.get(Calendar::YEAR) == secondDate.get(Calendar::YEAR));
}

void CalendarViewLegacyDelegate::setUpAdapter() {
    if (mAdapter == nullptr) {
        mAdapter = new WeeksAdapter(this,mContext);
        /*mAdapter.registerDataSetObserver(new DataSetObserver() {
            @Override
            public void onChanged() {
                if (mOnDateChangeListener != null) {
                    Calendar selectedDay = mAdapter.getSelectedDay();
                    mOnDateChangeListener.onSelectedDayChange(mDelegator,
                            selectedDay.get(Calendar.YEAR),
                            selectedDay.get(Calendar.MONTH),
                            selectedDay.get(Calendar.DAY_OF_MONTH));
                }
            }
        });*/
        mListView->setAdapter(mAdapter);
    }

    // refresh the view with the new parameters
    mAdapter->notifyDataSetChanged();
}

void CalendarViewLegacyDelegate::setUpHeader() {
    //mDayNamesShort = new String[mDaysPerWeek];
    //mDayNamesLong = new String[mDaysPerWeek];
    for (int i = mFirstDayOfWeek, count = mFirstDayOfWeek + mDaysPerWeek; i < count; i++) {
        int calendarDay = (i > Calendar::SATURDAY) ? i - Calendar::SATURDAY : i;
        mDayNamesShort[i - mFirstDayOfWeek] = std::to_string(calendarDay);
            //DateUtils::getDayOfWeekString(calendarDay,DateUtils::LENGTH_SHORTEST);
        mDayNamesLong[i - mFirstDayOfWeek] = std::to_string(calendarDay);
            //DateUtils::getDayOfWeekString(calendarDay,DateUtils::LENGTH_LONG);
    }

    TextView* label = (TextView*) mDayNamesHeader->getChildAt(0);
    if (mShowWeekNumber) {
        label->setVisibility(View::VISIBLE);
    } else {
        label->setVisibility(View::GONE);
    }
    for (int i = 1, count = mDayNamesHeader->getChildCount(); i < count; i++) {
        label = (TextView*) mDayNamesHeader->getChildAt(i);
        if (mWeekDayTextAppearanceResId.size()) {
            label->setTextAppearance(mWeekDayTextAppearanceResId);
        }
        if (i < mDaysPerWeek + 1) {
            label->setText(mDayNamesShort[i - 1]);
            label->setContentDescription(mDayNamesLong[i - 1]);
            label->setVisibility(View::VISIBLE);
        } else {
            label->setVisibility(View::GONE);
        }
    }
    mDayNamesHeader->invalidate();
}

void CalendarViewLegacyDelegate::setUpListView() {
    // Configure the listview
    mListView->setDivider(nullptr);
    mListView->setItemsCanFocus(true);
    mListView->setVerticalScrollBarEnabled(false);
    AbsListView::OnScrollListener ls;
    ls.onScrollStateChanged =[this](AbsListView& view, int scrollState){
        this->onScrollStateChanged(view, scrollState);
    };
    ls.onScroll=[this](AbsListView& view, int firstVisibleItem, int visibleItemCount,int totalItemCount){
        this->onScroll(view, firstVisibleItem, visibleItemCount, totalItemCount);
    };
    mListView->setOnScrollListener(ls);
    // Make the scrolling behavior nicer
    mListView->setFriction(mFriction);
    mListView->setVelocityScale(mVelocityScale);
}

void CalendarViewLegacyDelegate::goTo(Calendar& date, bool animate, bool setSelected, bool forceScroll) {
    if (date.before(mMinDate) || date.after(mMaxDate)) {
        FATAL("timeInMillis must be between the values of getMinDate() and getMaxDate()");
    }
    // Find the first and last entirely visible weeks
    int firstFullyVisiblePosition = mListView->getFirstVisiblePosition();
    View* firstChild = mListView->getChildAt(0);
    if (firstChild && firstChild->getTop() < 0) {
        firstFullyVisiblePosition++;
    }
    int lastFullyVisiblePosition = firstFullyVisiblePosition + mShownWeekCount - 1;
    if (firstChild && firstChild->getTop() > mBottomBuffer) {
        lastFullyVisiblePosition--;
    }
    if (setSelected) {
        mAdapter->setSelectedDay(date);
    }
    // Get the week we're going to
    int position = getWeeksSinceMinDate(date);

    // Check if the selected day is now outside of our visible range
    // and if so scroll to the month that contains it
    if (position < firstFullyVisiblePosition || position > lastFullyVisiblePosition
            || forceScroll) {
        mFirstDayOfMonth.setTimeInMillis(date.getTimeInMillis());
        mFirstDayOfMonth.set(Calendar::DAY_OF_MONTH, 1);

        setMonthDisplayed(mFirstDayOfMonth);

        // the earliest time we can scroll to is the min date
        if (mFirstDayOfMonth.before(mMinDate)) {
            position = 0;
        } else {
            position = getWeeksSinceMinDate(mFirstDayOfMonth);
        }

        mPreviousScrollState = AbsListView::OnScrollListener::SCROLL_STATE_FLING;
        if (animate) {
            mListView->smoothScrollToPositionFromTop(position, mListScrollTopOffset,
                    GOTO_SCROLL_DURATION);
        } else {
            mListView->setSelectionFromTop(position, mListScrollTopOffset);
            // Perform any after scroll operations that are needed
            onScrollStateChanged(*mListView, AbsListView::OnScrollListener::SCROLL_STATE_IDLE);
        }
    } else if (setSelected) {
        // Otherwise just set the selection
        setMonthDisplayed(date);
    }
}

void CalendarViewLegacyDelegate::onScrollStateChanged(AbsListView& view, int scrollState) {
    mScrollStateChangedRunnable->doScrollStateChange(&view,scrollState);
}

void CalendarViewLegacyDelegate::onScroll(AbsListView& view, int firstVisibleItem, int visibleItemCount,int totalItemCount) {
    WeekView* child = (WeekView*) view.getChildAt(0);
    if (child == nullptr) {
        return;
    }

    // Figure out where we are
    long currScroll = view.getFirstVisiblePosition() * child->getHeight() - child->getBottom();

    // If we have moved since our last call update the direction
    if (currScroll < mPreviousScrollPosition) {
        mIsScrollingUp = true;
    } else if (currScroll > mPreviousScrollPosition) {
        mIsScrollingUp = false;
    } else {
        return;
    }

    // Use some hysteresis for checking which month to highlight. This
    // causes the month to transition when two full weeks of a month are
    // visible when scrolling up, and when the first day in a month reaches
    // the top of the screen when scrolling down.
    int offset = child->getBottom() < mWeekMinVisibleHeight ? 1 : 0;
    if (mIsScrollingUp) {
        child = (WeekView*) view.getChildAt(SCROLL_HYST_WEEKS + offset);
    } else if (offset != 0) {
        child = (WeekView*) view.getChildAt(offset);
    }

    if (child != nullptr) {
        // Find out which month we're moving into
        int month;
        if (mIsScrollingUp) {
            month = child->getMonthOfFirstWeekDay();
        } else {
            month = child->getMonthOfLastWeekDay();
        }

        // And how it relates to our current highlighted month
        int monthDiff;
        if (mCurrentMonthDisplayed == 11 && month == 0) {
            monthDiff = 1;
        } else if (mCurrentMonthDisplayed == 0 && month == 11) {
            monthDiff = -1;
        } else {
            monthDiff = month - mCurrentMonthDisplayed;
        }

        // Only switch months if we're scrolling away from the currently
        // selected month
        if ((!mIsScrollingUp && monthDiff > 0) || (mIsScrollingUp && monthDiff < 0)) {
            Calendar firstDay = child->getFirstDay();
            if (mIsScrollingUp) {
                firstDay.add(Calendar::DAY_OF_MONTH, -DAYS_PER_WEEK);
            } else {
                firstDay.add(Calendar::DAY_OF_MONTH, DAYS_PER_WEEK);
            }
            setMonthDisplayed(firstDay);
        }
    }
    mPreviousScrollPosition = currScroll;
    mPreviousScrollState = mCurrentScrollState;
}

void CalendarViewLegacyDelegate::setMonthDisplayed(Calendar& calendar) {
    mCurrentMonthDisplayed = calendar.get(Calendar::MONTH);
    mAdapter->setFocusMonth(mCurrentMonthDisplayed);
    //const int flags = DateUtils.FORMAT_SHOW_DATE | DateUtils.FORMAT_NO_MONTH_DAY| DateUtils.FORMAT_SHOW_YEAR;
    const long millis = calendar.getTimeInMillis();
    std::string newMonthName = std::to_string(calendar.get(Calendar::MONTH));;//DateUtils.formatDateRange(mContext, millis, millis, flags);
    mMonthName->setText(newMonthName);
    mMonthName->invalidate();
}

int CalendarViewLegacyDelegate::getWeeksSinceMinDate(Calendar& date) {
    if (date.before(mMinDate)) {
        FATAL("fromDate:  + mMinDate.getTime()  does not precede toDate: + date.getTime()");
    }
    int64_t endTimeMillis = date.getTimeInMillis() ;//+ date.getTimeZone().getOffset(date.getTimeInMillis());
    int64_t startTimeMillis = mMinDate.getTimeInMillis() ;//+ mMinDate.getTimeZone().getOffset(mMinDate.getTimeInMillis());
    int64_t dayOffsetMillis = (mMinDate.get(Calendar::DAY_OF_WEEK) - mFirstDayOfWeek) * MILLIS_IN_DAY;
    return (int) ((endTimeMillis - startTimeMillis + dayOffsetMillis) / MILLIS_IN_WEEK);
}
/////////////////////////////////////////////////////////////////////

CalendarViewLegacyDelegate::ScrollStateRunnable::ScrollStateRunnable(CalendarViewLegacyDelegate*delegator){
    mDelegate = delegator;
    mRunnable  = [this](){run();};
}

void CalendarViewLegacyDelegate::ScrollStateRunnable::doScrollStateChange(AbsListView* view, int scrollState) {
    mView = view;
    mNewState = scrollState;
    mDelegate->mDelegator->removeCallbacks(mRunnable);
    mDelegate->mDelegator->postDelayed(mRunnable, SCROLL_CHANGE_DELAY);
}

void CalendarViewLegacyDelegate::ScrollStateRunnable::run() {
    mDelegate->mCurrentScrollState = mNewState;
    // Fix the position after a scroll or a fling ends
    if (mNewState == AbsListView::OnScrollListener::SCROLL_STATE_IDLE
            && mDelegate->mPreviousScrollState != AbsListView::OnScrollListener::SCROLL_STATE_IDLE) {
        View* child = mView->getChildAt(0);
        if (child == nullptr) {
            // The view is no longer visible, just return
            return;
        }
        const int dist = child->getBottom() - mDelegate->mListScrollTopOffset;
        if (dist > mDelegate->mListScrollTopOffset) {
            if (mDelegate->mIsScrollingUp) {
                mView->smoothScrollBy(dist - child->getHeight(), ADJUSTMENT_SCROLL_DURATION);
            } else {
                mView->smoothScrollBy(dist, ADJUSTMENT_SCROLL_DURATION);
            }
        }
    }
    mDelegate->mPreviousScrollState = mNewState;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
CalendarViewLegacyDelegate::WeeksAdapter::WeeksAdapter(CalendarViewLegacyDelegate*cv,Context* context)
    :BaseAdapter(){
    mCV = cv;
    mCV->mContext = context;
    GestureDetector::OnGestureListener gs;
    gs.onSingleTapUp=[](MotionEvent&){return true;};
    mGestureDetector = new GestureDetector(context, gs);
    init();
}

void CalendarViewLegacyDelegate::WeeksAdapter::init() {
    mSelectedWeek = mCV->getWeeksSinceMinDate(mSelectedDate);
    mTotalWeekCount = mCV->getWeeksSinceMinDate(mCV->mMaxDate);
    if (mCV->mMinDate.get(Calendar::DAY_OF_WEEK) != mCV->mFirstDayOfWeek
            || mCV->mMaxDate.get(Calendar::DAY_OF_WEEK) != mCV->mFirstDayOfWeek) {
        mTotalWeekCount++;
    }
    notifyDataSetChanged();
}

void CalendarViewLegacyDelegate::WeeksAdapter::setSelectedDay(Calendar& selectedDay) {
    if (selectedDay.get(Calendar::DAY_OF_YEAR) == mSelectedDate.get(Calendar::DAY_OF_YEAR)
            && selectedDay.get(Calendar::YEAR) == mSelectedDate.get(Calendar::YEAR)) {
        return;
    }
    mSelectedDate.setTimeInMillis(selectedDay.getTimeInMillis());
    mSelectedWeek = mCV->getWeeksSinceMinDate(mSelectedDate);
    mFocusedMonth = mSelectedDate.get(Calendar::MONTH);
    notifyDataSetChanged();
}

Calendar CalendarViewLegacyDelegate::WeeksAdapter::getSelectedDay() {
    return mSelectedDate;
}

int CalendarViewLegacyDelegate::WeeksAdapter::getCount() const{
    return mTotalWeekCount;
}

void* CalendarViewLegacyDelegate::WeeksAdapter::getItem(int position) const{
    return nullptr;
}

long CalendarViewLegacyDelegate::WeeksAdapter::getItemId(int position) const{
    return position;
}

View* CalendarViewLegacyDelegate::WeeksAdapter::getView(int position, View* convertView, ViewGroup* parent) {
    WeekView* weekView = nullptr;
    if (convertView != nullptr) {
        weekView = (WeekView*) convertView;
    } else {
        weekView = new WeekView(mCV,mCV->mContext,AttributeSet(mCV->mContext,"android"));
        AbsListView::LayoutParams* params =
                new AbsListView::LayoutParams(FrameLayout::LayoutParams::WRAP_CONTENT,
                        FrameLayout::LayoutParams::WRAP_CONTENT);
        weekView->setLayoutParams(params);
        weekView->setClickable(true);
        weekView->setOnTouchListener([this](View&v,MotionEvent&e){return onTouch(v,e);});
    }

    int selectedWeekDay = (mSelectedWeek == position) ? mSelectedDate.get(
            Calendar::DAY_OF_WEEK) : -1;
    weekView->init(position, selectedWeekDay, mFocusedMonth);

    return weekView;
}

void CalendarViewLegacyDelegate::WeeksAdapter::setFocusMonth(int month) {
    if (mFocusedMonth == month) {
        return;
    }
    mFocusedMonth = month;
    notifyDataSetChanged();
}

bool CalendarViewLegacyDelegate::WeeksAdapter::onTouch(View& v, MotionEvent& event) {
    if (mCV->mListView->isEnabled() && mGestureDetector->onTouchEvent(event)) {
        WeekView* weekView = (WeekView*) &v;
        // if we cannot find a day for the given location we are done
        Calendar& mTempDate=mCV->mTempDate;
        if (!weekView->getDayFromLocation(event.getX(), mTempDate)) {
            return true;
        }
        // it is possible that the touched day is outside the valid range
        // we draw whole weeks but range end can fall not on the week end
        if (mTempDate.before(mCV->mMinDate) || mTempDate.after(mCV->mMaxDate)) {
            return true;
        }
        onDateTapped(mTempDate);
        return true;
    }
    return false;
}

void CalendarViewLegacyDelegate::WeeksAdapter::onDateTapped(Calendar& day) {
    setSelectedDay(day);
    mCV->setMonthDisplayed(day);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CalendarViewLegacyDelegate::WeekView::WeekView(CalendarViewLegacyDelegate*cv,Context* context,const AttributeSet&attrs)
    :View(context,attrs),mCV(cv){
    // Sets up any standard paints that will be used
    initializePaints();
}

void CalendarViewLegacyDelegate::WeekView::init(int weekNumber, int selectedWeekDay, int focusedMonth) {
    mSelectedDay = selectedWeekDay;
    mHasSelectedDay = mSelectedDay != -1;
    mNumCells = mCV->mShowWeekNumber ? mCV->mDaysPerWeek + 1 : mCV->mDaysPerWeek;
    mWeek = weekNumber;
    Calendar& mTempDate=mCV->mTempDate;
    mTempDate.setTimeInMillis(mCV->mMinDate.getTimeInMillis());

    mTempDate.add(Calendar::WEEK_OF_YEAR, mWeek);
    mTempDate.setFirstDayOfWeek(mCV->mFirstDayOfWeek);

    // Allocate space for caching the day numbers and focus values
    //mDayNumbers = new String[mNumCells];
    //mFocusDay = new boolean[mNumCells];

    // If we're showing the week number calculate it based on Monday
    int i = 0;
    if (mCV->mShowWeekNumber) {
        mDayNumbers[0] = std::to_string(mTempDate.get(Calendar::WEEK_OF_YEAR));
        i++;
    }

    // Now adjust our starting day based on the start day of the week
    int diff = mCV->mFirstDayOfWeek - mTempDate.get(Calendar::DAY_OF_WEEK);
    mTempDate.add(Calendar::DAY_OF_MONTH, diff);

    mFirstDay = mTempDate;//(Calendar) mTempDate.clone();
    mMonthOfFirstWeekDay = mTempDate.get(Calendar::MONTH);

    mHasUnfocusedDay = true;
    for (; i < mNumCells; i++) {
        const bool isFocusedDay = (mTempDate.get(Calendar::MONTH) == focusedMonth);
        mFocusDay[i] = isFocusedDay;
        mHasFocusedDay |= isFocusedDay;
        mHasUnfocusedDay &= !isFocusedDay;
        // do not draw dates outside the valid range to avoid user confusion
        if (mTempDate.before(mCV->mMinDate) || mTempDate.after(mCV->mMaxDate)) {
            mDayNumbers[i] = "";
        } else {
            mDayNumbers[i] = std::to_string(mTempDate.get(Calendar::DAY_OF_MONTH));
        }
        mTempDate.add(Calendar::DAY_OF_MONTH, 1);
    }
    // We do one extra add at the end of the loop, if that pushed us to
    // new month undo it
    if (mTempDate.get(Calendar::DAY_OF_MONTH) == 1) {
        mTempDate.add(Calendar::DAY_OF_MONTH, -1);
    }
    mLastWeekDayMonth = mTempDate.get(Calendar::MONTH);

    updateSelectionPositions();
}

void CalendarViewLegacyDelegate::WeekView::initializePaints() {
    /*mDrawPaint.setFakeBoldText(false);
    mDrawPaint.setAntiAlias(true);
    mDrawPaint.setStyle(Paint.Style.FILL);

    mMonthNumDrawPaint.setFakeBoldText(true);
    mMonthNumDrawPaint.setAntiAlias(true);
    mMonthNumDrawPaint.setStyle(Paint.Style.FILL);
    mMonthNumDrawPaint.setTextAlign(Paint.Align.CENTER);
    mMonthNumDrawPaint.setTextSize(mCV->mDateTextSize);*/
}

/**
 * Returns the month of the first day in this week.
 *
 * @return The month the first day of this view is in.
 */
int CalendarViewLegacyDelegate::WeekView::getMonthOfFirstWeekDay() {
    return mMonthOfFirstWeekDay;
}

/**
 * Returns the month of the last day in this week
 *
 * @return The month the last day of this view is in
 */
int CalendarViewLegacyDelegate::WeekView::getMonthOfLastWeekDay() {
    return mLastWeekDayMonth;
}

/**
 * Returns the first day in this view.
 *
 * @return The first day in the view.
 */
Calendar& CalendarViewLegacyDelegate::WeekView::getFirstDay() {
    return mFirstDay;
}

/**
 * Calculates the day that the given x position is in, accounting for
 * week number.
 *
 * @param x The x position of the touch event.
 * @return True if a day was found for the given location.
 */
bool CalendarViewLegacyDelegate::WeekView::getDayFromLocation(float x, Calendar& outCalendar) {
    const bool IsLayoutRtl = isLayoutRtl();

    int start;
    int end;

    if (IsLayoutRtl) {
        start = 0;
        end = mCV->mShowWeekNumber ? mWidth - mWidth / mNumCells : mWidth;
    } else {
        start = mCV->mShowWeekNumber ? mWidth / mNumCells : 0;
        end = mWidth;
    }

    if (x < start || x > end) {
        outCalendar.clear();
        return false;
    }

    // Selection is (x - start) / (pixels/day) which is (x - start) * day / pixels
    int dayPosition = (int) ((x - start) * mCV->mDaysPerWeek / (end - start));

    if (IsLayoutRtl) {
        dayPosition = mCV->mDaysPerWeek - 1 - dayPosition;
    }

    outCalendar.setTimeInMillis(mFirstDay.getTimeInMillis());
    outCalendar.add(Calendar::DAY_OF_MONTH, dayPosition);

    return true;
}

bool CalendarViewLegacyDelegate::WeekView::getBoundsForDate(Calendar& date, Rect& outBounds) {
    Calendar currDay;// = Calendar.getInstance();
    currDay.setTime(mFirstDay.getTime());
    for (int i = 0; i < mCV->mDaysPerWeek; i++) {
        if ((date.get(Calendar::YEAR) == currDay.get(Calendar::YEAR))
            && (date.get(Calendar::MONTH) == currDay.get(Calendar::MONTH))
            && (date.get(Calendar::DAY_OF_MONTH) == currDay.get(Calendar::DAY_OF_MONTH))) {
            // We found the matching date. Follow the logic in the draw pass that divides
            // the available horizontal space equally between all the entries in this week.
            // Note that if we're showing week number, the start entry will be that number.
            int cellSize = mWidth / mNumCells;
            if (isLayoutRtl()) {
                outBounds.left = cellSize *
                        (mCV->mShowWeekNumber ? (mNumCells - i - 2) : (mNumCells - i - 1));
            } else {
                outBounds.left = cellSize * (mCV->mShowWeekNumber ? i + 1 : i);
            }
            outBounds.top = 0;
            outBounds.width = cellSize;
            outBounds.height = getHeight();
            return true;
        }
        // Add one day
        currDay.add(Calendar::DAY_OF_MONTH, 1);
    }
    return false;
}

void CalendarViewLegacyDelegate::WeekView::onDraw(Canvas& canvas) {
    drawBackground(canvas);
    drawWeekNumbersAndDates(canvas);
    drawWeekSeparators(canvas);
    drawSelectedDateVerticalBars(canvas);
}

/**
 * This draws the selection highlight if a day is selected in this week.
 *
 * @param canvas The canvas to draw on
 */
void CalendarViewLegacyDelegate::WeekView::drawBackground(Canvas& canvas) {
    if (!mHasSelectedDay) {
        return;
    }
    int recLeft,recTop,recRight,recBottom;
    canvas.set_color(mCV->mSelectedWeekBackgroundColor);

    recTop = mCV->mWeekSeparatorLineWidth;
    recBottom = mHeight;

    const bool bIsLayoutRtl = isLayoutRtl();

    if (bIsLayoutRtl) {
        recLeft = 0;
        recRight = mSelectedLeft - 2;
    } else {
        recLeft = mCV->mShowWeekNumber ? mWidth / mNumCells : 0;
        recRight = mSelectedLeft - 2;
    }
    canvas.rectangle(recLeft,recTop,recRight-recLeft,recBottom-recTop);

    if (bIsLayoutRtl) {
        recLeft = mSelectedRight + 3;
        recRight = mCV->mShowWeekNumber ? mWidth - mWidth / mNumCells : mWidth;
    } else {
        recLeft = mSelectedRight + 3;
        recRight = mWidth;
    }
    canvas.rectangle(recLeft,recTop,recRight-recLeft,recBottom-recTop);
}

void CalendarViewLegacyDelegate::WeekView::drawWeekNumbersAndDates(Canvas& canvas) {
    const float textHeight = mCV->mDateTextSize+2;//mDrawPaint.getTextSize();
    const int y = (int) ((mHeight + textHeight) / 2) - mCV->mWeekSeparatorLineWidth;
    const int nDays = mNumCells;
    const int divisor = 2 * nDays;

    canvas.set_font_size(mCV->mDateTextSize);

    Rect rect={0,0,mWidth/divisor,mHeight};
    if (isLayoutRtl()) {
        for (int i=0; i < nDays - 1; i++) {
            canvas.set_color(mFocusDay[i] ? mCV->mFocusedMonthDateColor
                    : mCV->mUnfocusedMonthDateColor);
            rect.left = (2 * i + 1) * mWidth / divisor;
            canvas.draw_text(rect,mDayNumbers[nDays - 1 - i]);
        }
        if (mCV->mShowWeekNumber) {
            canvas.set_color(mCV->mWeekNumberColor);
            rect.left = mWidth - mWidth / divisor;
            canvas.draw_text(rect,mDayNumbers[0]);
        }
    } else {
        int i=0;
        if (mCV->mShowWeekNumber) {
            canvas.set_color(mCV->mWeekNumberColor);
            rect.left = mWidth / divisor;
            canvas.draw_text(rect,mDayNumbers[0]);
            i++;
        }
        for (; i < nDays; i++) {
            canvas.set_color(mFocusDay[i] ? mCV->mFocusedMonthDateColor
                    : mCV->mUnfocusedMonthDateColor);
            rect.left = (2 * i + 1) * mWidth / divisor;
            canvas.draw_text(rect,mDayNumbers[i]);
        }
    }
}

void CalendarViewLegacyDelegate::WeekView::drawWeekSeparators(Canvas& canvas) {
    // If it is the topmost fully visible child do not draw separator line
    int firstFullyVisiblePosition = mCV->mListView->getFirstVisiblePosition();
    if (mCV->mListView->getChildAt(0)->getTop() < 0) {
        firstFullyVisiblePosition++;
    }
    if (firstFullyVisiblePosition == mWeek) {
        return;
    }
    canvas.set_color(mCV->mWeekSeparatorLineColor);
    canvas.set_line_width(mCV->mWeekSeparatorLineWidth);
    float startX, stopX;
    if (isLayoutRtl()) {
        startX = 0;
        stopX = mCV->mShowWeekNumber ? mWidth - mWidth / mNumCells : mWidth;
    } else {
        startX = mCV->mShowWeekNumber ? mWidth / mNumCells : 0;
        stopX = mWidth;
    }
    canvas.move_to(startX, 0);
    canvas.line_to(stopX, 0);
    canvas.stroke();
}

void CalendarViewLegacyDelegate::WeekView::drawSelectedDateVerticalBars(Canvas& canvas) {
    if (!mHasSelectedDay) {
        return;
    }
    mCV->mSelectedDateVerticalBar->setBounds(
            mSelectedLeft - mCV->mSelectedDateVerticalBarWidth / 2,
            mCV->mWeekSeparatorLineWidth,
            mSelectedLeft + mCV->mSelectedDateVerticalBarWidth / 2,
            mHeight);
    mCV->mSelectedDateVerticalBar->draw(canvas);
    mCV->mSelectedDateVerticalBar->setBounds(
            mSelectedRight - mCV->mSelectedDateVerticalBarWidth / 2,
            mCV->mWeekSeparatorLineWidth,
            mSelectedRight + mCV->mSelectedDateVerticalBarWidth / 2,
            mHeight);
    mCV->mSelectedDateVerticalBar->draw(canvas);
}

void CalendarViewLegacyDelegate::WeekView::onSizeChanged(int w, int h, int oldw, int oldh) {
    mWidth = w;
    updateSelectionPositions();
}

void CalendarViewLegacyDelegate::WeekView::updateSelectionPositions() {
    if (mHasSelectedDay) {
        const bool IsLayoutRtl = isLayoutRtl();
        int selectedPosition = mSelectedDay - mCV->mFirstDayOfWeek;
        if (selectedPosition < 0) {
            selectedPosition += 7;
        }
        if (mCV->mShowWeekNumber && !IsLayoutRtl) {
            selectedPosition++;
        }
        if (IsLayoutRtl) {
            mSelectedLeft = (mCV->mDaysPerWeek - 1 - selectedPosition) * mWidth / mNumCells;

        } else {
            mSelectedLeft = selectedPosition * mWidth / mNumCells;
        }
        mSelectedRight = mSelectedLeft + mWidth / mNumCells;
    }
}

void CalendarViewLegacyDelegate::WeekView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    ListView* mListView = mCV->mListView;
    mHeight = (mListView->getHeight() - mListView->getPaddingTop()
            - mListView->getPaddingBottom()) / mCV->mShownWeekCount;
    setMeasuredDimension(MeasureSpec::getSize(widthMeasureSpec), mHeight);
}
}/*endof namespace*/

