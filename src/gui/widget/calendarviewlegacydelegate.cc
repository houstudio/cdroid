#include <widget/calendarviewlegacydelegate.h>
#include <widget/listview.h>
#include <core/systemclock.h>
#include <widget/R.h>
namespace cdroid{

CalendarViewLegacyDelegate::CalendarViewLegacyDelegate(CalendarView* delegator, Context* context,const AttributeSet& attrs)
    :CalendarView::AbstractCalendarViewDelegate(delegator,context){
#if 0
    final TypedArray a = context.obtainStyledAttributes(attrs,
            R.styleable.CalendarView, defStyleAttr, defStyleRes);
    mShowWeekNumber = a.getBoolean(R.styleable.CalendarView_showWeekNumber,
            DEFAULT_SHOW_WEEK_NUMBER);
    mFirstDayOfWeek = a.getInt(R.styleable.CalendarView_firstDayOfWeek,
            Calendar.getInstance().getFirstDayOfWeek());
    const std::string minDate = a.getString(R.styleable.CalendarView_minDate);
    if (!CalendarView.parseDate(minDate, mMinDate)) {
        CalendarView.parseDate(DEFAULT_MIN_DATE, mMinDate);
    }
    const std::string maxDate = a.getString(R.styleable.CalendarView_maxDate);
    if (!CalendarView.parseDate(maxDate, mMaxDate)) {
        CalendarView.parseDate(DEFAULT_MAX_DATE, mMaxDate);
    }
    if (mMaxDate.before(mMinDate)) {
        throw new IllegalArgumentException("Max date cannot be before min date.");
    }
    mShownWeekCount = a.getInt(R.styleable.CalendarView_shownWeekCount,
            DEFAULT_SHOWN_WEEK_COUNT);
    mSelectedWeekBackgroundColor = a.getColor(
            R.styleable.CalendarView_selectedWeekBackgroundColor, 0);
    mFocusedMonthDateColor = a.getColor(
            R.styleable.CalendarView_focusedMonthDateColor, 0);
    mUnfocusedMonthDateColor = a.getColor(
            R.styleable.CalendarView_unfocusedMonthDateColor, 0);
    mWeekSeparatorLineColor = a.getColor(
            R.styleable.CalendarView_weekSeparatorLineColor, 0);
    mWeekNumberColor = a.getColor(R.styleable.CalendarView_weekNumberColor, 0);
    mSelectedDateVerticalBar = a.getDrawable(
            R.styleable.CalendarView_selectedDateVerticalBar);

    mDateTextAppearanceResId = a.getResourceId(
            R.styleable.CalendarView_dateTextAppearance, R.style.TextAppearance_Small);
    updateDateTextSize();

    mWeekDayTextAppearanceResId = a.getResourceId(
            R.styleable.CalendarView_weekDayTextAppearance,
            DEFAULT_WEEK_DAY_TEXT_APPEARANCE_RES_ID);
    a.recycle();

    DisplayMetrics displayMetrics = mDelegator.getResources().getDisplayMetrics();
    mWeekMinVisibleHeight = (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP,
            UNSCALED_WEEK_MIN_VISIBLE_HEIGHT, displayMetrics);
    mListScrollTopOffset = (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP,
            UNSCALED_LIST_SCROLL_TOP_OFFSET, displayMetrics);
    mBottomBuffer = (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP,
            UNSCALED_BOTTOM_BUFFER, displayMetrics);
    mSelectedDateVerticalBarWidth = (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP,
            UNSCALED_SELECTED_DATE_VERTICAL_BAR_WIDTH, displayMetrics);
    mWeekSeparatorLineWidth = (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP,
            UNSCALED_WEEK_SEPARATOR_LINE_WIDTH, displayMetrics);
#endif
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

void CalendarViewLegacyDelegate::setShownWeekCount(int count) {
    if (mShownWeekCount != count) {
        mShownWeekCount = count;
        mDelegator->invalidate();
    }
}

int CalendarViewLegacyDelegate::getShownWeekCount() {
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

int CalendarViewLegacyDelegate::getSelectedWeekBackgroundColor() {
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

int CalendarViewLegacyDelegate::getFocusedMonthDateColor() {
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

int CalendarViewLegacyDelegate::getUnfocusedMonthDateColor() {
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

int CalendarViewLegacyDelegate::getWeekNumberColor() {
    return mWeekNumberColor;
}

void CalendarViewLegacyDelegate::setWeekSeparatorLineColor(int color) {
    if (mWeekSeparatorLineColor != color) {
        mWeekSeparatorLineColor = color;
        invalidateAllWeekViews();
    }
}

int CalendarViewLegacyDelegate::getWeekSeparatorLineColor() {
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

Drawable* CalendarViewLegacyDelegate::getSelectedDateVerticalBar() {
    return mSelectedDateVerticalBar;
}

void CalendarViewLegacyDelegate::setWeekDayTextAppearance(const std::string& resourceId) {
    if (mWeekDayTextAppearanceResId != resourceId) {
        mWeekDayTextAppearanceResId = resourceId;
        setUpHeader();
    }
}

const std::string CalendarViewLegacyDelegate::getWeekDayTextAppearance() {
    return mWeekDayTextAppearanceResId;
}

void CalendarViewLegacyDelegate::setDateTextAppearance(const std::string& resourceId) {
    if (mDateTextAppearanceResId != resourceId) {
        mDateTextAppearanceResId = resourceId;
        updateDateTextSize();
        invalidateAllWeekViews();
    }
}

const std::string CalendarViewLegacyDelegate::getDateTextAppearance() {
    return mDateTextAppearanceResId;
}

void CalendarViewLegacyDelegate::setMinDate(long minDate) {
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

long CalendarViewLegacyDelegate::getMinDate() {
    return mMinDate.getTimeInMillis();
}

void CalendarViewLegacyDelegate::setMaxDate(long maxDate) {
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

long CalendarViewLegacyDelegate::getMaxDate() {
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

bool CalendarViewLegacyDelegate::getShowWeekNumber() {
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

int CalendarViewLegacyDelegate::getFirstDayOfWeek() {
    return mFirstDayOfWeek;
}

void CalendarViewLegacyDelegate::setDate(long date) {
    setDate(date, false, false);
}

void CalendarViewLegacyDelegate::setDate(long date, bool animate, bool center) {
    mTempDate.setTimeInMillis(date);
    if (isSameDate(mTempDate, mAdapter->mSelectedDate)) {
        return;
    }
    goTo(mTempDate, animate, true, center);
}

long CalendarViewLegacyDelegate::getDate() {
    return mAdapter->mSelectedDate.getTimeInMillis();
}

void CalendarViewLegacyDelegate::setOnDateChangeListener(const CalendarView::OnDateChangeListener& listener) {
    mOnDateChangeListener = listener;
}

bool CalendarViewLegacyDelegate::getBoundsForDate(long date, Rect& outBounds) {
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
    setCurrentLocale(newConfig.locale);
}

/**
 * Sets the current locale.
 *
 * @param locale The current locale.
 */
void setCurrentLocale(Locale locale) {
    super.setCurrentLocale(locale);

    mTempDate = getCalendarForLocale(mTempDate, locale);
    mFirstDayOfMonth = getCalendarForLocale(mFirstDayOfMonth, locale);
    mMinDate = getCalendarForLocale(mMinDate, locale);
    mMaxDate = getCalendarForLocale(mMaxDate, locale);
}

void CalendarViewLegacyDelegate::updateDateTextSize() {
    TypedArray dateTextAppearance = mDelegator->getContext()->obtainStyledAttributes(
            mDateTextAppearanceResId, R.styleable.TextAppearance);
    mDateTextSize = dateTextAppearance.getDimensionPixelSize(
            R.styleable.TextAppearance_textSize, DEFAULT_DATE_TEXT_SIZE);
    dateTextAppearance.recycle();
}

void CalendarViewLegacyDelegate::invalidateAllWeekViews() {
    const int childCount = mListView->getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* view = mListView->getChildAt(i);
        view->invalidate();
    }
}

/**
 * Gets a calendar for locale bootstrapped with the value of a given calendar.
 *
 * @param oldCalendar The old calendar.
 * @param locale The locale.
 */
Calendar CalendarViewLegacyDelegate::getCalendarForLocale(Calendar& oldCalendar, Locale locale) {
    /*if (oldCalendar == null) {
        return Calendar.getInstance(locale);
    } else */{
        const long currentTimeMillis = oldCalendar.getTimeInMillis();
        Calendar newCalendar;// = Calendar.getInstance(locale);
        newCalendar.setTimeInMillis(currentTimeMillis);
        return newCalendar;
    }
}

/**
 * @return True if the <code>firstDate</code> is the same as the <code>
 * secondDate</code>.
 */
bool CalendarViewLegacyDelegate::isSameDate(Calendar& firstDate, Calendar& secondDate) {
    return (firstDate.get(Calendar::DAY_OF_YEAR) == secondDate.get(Calendar::DAY_OF_YEAR)
            && firstDate.get(Calendar::YEAR) == secondDate.get(Calendar::YEAR));
}

/**
 * Creates a new adapter if necessary and sets up its parameters.
 */
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

/**
 * Sets up the strings to be used by the header.
 */
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

/**
 * Sets all the required fields for the list view.
 */
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

/**
 * This moves to the specified time in the view. If the time is not already
 * in range it will move the list so that the first of the month containing
 * the time is at the top of the view. If the new time is already in view
 * the list will not be scrolled unless forceScroll is true. This time may
 * optionally be highlighted as selected as well.
 *
 * @param date The time to move to.
 * @param animate Whether to scroll to the given time or just redraw at the
 *            new location.
 * @param setSelected Whether to set the given time as selected.
 * @param forceScroll Whether to recenter even if the time is already
 *            visible.
 *
 * @throws IllegalArgumentException if the provided date is before the
 *         range start or after the range end.
 */
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

/**
 * Called when a <code>view</code> transitions to a new <code>scrollState
 * </code>.
 */
void CalendarViewLegacyDelegate::onScrollStateChanged(AbsListView& view, int scrollState) {
    mScrollStateChangedRunnable.doScrollStateChange(view, scrollState);
}

/**
 * Updates the title and selected month if the <code>view</code> has moved to a new
 * month.
 */
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

/**
 * Sets the month displayed at the top of this view based on time. Override
 * to add custom events when the title is changed.
 *
 * @param calendar A day in the new focus month.
 */
void CalendarViewLegacyDelegate::setMonthDisplayed(Calendar& calendar) {
    mCurrentMonthDisplayed = calendar.get(Calendar::MONTH);
    mAdapter->setFocusMonth(mCurrentMonthDisplayed);
    //const int flags = DateUtils.FORMAT_SHOW_DATE | DateUtils.FORMAT_NO_MONTH_DAY| DateUtils.FORMAT_SHOW_YEAR;
    const long millis = calendar.getTimeInMillis();
    std::string newMonthName = std::to_string(calendar.get(Calendar::MONTH));;//DateUtils.formatDateRange(mContext, millis, millis, flags);
    mMonthName->setText(newMonthName);
    mMonthName->invalidate();
}

/**
 * @return Returns the number of weeks between the current <code>date</code>
 *         and the <code>mMinDate</code>.
 */
int CalendarViewLegacyDelegate::getWeeksSinceMinDate(Calendar& date) {
    if (date.before(mMinDate)) {
        FATAL("fromDate:  + mMinDate.getTime()  does not precede toDate: + date.getTime()");
    }
    long endTimeMillis = date.getTimeInMillis()
            + date.getTimeZone().getOffset(date.getTimeInMillis());
    long startTimeMillis = mMinDate.getTimeInMillis()
            + mMinDate.getTimeZone().getOffset(mMinDate.getTimeInMillis());
    long dayOffsetMillis = (mMinDate.get(Calendar::DAY_OF_WEEK) - mFirstDayOfWeek)
            * MILLIS_IN_DAY;
    return (int) ((endTimeMillis - startTimeMillis + dayOffsetMillis) / MILLIS_IN_WEEK);
}
#if 0
/**
 * Command responsible for acting upon scroll state changes.
 */
private class ScrollStateRunnable implements Runnable {
    private AbsListView mView;

    private int mNewState;

    /**
     * Sets up the runnable with a short delay in case the scroll state
     * immediately changes again.
     *
     * @param view The list view that changed state
     * @param scrollState The new state it changed to
     */
    public void doScrollStateChange(AbsListView view, int scrollState) {
        mView = view;
        mNewState = scrollState;
        mDelegator->removeCallbacks(this);
        mDelegator->postDelayed(this, SCROLL_CHANGE_DELAY);
    }

    public void run() {
        mCurrentScrollState = mNewState;
        // Fix the position after a scroll or a fling ends
        if (mNewState == AbsListView.OnScrollListener.SCROLL_STATE_IDLE
                && mPreviousScrollState != AbsListView.OnScrollListener.SCROLL_STATE_IDLE) {
            View child = mView.getChildAt(0);
            if (child == null) {
                // The view is no longer visible, just return
                return;
            }
            int dist = child.getBottom() - mListScrollTopOffset;
            if (dist > mListScrollTopOffset) {
                if (mIsScrollingUp) {
                    mView.smoothScrollBy(dist - child.getHeight(),
                            ADJUSTMENT_SCROLL_DURATION);
                } else {
                    mView.smoothScrollBy(dist, ADJUSTMENT_SCROLL_DURATION);
                }
            }
        }
        mPreviousScrollState = mNewState;
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////
/**
 * <p>
 * This is a specialized adapter for creating a list of weeks with
 * selectable days. It can be configured to display the week number, start
 * the week on a given day, show a reduced number of days, or display an
 * arbitrary number of weeks at a time.
 * </p>
 */
CalendarViewLegacyDelegate::WeeksAdapter::WeeksAdapter(CalendarViewLegacyDelegate*cv,Context* context)
    :BaseAdapter(){
    mCV = cv;
    mCV->mContext = context;
    GestureDetector::OnGestureListener gs;
    gs.onSingleTapUp=[](MotionEvent&){return true;};
    mGestureDetector = new GestureDetector(context, gs);
    init();
}

    /**
     * Set up the gesture detector and selected time
     */
void CalendarViewLegacyDelegate::WeeksAdapter::init() {
    mSelectedWeek = mCV->getWeeksSinceMinDate(mSelectedDate);
    mTotalWeekCount = mCV->getWeeksSinceMinDate(mCV->mMaxDate);
    if (mCV->mMinDate.get(Calendar::DAY_OF_WEEK) != mCV->mFirstDayOfWeek
            || mCV->mMaxDate.get(Calendar::DAY_OF_WEEK) != mCV->mFirstDayOfWeek) {
        mTotalWeekCount++;
    }
    notifyDataSetChanged();
}

/**
 * Updates the selected day and related parameters.
 *
 * @param selectedDay The time to highlight
 */
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

/**
 * @return The selected day of month.
 */
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
        weekView->setOnTouchListener(std::bind(&WeeksAdapter::onTouch,this,std::placeholders::_1,std::placeholders::_2));
    }

    int selectedWeekDay = (mSelectedWeek == position) ? mSelectedDate.get(
            Calendar::DAY_OF_WEEK) : -1;
    weekView->init(position, selectedWeekDay, mFocusedMonth);

    return weekView;
}

/**
 * Changes which month is in focus and updates the view.
 *
 * @param month The month to show as in focus [0-11]
 */
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

/**
 * Maintains the same hour/min/sec but moves the day to the tapped day.
 *
 * @param day The day that was tapped
 */
void CalendarViewLegacyDelegate::WeeksAdapter::onDateTapped(Calendar& day) {
    setSelectedDay(day);
    mCV->setMonthDisplayed(day);
}

///////////////////////////////////////////////////////////////////////////////////////////

/**
 * <p>
 * This is a dynamic view for drawing a single week. It can be configured to
 * display the week number, start the week on a given day, or show a reduced
 * number of days. It is intended for use as a single view within a
 * ListView. See {@link WeeksAdapter} for usage.
 * </p>
 */

CalendarViewLegacyDelegate::WeekView::WeekView(CalendarViewLegacyDelegate*cv,Context* context,const AttributeSet&attrs)
    :View(context,attrs),mCV(cv){
    // Sets up any standard paints that will be used
    initializePaints();
}

/**
 * Initializes this week view.
 *
 * @param weekNumber The number of the week this view represents. The
 *            week number is a zero based index of the weeks since
 *            {@link android.widget.CalendarView#getMinDate()}.
 * @param selectedWeekDay The selected day of the week from 0 to 6, -1 if no
 *            selected day.
 * @param focusedMonth The month that is currently in focus i.e.
 *            highlighted.
 */
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

/**
 * Initialize the paint instances.
 */
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

    const bool IsLayoutRtl = isLayoutRtl();

    if (IsLayoutRtl) {
        recLeft = 0;
        recRight = mSelectedLeft - 2;
    } else {
        recLeft = mCV->mShowWeekNumber ? mWidth / mNumCells : 0;
        recRight = mSelectedLeft - 2;
    }
    canvas.rectangle(recLeft,recTop,recRight-recLeft,recBottom-recTop);

    if (IsLayoutRtl) {
        recLeft = mSelectedRight + 3;
        recRight = mCV->mShowWeekNumber ? mWidth - mWidth / mNumCells : mWidth;
    } else {
        recLeft = mSelectedRight + 3;
        recRight = mWidth;
    }
    canvas.rectangle(recLeft,recTop,recRight-recLeft,recBottom-recTop);//, mDrawPaint);
}

/**
 * Draws the week and month day numbers for this week.
 *
 * @param canvas The canvas to draw on
 */
void CalendarViewLegacyDelegate::WeekView::drawWeekNumbersAndDates(Canvas& canvas) {
    //const float textHeight = mDrawPaint.getTextSize();
    //const int y = (int) ((mHeight + textHeight) / 2) - mCV->mWeekSeparatorLineWidth;
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

/**
 * Draws a horizontal line for separating the weeks.
 *
 * @param canvas The canvas to draw on.
 */
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

/**
 * Draws the selected date bars if this week has a selected day.
 *
 * @param canvas The canvas to draw on
 */
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

/**
 * This calculates the positions for the selected day lines.
 */
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

