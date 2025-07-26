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

    //static constexpr int DEFAULT_WEEK_DAY_TEXT_APPEARANCE_RES_ID = -1;
private:
    class WeeksAdapter;
    class WeekView;
    class ScrollStateRunnable{
    private:
        CalendarViewLegacyDelegate*mDelegate;
        AbsListView* mView;
        int mNewState;
        Runnable mRunnable;
    public:
        ScrollStateRunnable(CalendarViewLegacyDelegate*delegate);
        void doScrollStateChange(AbsListView* view, int scrollState);
        void run();
    };
    friend ScrollStateRunnable;
    CalendarView*mDelegator;
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

    int mListScrollTopOffset = 2;
    int mWeekMinVisibleHeight = 12;
    int mBottomBuffer = 20;
    int mShownWeekCount;

    bool mShowWeekNumber;
    bool mIsScrollingUp = false;

    int mDaysPerWeek = 7;

    float mFriction = .05f;
    float mVelocityScale = 0.333f;

    WeeksAdapter* mAdapter;

    ListView* mListView;

    TextView* mMonthName;

    ViewGroup* mDayNamesHeader;

    std::vector<std::string> mDayNamesShort;

    std::vector<std::string> mDayNamesLong;

    int mFirstDayOfWeek;
    int mCurrentMonthDisplayed = -1;

    long mPreviousScrollPosition;

    int mPreviousScrollState = AbsListView::OnScrollListener::SCROLL_STATE_IDLE;

    int mCurrentScrollState = AbsListView::OnScrollListener::SCROLL_STATE_IDLE;

    CalendarView::OnDateChangeListener mOnDateChangeListener;

    ScrollStateRunnable* mScrollStateChangedRunnable;

    Calendar mTempDate;

    Calendar mFirstDayOfMonth;

    Calendar mMinDate;

    Calendar mMaxDate;
public:
    CalendarViewLegacyDelegate(CalendarView* delegator, Context* context,const AttributeSet& attrs);
    ~CalendarViewLegacyDelegate()override;
    void setShownWeekCount(int count) override;
    int getShownWeekCount() const override;

    void setSelectedWeekBackgroundColor(int color) override;

    int getSelectedWeekBackgroundColor() const override;

    void setFocusedMonthDateColor(int color) override;

    int getFocusedMonthDateColor() const override;

    void setUnfocusedMonthDateColor(int color) override;

    int getUnfocusedMonthDateColor() const override;

    void setWeekNumberColor(int color) override;

    int getWeekNumberColor() const override;

    void setWeekSeparatorLineColor(int color) override;

    int getWeekSeparatorLineColor() const override;

    void setSelectedDateVerticalBar(const std::string& resourceId) override;

    void setSelectedDateVerticalBar(Drawable* drawable) override;

    Drawable* getSelectedDateVerticalBar() const override;

    void setWeekDayTextAppearance(const std::string& resourceId) override;

    std::string getWeekDayTextAppearance() const override;

    void setDateTextAppearance(const std::string& resourceId) override;

    std::string getDateTextAppearance()const override;

    void setMinDate(int64_t minDate) override;

    int64_t getMinDate()override;

    void setMaxDate(int64_t maxDate) override;

    int64_t getMaxDate()override;

    void setShowWeekNumber(bool showWeekNumber) override;

    bool getShowWeekNumber()const override;

    void setFirstDayOfWeek(int firstDayOfWeek) override;

    int getFirstDayOfWeek()const override;

    void setDate(int64_t date) override;

    void setDate(int64_t date, bool animate, bool center)override;

    int64_t getDate()override;

    void setOnDateChangeListener(const CalendarView::OnDateChangeListener& listener) override;

    bool getBoundsForDate(int64_t date, Rect& outBounds)override;

    void onConfigurationChanged(int newConfig) override;
private:
    void updateDateTextSize();

    void invalidateAllWeekViews();

    //static Calendar getCalendarForLocale(Calendar& oldCalendar, Locale locale);

    static bool isSameDate(Calendar& firstDate, Calendar& secondDate);

    void setUpAdapter(); 

    void setUpHeader();

    void setUpListView();

    void goTo(Calendar& date, bool animate, bool setSelected,bool forceScroll);

    void onScrollStateChanged(AbsListView& view, int scrollState);

    void onScroll(AbsListView& view, int firstVisibleItem, int visibleItemCount,int totalItemCount);

    void setMonthDisplayed(Calendar& calendar);

    int getWeeksSinceMinDate(Calendar& date);

};/*CalendarViewLegacyDelegate*/


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
    void init();
    void onDateTapped(Calendar& day);
public:
    WeeksAdapter(CalendarViewLegacyDelegate*,Context* context);

    void setSelectedDay(Calendar& selectedDay);

    Calendar getSelectedDay();

    int getCount() const override;
    void* getItem(int position) const override;
    long getItemId(int position) const override;

    View* getView(int position, View* convertView, ViewGroup* parent)override;
    void setFocusMonth(int month);
    bool onTouch(View& v, MotionEvent& event);
};

class CalendarViewLegacyDelegate::WeekView:public View {
private:
    CalendarViewLegacyDelegate*mCV;
    friend CalendarViewLegacyDelegate;
    std::vector<std::string> mDayNumbers;

    std::vector<bool> mFocusDay;

    bool mHasFocusedDay;
    bool mHasUnfocusedDay;
    bool mHasSelectedDay = false;

    Calendar mFirstDay;

    int mMonthOfFirstWeekDay = -1;
    int mLastWeekDayMonth = -1;
    int mWeek = -1;
    int mWidth;
    int mHeight;
    int mSelectedDay = -1;
    int mNumCells;
    int mSelectedLeft = -1;
    int mSelectedRight = -1;
private:
    void initializePaints();
    void drawBackground(Canvas& canvas);
    void drawWeekNumbersAndDates(Canvas& canvas);
    void drawWeekSeparators(Canvas& canvas);
    void drawSelectedDateVerticalBars(Canvas& canvas);
    void updateSelectionPositions();
protected:
    void onDraw(Canvas& canvas)override;
    void onSizeChanged(int w, int h, int oldw, int oldh)override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
public:
    WeekView(CalendarViewLegacyDelegate*,Context* context,const AttributeSet&);

    void init(int weekNumber, int selectedWeekDay, int focusedMonth);

    int getMonthOfFirstWeekDay();
    int getMonthOfLastWeekDay();

    Calendar& getFirstDay();

    bool getDayFromLocation(float x, Calendar& outCalendar);
    bool getBoundsForDate(Calendar& date, Rect& outBounds);
};

}/*endof namspace*/

#endif /*__CALENDAR_VIEW_LEGACY_DELEGATE_H__*/

