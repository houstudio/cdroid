#ifndef __CALENDAR_VIEW_H__
#define __CALENDAR_VIEW_H__
#include <widget/framelayout.h>

namespace cdroid{

class CalendarView:public FrameLayout{
public:
    DECLARE_UIEVENT(void,OnDateChangeListener,CalendarView& view, int year, int month, int dayOfMonth);
private:
public:
    CalendarView(int w,int h);
    CalendarView(Context*,const AttributeSet&atts,const std::string&defstyle=nullptr);
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

}//namespace
#endif
