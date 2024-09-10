#ifndef __CALENDAR_VIEW_LEGACY_DELEGATE_H__
#define __CALENDAR_VIEW_LEGACY_DELEGATE_H__
#include <core/calendar.h>
#include <view/gesturedetector.h>
#include <widget/textview.h>
#include <widget/abslistview.h>
#include <widget/calendarview.h>

namespace cdroid{
class TextView;
class ListView;
class CalendarViewLegacyDelegate:public CalendarView::AbstractCalendarViewDelegate {
private:
    /**
     * Default value whether to show week number.
     */
    static constexpr bool DEFAULT_SHOW_WEEK_NUMBER = true;

    /**
     * The number of milliseconds in a day.e
     */
    static constexpr long MILLIS_IN_DAY = 86400000L;

    /**
     * The number of day in a week.
     */
    static constexpr int DAYS_PER_WEEK = 7;

    /**
     * The number of milliseconds in a week.
     */
    static constexpr long MILLIS_IN_WEEK = DAYS_PER_WEEK * MILLIS_IN_DAY;

    /**
     * Affects when the month selection will change while scrolling upe
     */
    static constexpr int SCROLL_HYST_WEEKS = 2;

    /**
     * How long the GoTo fling animation should last.
     */
    static constexpr int GOTO_SCROLL_DURATION = 1000;

    /**
     * The duration of the adjustment upon a user scroll in milliseconds.
     */
    static constexpr int ADJUSTMENT_SCROLL_DURATION = 500;

    /**
     * How long to wait after receiving an onScrollStateChanged notification
     * before acting on it.
     */
    static constexpr int SCROLL_CHANGE_DELAY = 40;

    static constexpr int DEFAULT_SHOWN_WEEK_COUNT = 6;

    static constexpr int DEFAULT_DATE_TEXT_SIZE = 14;

    static constexpr int UNSCALED_SELECTED_DATE_VERTICAL_BAR_WIDTH = 6;

    static constexpr int UNSCALED_WEEK_MIN_VISIBLE_HEIGHT = 12;

    static constexpr int UNSCALED_LIST_SCROLL_TOP_OFFSET = 2;

    static constexpr int UNSCALED_BOTTOM_BUFFER = 20;

    static constexpr int UNSCALED_WEEK_SEPARATOR_LINE_WIDTH = 1;

    static constexpr int DEFAULT_WEEK_DAY_TEXT_APPEARANCE_RES_ID = -1;
private:
    class WeeksAdapter;
    class WeekView;
    int mWeekSeparatorLineWidth;

    int mDateTextSize;

    Drawable* mSelectedDateVerticalBar;

    int mSelectedDateVerticalBarWidth;

    int mSelectedWeekBackgroundColor;

    int mFocusedMonthDateColor;

    int mUnfocusedMonthDateColor;

    int mWeekSeparatorLineColor;

    int mWeekNumberColor;

    std::string mWeekDayTextAppearanceResId;

    std::string mDateTextAppearanceResId;

    /**
     * The top offset of the weeks list.
     */
    int mListScrollTopOffset = 2;

    /**
     * The visible height of a week view.
     */
    int mWeekMinVisibleHeight = 12;

    /**
     * The visible height of a week view.
     */
    int mBottomBuffer = 20;

    /**
     * The number of shown weeks.
     */
    int mShownWeekCount;

    /**
     * Flag whether to show the week number.
     */
    bool mShowWeekNumber;

    /**
     * The number of day per week to be shown.
     */
    int mDaysPerWeek = 7;

    /**
     * The friction of the week list while flinging.
     */
    float mFriction = .05f;

    /**
     * Scale for adjusting velocity of the week list while flinging.
     */
    float mVelocityScale = 0.333f;

    /**
     * The adapter for the weeks list.
     */
    WeeksAdapter* mAdapter;

    /**
     * The weeks list.
     */
    ListView* mListView;

    /**
     * The name of the month to display.
     */
    TextView* mMonthName;

    /**
     * The header with week day names.
     */
    ViewGroup* mDayNamesHeader;

    /**
     * Cached abbreviations for day of week names.
     */
    std::vector<std::string> mDayNamesShort;

    /**
     * Cached full-length day of week names.
     */
    std::vector<std::string> mDayNamesLong;

    /**
     * The first day of the week.
     */
    int mFirstDayOfWeek;

    /**
     * Which month should be displayed/highlighted [0-11].
     */
    int mCurrentMonthDisplayed = -1;

    /**
     * Used for tracking during a scroll.
     */
    long mPreviousScrollPosition;

    /**
     * Used for tracking which direction the view is scrolling.
     */
    bool mIsScrollingUp = false;

    /**
     * The previous scroll state of the weeks ListView.
     */
    int mPreviousScrollState = AbsListView::OnScrollListener::SCROLL_STATE_IDLE;

    /**
     * The current scroll state of the weeks ListView.
     */
    int mCurrentScrollState = AbsListView::OnScrollListener::SCROLL_STATE_IDLE;

    /**
     * Listener for changes in the selected day.
     */
    CalendarView::OnDateChangeListener mOnDateChangeListener;

    /**
     * Command for adjusting the position after a scroll/fling.
     */
    //ScrollStateRunnable mScrollStateChangedRunnable = new ScrollStateRunnable();

    /**
     * Temporary instance to avoid multiple instantiations.
     */
    Calendar mTempDate;

    /**
     * The first day of the focused month.
     */
    Calendar mFirstDayOfMonth;

    /**
     * The start date of the range supported by this picker.
     */
    Calendar mMinDate;

    /**
     * The end date of the range supported by this picker.
     */
    Calendar mMaxDate;
public:
    CalendarViewLegacyDelegate(CalendarView* delegator, Context* context,const AttributeSet& attrs);

    void setShownWeekCount(int count) override;
    int getShownWeekCount() override;

    void setSelectedWeekBackgroundColor(int color) override;

    int getSelectedWeekBackgroundColor() override;

    void setFocusedMonthDateColor(int color) override;

    int getFocusedMonthDateColor() override;

    void setUnfocusedMonthDateColor(int color) override;

    int getUnfocusedMonthDateColor() override;

    void setWeekNumberColor(int color) override;

    int getWeekNumberColor() override;

    void setWeekSeparatorLineColor(int color) override;

    int getWeekSeparatorLineColor() override;

    void setSelectedDateVerticalBar(const std::string& resourceId) override;

    void setSelectedDateVerticalBar(Drawable* drawable) override;

    Drawable* getSelectedDateVerticalBar() override;

    void setWeekDayTextAppearance(const std::string& resourceId) override;

    const std::string getWeekDayTextAppearance() override;

    void setDateTextAppearance(const std::string& resourceId) override;

    const std::string getDateTextAppearance() override;

    void setMinDate(long minDate) override;

    long getMinDate() override;

    void setMaxDate(long maxDate) override;

    long getMaxDate();

    void setShowWeekNumber(bool showWeekNumber) override;

    bool getShowWeekNumber() override;

    void setFirstDayOfWeek(int firstDayOfWeek) override;

    int getFirstDayOfWeek() override;

    void setDate(long date) override;

    void setDate(long date, bool animate, bool center)override;

    long getDate() override;

    void setOnDateChangeListener(const CalendarView::OnDateChangeListener& listener) override;

    bool getBoundsForDate(long date, Rect& outBounds) override;

    void onConfigurationChanged(int newConfig) override;
private:
    /**
     * Sets the current locale.
     *
     * @param locale The current locale.
     */
    //protected void setCurrentLocale(Locale locale)override;
    void updateDateTextSize();

    /**
     * Invalidates all week views.
     */
    void invalidateAllWeekViews();

    /**
     * Gets a calendar for locale bootstrapped with the value of a given calendar.
     *
     * @param oldCalendar The old calendar.
     * @param locale The locale.
     */
    static Calendar getCalendarForLocale(Calendar& oldCalendar, Locale locale);

    /**
     * @return True if the <code>firstDate</code> is the same as the <code>
     * secondDate</code>.
     */
    static bool isSameDate(Calendar& firstDate, Calendar& secondDate);

    /**
     * Creates a new adapter if necessary and sets up its parameters.
     */
    void setUpAdapter(); 

    /**
     * Sets up the strings to be used by the header.
     */
    void setUpHeader();

    /**
     * Sets all the required fields for the list view.
     */
    void setUpListView();

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
    void goTo(Calendar& date, bool animate, bool setSelected,bool forceScroll);

    /**
     * Called when a <code>view</code> transitions to a new <code>scrollState
     * </code>.
     */
    void onScrollStateChanged(AbsListView& view, int scrollState);

    /**
     * Updates the title and selected month if the <code>view</code> has moved to a new
     * month.
     */
    void onScroll(AbsListView& view, int firstVisibleItem, int visibleItemCount,int totalItemCount);

    /**
     * Sets the month displayed at the top of this view based on time. Override
     * to add custom events when the title is changed.
     *
     * @param calendar A day in the new focus month.
     */
    void setMonthDisplayed(Calendar& calendar);

    /**
     * @return Returns the number of weeks between the current <code>date</code>
     *         and the <code>mMinDate</code>.
     */
    int getWeeksSinceMinDate(Calendar& date);

};/*CalendarViewLegacyDelegate*/


/**
 * <p>
 * This is a specialized adapter for creating a list of weeks with
 * selectable days. It can be configured to display the week number, start
 * the week on a given day, show a reduced number of days, or display an
 * arbitrary number of weeks at a time.
 * </p>
 */
class CalendarViewLegacyDelegate::WeeksAdapter:public BaseAdapter{// implements View.OnTouchListener {
private:
    int mSelectedWeek;
    int mFocusedMonth;
    int mTotalWeekCount;
    GestureDetector* mGestureDetector;
    Calendar mSelectedDate;
    CalendarViewLegacyDelegate*mCV;
    friend CalendarViewLegacyDelegate;
private:
    /**
     * Set up the gesture detector and selected time
     */
    void init();
    /**
     * Maintains the same hour/min/sec but moves the day to the tapped day.
     *
     * @param day The day that was tapped
     */
    void onDateTapped(Calendar& day);
public:
    WeeksAdapter(CalendarViewLegacyDelegate*,Context* context);

    /**
     * Updates the selected day and related parameters.
     *
     * @param selectedDay The time to highlight
     */
    void setSelectedDay(Calendar& selectedDay);

    /**
     * @return The selected day of month.
     */
    Calendar getSelectedDay();

    int getCount() const override;

    void* getItem(int position) const override;

    long getItemId(int position) const override;

    View* getView(int position, View* convertView, ViewGroup* parent)override;
    /**
     * Changes which month is in focus and updates the view.
     *
     * @param month The month to show as in focus [0-11]
     */
    void setFocusMonth(int month);

    bool onTouch(View& v, MotionEvent& event);
};

/**
 * <p>
 * This is a dynamic view for drawing a single week. It can be configured to
 * display the week number, start the week on a given day, or show a reduced
 * number of days. It is intended for use as a single view within a
 * ListView. See {@link WeeksAdapter} for usage.
 * </p>
 */
class CalendarViewLegacyDelegate::WeekView:public View {
private:
    CalendarViewLegacyDelegate*mCV;
    friend CalendarViewLegacyDelegate;
    // Cache the number strings so we don't have to recompute them each time
    std::vector<std::string> mDayNumbers;

    // Quick lookup for checking which days are in the focus month
    std::vector<bool> mFocusDay;

    // Whether this view has a focused day.
    bool mHasFocusedDay;

    // Whether this view has only focused days.
    bool mHasUnfocusedDay;

    // The first day displayed by this item
    Calendar mFirstDay;

    // The month of the first day in this week
    int mMonthOfFirstWeekDay = -1;

    // The month of the last day in this week
    int mLastWeekDayMonth = -1;

    // The position of this week, equivalent to weeks since the week of Jan
    // 1st, 1900
    int mWeek = -1;

    // Quick reference to the width of this view, matches parent
    int mWidth;

    // The height this view should draw at in pixels, set by height param
    int mHeight;

    // If this view contains the selected day
    bool mHasSelectedDay = false;

    // Which day is selected [0-6] or -1 if no day is selected
    int mSelectedDay = -1;

    // The number of days + a spot for week number if it is displayed
    int mNumCells;

    // The left edge of the selected day
    int mSelectedLeft = -1;

    // The right edge of the selected day
    int mSelectedRight = -1;
private:
    /**
     * Initialize the paint instances.
     */
    void initializePaints();
    /**
     * This draws the selection highlight if a day is selected in this week.
     *
     * @param canvas The canvas to draw on
     */
    void drawBackground(Canvas& canvas);

    /**
     * Draws the week and month day numbers for this week.
     *
     * @param canvas The canvas to draw on
     */
    void drawWeekNumbersAndDates(Canvas& canvas);

    /**
     * Draws a horizontal line for separating the weeks.
     *
     * @param canvas The canvas to draw on.
     */
    void drawWeekSeparators(Canvas& canvas);

    /**
     * Draws the selected date bars if this week has a selected day.
     *
     * @param canvas The canvas to draw on
     */
    void drawSelectedDateVerticalBars(Canvas& canvas);
    /**
     * This calculates the positions for the selected day lines.
     */
    void updateSelectionPositions();
protected:
    void onDraw(Canvas& canvas)override;
    void onSizeChanged(int w, int h, int oldw, int oldh)override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
public:
    WeekView(CalendarViewLegacyDelegate*,Context* context,const AttributeSet&);

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
    void init(int weekNumber, int selectedWeekDay, int focusedMonth);

    /**
     * Returns the month of the first day in this week.
     *
     * @return The month the first day of this view is in.
     */
    int getMonthOfFirstWeekDay();

    /**
     * Returns the month of the last day in this week
     *
     * @return The month the last day of this view is in
     */
    int getMonthOfLastWeekDay();

    /**
     * Returns the first day in this view.
     *
     * @return The first day in the view.
     */
    Calendar& getFirstDay();

    /**
     * Calculates the day that the given x position is in, accounting for
     * week number.
     *
     * @param x The x position of the touch event.
     * @return True if a day was found for the given location.
     */
    bool getDayFromLocation(float x, Calendar& outCalendar);

    bool getBoundsForDate(Calendar& date, Rect& outBounds);
};

}/*endof namspace*/

#endif /*__CALENDAR_VIEW_LEGACY_DELEGATE_H__*/

