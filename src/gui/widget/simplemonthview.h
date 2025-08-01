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
#ifndef __SIMPLE_MONTHVIEW_H__
#define __SIMPLE_MONTHVIEW_H__
#include <view/view.h>
#include <core/calendar.h>
#include <core/typeface.h>
#include <widget/explorebytouchhelper.h>
namespace cdroid{

class SimpleMonthView:public View{
public:
    DECLARE_UIEVENT(void,OnDayClickListener,SimpleMonthView& view, Calendar& day);
private:
    static constexpr int DAYS_IN_WEEK =7;
    static constexpr int MAX_WEEKS_IN_MONTH = 6;
    static constexpr int DEFAULT_SELECTED_DAY=-1;
    static constexpr int DEFAULT_WEEK_START=Calendar::SUNDAY;
    static constexpr int SELECTED_HIGHLIGHT_ALPHA = 0xB0;
    class MonthViewTouchHelper;

    Calendar mCalendar;
    std::string mDayOfWeekLabels[7];
    Typeface*mDayTypeface;
    Typeface*mMonthTypeface;
    Typeface*mDayOfWeekTypeface;
    MonthViewTouchHelper*mTouchHelper;
    int mDayTextSize;
    int mMonthTextSize;
    int mDayOfWeekTextSize;

    const ColorStateList* mDayTextColor;
    const ColorStateList* mMonthTextColor;
    const ColorStateList* mDayOfWeekTextColor;
    const ColorStateList* mDaySelectorColor;
    int mDesiredMonthHeight;
    int mDesiredDayOfWeekHeight;
    int mDesiredDayHeight;
    int mDesiredCellWidth;
    int mDesiredDaySelectorRadius;

    std::string mMonthYearLabel;
    int mMonth;
    int mYear;
    // Dimensions as laid out.
    int mMonthHeight;
    int mDayOfWeekHeight;
    int mDayHeight;
    int mCellWidth;
    int mDaySelectorRadius;
    int mPaddedWidth;
    int mPaddedHeight;
    /** The day of month for the selected day, or -1 if no day is selected. */
    int mActivatedDay = -1;

    /* The day of month for today, or -1 if the today is not in the current month.*/
    int mToday = DEFAULT_SELECTED_DAY;

    /** The first day of the week (ex. Calendar.SUNDAY) indexed from one. */
    int mWeekStart = DEFAULT_WEEK_START;

    /** The number of days (ex. 28) in the current month. */
    int mDaysInMonth;

    /*The day of week (ex. Calendar.SUNDAY) for the first day of the current month. */
    int mDayOfWeekStart;

    /** The day of month for the first (inclusive) enabled day. */
    int mEnabledDayStart = 1;

    /** The day of month for the last (inclusive) enabled day. */
    int mEnabledDayEnd = 31;

    /** Optional listener for handling day click actions. */
    OnDayClickListener mOnDayClickListener;

    int mHighlightedDay = -1;
    int mPreviouslyHighlightedDay = -1;
    bool mIsTouchHighlighted = false;
private:
    void initMonthView();
    void updateMonthYearLabel();
    void updateDayOfWeekLabels();
    ColorStateList*applyTextAppearance(Typeface*&face,int& txtSize,const std::string& resId);
    bool moveOneDay(bool positive);
    int findClosestRow(const Rect* previouslyFocusedRect);
    int findClosestColumn(const Rect*previouslyFocusedRect);
    void ensureFocusedDay();
    bool isFirstDayOfWeek(int day);
    bool isLastDayOfWeek(int day);

    void drawMonth(Canvas& canvas);
    void drawDaysOfWeek(Canvas& canvas);
    void drawDays(Canvas& canvas);

    bool isDayEnabled(int day)const;
    bool isValidDayOfMonth(int day)const;
    static bool isValidDayOfWeek(int day);
    static bool isValidMonth(int month);
    static int getDaysInMonth(int month, int year);

    bool sameDay(int day, Calendar& today);
    int findDayOffset()const;
    int getDayAtLocation(int x, int y);
    bool onDayClicked(int day);
protected:
    void onFocusChanged(bool gainFocus,int direction,Rect* previouslyFocusedRect)override;
    void onFocusLost()override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int left, int top, int width, int height)override;
    void onDraw(Canvas& canvas)override;
public:
    SimpleMonthView(int,int);
    SimpleMonthView(Context*,const AttributeSet&atts);
    ~SimpleMonthView()override;
    int getMonthHeight()const;
    int getCellWidth()const;
     
    void setMonthTextAppearance(const std::string& resId);
    void setDayOfWeekTextAppearance(const std::string& resId);
    void setDayTextAppearance(const std::string& resId);

    void setMonthTextColor(const ColorStateList* monthTextColor);
    void setDayOfWeekTextColor(const ColorStateList* dayOfWeekTextColor);
    void setDayTextColor(const ColorStateList* dayTextColor);
    void setDaySelectorColor(const ColorStateList* dayBackgroundColor);
    void setDayHighlightColor(const ColorStateList* dayHighlightColor);
    void setOnDayClickListener(OnDayClickListener listener);
    bool onTouchEvent(MotionEvent& event)override;
    bool onKeyDown(int keyCode, KeyEvent& event)override;
    void getFocusedRect(Rect& r)override;
    const std::string getMonthYearLabel();
    void setSelectedDay(int dayOfMonth);
    void setFirstDayOfWeek(int weekStart);
    void setMonthParams(int selectedDay,int month,int year,int weekStart,int enabledDayStart, int enabledDayEnd);
    bool getBoundsForDay(int id,Rect&outBounds);
    PointerIcon*onResolvePointerIcon(MotionEvent& event, int pointerIndex)override;
};

 /**
  * Provides a virtual view hierarchy for interfacing with an accessibility
  * service.
  */
class SimpleMonthView::MonthViewTouchHelper:public ExploreByTouchHelper {
private:
    static const std::string DATE_FORMAT;
    SimpleMonthView*mSMV;
    Rect mTempRect;
    //final Calendar mTempCalendar = Calendar.getInstance();

    /**
     * Generates a description for a given virtual view.
     *
     * @param id the day to generate a description for
     * @return a description of the virtual view
     */
    std::string getDayDescription(int id);

    /**
     * Generates displayed text for a given virtual view.
     *
     * @param id the day to generate text for
     * @return the visible text of the virtual view
     */
    std::string getDayText(int id);
protected:
    int getVirtualViewAt(float x, float y)override;
    void getVisibleVirtualViews(std::vector<int>& virtualViewIds)override;
    void onPopulateEventForVirtualView(int virtualViewId, AccessibilityEvent& event)override;
    void onPopulateNodeForVirtualView(int virtualViewId, AccessibilityNodeInfo& node)override;
    bool onPerformActionForVirtualView(int virtualViewId, int action, Bundle* arguments)override;
public:
    MonthViewTouchHelper(View* host);
};
}//namespace
#endif
