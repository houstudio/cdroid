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
#ifndef __CALENDAR_VIEW_H__
#define __CALENDAR_VIEW_H__
#include <core/calendar.h>
#include <widget/framelayout.h>
namespace cdroid{

class CalendarView:public FrameLayout{
private:
    static constexpr int MODE_HOLO = 0;
    static constexpr int MODE_MATERIAL = 1;
public:
    class CalendarViewDelegate;
    class AbstractCalendarViewDelegate;
    DECLARE_UIEVENT(void,OnDateChangeListener,CalendarView& view, int year, int month, int dayOfMonth);
private:
    CalendarViewDelegate* mDelegate;;
public:
    CalendarView(int w,int h);
    CalendarView(Context*,const AttributeSet&atts);
    ~CalendarView()override;

    void setShownWeekCount(int count);
    int getShownWeekCount()const;

    void setSelectedWeekBackgroundColor(int color);
    int  getSelectedWeekBackgroundColor()const;

    void setFocusedMonthDateColor(int color);
    int  getFocusedMonthDateColor()const;

    void setUnfocusedMonthDateColor(int color);
    int  getUnfocusedMonthDateColor()const;

    void setWeekNumberColor(int color);
    int  getWeekNumberColor()const;
    void setWeekSeparatorLineColor(int color);
    int  getWeekSeparatorLineColor()const;

    void setSelectedDateVerticalBar(const std::string& resourceId);
    void setSelectedDateVerticalBar(Drawable* drawable);
    Drawable* getSelectedDateVerticalBar()const;

    void setWeekDayTextAppearance(const std::string&resid);
    std::string getWeekDayTextAppearance()const;
    void setDateTextAppearance(const std::string&resid);
    std::string getDateTextAppearance()const;

    void setMinDate(int64_t minDate);/*date from 1970.1.1*/
    int64_t getMinDate()const;
    void setMaxDate(int64_t maxDate);
    int64_t getMaxDate()const;

    void setShowWeekNumber(bool showWeekNumber);
    bool getShowWeekNumber()const;

    void setFirstDayOfWeek(int firstDayOfWeek);
    int getFirstDayOfWeek()const;
    void setOnDateChangeListener(const OnDateChangeListener& listener);

    int64_t getDate();
    void setDate(int64_t date);
    void setDate(int64_t date, bool animate, bool center);
    bool getBoundsForDate(int64_t date,Rect& outBounds);

    std::string getAccessibilityClassName() const override;
    static bool parseDate(const std::string& date, Calendar& outDate);
};

class CalendarView::CalendarViewDelegate{
public:
    virtual ~CalendarViewDelegate()=default;
    virtual void setShownWeekCount(int count)=0;
    virtual int getShownWeekCount()const =0;

    virtual void setSelectedWeekBackgroundColor(int color)=0;
    virtual int getSelectedWeekBackgroundColor()const=0;

    virtual void setFocusedMonthDateColor(int color)=0;
    virtual int getFocusedMonthDateColor()const=0;

    virtual void setUnfocusedMonthDateColor(int color)=0;
    virtual int getUnfocusedMonthDateColor()const=0;

    virtual void setWeekNumberColor(int color)=0;
    virtual int getWeekNumberColor()const=0;

    virtual void setWeekSeparatorLineColor(int color)=0;
    virtual int getWeekSeparatorLineColor()const=0;

    virtual void setSelectedDateVerticalBar(const std::string& resourceId)=0;
    virtual void setSelectedDateVerticalBar(Drawable* drawable)=0;
    virtual Drawable* getSelectedDateVerticalBar()const=0;

    virtual void setWeekDayTextAppearance(const std::string& resourceId)=0;
    virtual std::string getWeekDayTextAppearance()const=0;

    virtual void setDateTextAppearance(const std::string&resourceId)=0;
    virtual std::string getDateTextAppearance()const=0;

    virtual void setMinDate(int64_t minDate)=0;
    virtual int64_t getMinDate()=0;

    virtual void setMaxDate(int64_t maxDate)=0;
    virtual int64_t getMaxDate()=0;

    virtual void setShowWeekNumber(bool showWeekNumber)=0;
    virtual bool getShowWeekNumber()const=0;

    virtual void setFirstDayOfWeek(int firstDayOfWeek)=0;
    virtual int getFirstDayOfWeek()const=0;

    virtual void setDate(int64_t date)=0;
    virtual void setDate(int64_t date, bool animate, bool center)=0;
    virtual int64_t getDate()=0;

    virtual bool getBoundsForDate(int64_t date, Rect& outBounds)=0;

    virtual void setOnDateChangeListener(const OnDateChangeListener& listener)=0;

    virtual void onConfigurationChanged(int newConfig)=0;
};

class CalendarView::AbstractCalendarViewDelegate:public CalendarViewDelegate {
protected:
    /** The default minimal date. */
    static constexpr const char* DEFAULT_MIN_DATE = "01/01/1900";

    /** The default maximal date. */
    static constexpr const char* DEFAULT_MAX_DATE = "01/01/2100";

    CalendarView* mDelegator;
    Context* mContext;
    //Locale mCurrentLocale;
public:
    AbstractCalendarViewDelegate(CalendarView* delegator, Context* context) {
        mDelegator = delegator;
        mContext = context;

        // Initialization based on locale
        //setCurrentLocale(Locale.getDefault());
    }

    /*void setCurrentLocale(Locale locale) {
        if (locale.equals(mCurrentLocale)) {
            return;
        }
        mCurrentLocale = locale;
    }*/

    void setShownWeekCount(int count) override{
        // Deprecated.
    }

   int getShownWeekCount() const override{
        // Deprecated.
        return 0;
    }

    void setSelectedWeekBackgroundColor(int color) override{
        // Deprecated.
    }

   int getSelectedWeekBackgroundColor() const override{
        return 0;
    }

    void setFocusedMonthDateColor(int color) override{
        // Deprecated.
    }

    int getFocusedMonthDateColor() const override{
        return 0;
    }

    void setUnfocusedMonthDateColor(int color) override{
        // Deprecated.
    }

    int getUnfocusedMonthDateColor() const override{
        return 0;
    }

    void setWeekNumberColor(int color) override{
        // Deprecated.
    }

    int getWeekNumberColor() const override{
        // Deprecated.
        return 0;
    }

    void setWeekSeparatorLineColor( int color) {
        // Deprecated.
    }

    int getWeekSeparatorLineColor() const override{
        // Deprecated.
        return 0;
    }

    void setSelectedDateVerticalBar(const std::string& resId) override{
        // Deprecated.
    }

    void setSelectedDateVerticalBar(Drawable* drawable) override{
        // Deprecated.
    }

    Drawable* getSelectedDateVerticalBar() const{
        // Deprecated.
        return nullptr;
    }

    void setShowWeekNumber(bool showWeekNumber) override{
        // Deprecated.
    }

    bool getShowWeekNumber() const override{
        // Deprecated.
        return false;
    }

    void onConfigurationChanged(int newConfig) override{
        // Nothing to do here, configuration changes are already propagated
        // by ViewGroup.
    }
};

}//namespace
#endif
