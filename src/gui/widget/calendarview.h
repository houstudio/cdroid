#ifndef __CALENDAR_VIEW_H__
#define __CALENDAR_VIEW_H__
#include <widget/framelayout.h>

namespace cdroid{

class CalendarView:public FrameLayout{
public:
    class CalendarViewDelegate;
    class AbstractCalendarViewDelegate;
    DECLARE_UIEVENT(void,OnDateChangeListener,CalendarView& view, int year, int month, int dayOfMonth);
private:
public:
    CalendarView(int w,int h);
    CalendarView(Context*,const AttributeSet&atts);
    void setWeekDayTextAppearance(const std::string&resid);
    const std::string getWeekDayTextAppearance()const;
    void setDateTextAppearance(const std::string&resid);
    const std::string getDateTextAppearance()const;
    void setMinDate(long minDate);/*date from 1970.1.1*/
    long getMinDate()const;
    void setMaxDate(long maxDate);
    long getMaxDate()const;
    void setFirstDayOfWeek(int firstDayOfWeek);
    int getFirstDayOfWeek()const;
    void setOnDateChangeListener(OnDateChangeListener listener);
    void setDate(long date);
    /*get selected Date*/
    long getDate();
    bool getBoundsForDate(long date,Rect& outBounds);
};

class CalendarView::CalendarViewDelegate{
public:
    virtual void setShownWeekCount(int count)=0;
    virtual int getShownWeekCount()=0;

    virtual void setSelectedWeekBackgroundColor(int color)=0;
    virtual int getSelectedWeekBackgroundColor()=0;

    virtual void setFocusedMonthDateColor(int color)=0;
    virtual int getFocusedMonthDateColor()=0;

    virtual void setUnfocusedMonthDateColor(int color)=0;
    virtual int getUnfocusedMonthDateColor()=0;

    virtual void setWeekNumberColor(int color)=0;
    virtual int getWeekNumberColor()=0;

    virtual void setWeekSeparatorLineColor(int color)=0;
    virtual int getWeekSeparatorLineColor()=0;

    virtual void setSelectedDateVerticalBar(const std::string& resourceId)=0;
    virtual void setSelectedDateVerticalBar(Drawable* drawable)=0;
    virtual Drawable* getSelectedDateVerticalBar()=0;

    virtual void setWeekDayTextAppearance(const std::string& resourceId)=0;
    virtual const std::string getWeekDayTextAppearance()=0;

    virtual void setDateTextAppearance(const std::string&resourceId)=0;
    virtual const std::string getDateTextAppearance()=0;

    virtual void setMinDate(long minDate)=0;
    virtual long getMinDate()=0;

    virtual void setMaxDate(long maxDate)=0;
    virtual long getMaxDate()=0;

    virtual void setShowWeekNumber(bool showWeekNumber)=0;
    virtual bool getShowWeekNumber()=0;

    virtual void setFirstDayOfWeek(int firstDayOfWeek)=0;
    virtual int getFirstDayOfWeek()=0;

    virtual void setDate(long date)=0;
    virtual void setDate(long date, bool animate, bool center)=0;
    virtual long getDate()=0;

    virtual bool getBoundsForDate(long date, Rect& outBounds)=0;

    virtual void setOnDateChangeListener(const OnDateChangeListener& listener);

    virtual void onConfigurationChanged(int newConfig)=0;
};

class CalendarView::AbstractCalendarViewDelegate:public CalendarViewDelegate {
protected:
    /** The default minimal date. */
    //static constexpr std::string DEFAULT_MIN_DATE = "01/01/1900";

    /** The default maximal date. */
    //static constexpr std::string DEFAULT_MAX_DATE = "01/01/2100";

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

   int getShownWeekCount() override{
        // Deprecated.
        return 0;
    }

    void setSelectedWeekBackgroundColor(int color) override{
        // Deprecated.
    }

   int getSelectedWeekBackgroundColor() override{
        return 0;
    }

    void setFocusedMonthDateColor(int color) override{
        // Deprecated.
    }

    int getFocusedMonthDateColor() override{
        return 0;
    }

    void setUnfocusedMonthDateColor(int color) override{
        // Deprecated.
    }

    int getUnfocusedMonthDateColor() override{
        return 0;
    }

    void setWeekNumberColor(int color) override{
        // Deprecated.
    }

    int getWeekNumberColor() override{
        // Deprecated.
        return 0;
    }

    void setWeekSeparatorLineColor( int color) {
        // Deprecated.
    }

    int getWeekSeparatorLineColor() override{
        // Deprecated.
        return 0;
    }

    void setSelectedDateVerticalBar(const std::string& resId) override{
        // Deprecated.
    }

    void setSelectedDateVerticalBar(Drawable* drawable) override{
        // Deprecated.
    }

    Drawable* getSelectedDateVerticalBar() {
        // Deprecated.
        return nullptr;
    }

    void setShowWeekNumber(bool showWeekNumber) override{
        // Deprecated.
    }

    bool getShowWeekNumber() override{
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
