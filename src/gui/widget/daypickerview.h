/*************************************************************************
	> File Name: daypickerview.h
	> Author: 
	> Mail: 
	> Created Time: Sun 23 May 2021 04:50:41 AM UTC
 ************************************************************************/

#ifndef __DAY_PICKER_VIEW_H__
#define __DAY_PICKER_VIEW_H__
#include <widget/viewgroup.h>
#include <widget/imagebutton.h>
#include <widget/adapter.h>
#include <widget/simplemonthview.h>
#include <widget/viewpager.h>
#include <sparsearray.h>
#include <calendar.h>

namespace cdroid{

class DayPickerView:public ViewGroup{
public:
    DECLARE_UIEVENT(void,OnDaySelectedListener,DayPickerView&,Calendar&);
private:
    static constexpr int DEFAULT_LAYOUT =0;// R.layout.day_picker_content_material;
    static constexpr int DEFAULT_START_YEAR = 1900;
    static constexpr int DEFAULT_END_YEAR   = 2100;
    
    Calendar mSelectedDay;
    Calendar mMinDate ;
    Calendar mMaxDate ;
    /** Temporary calendar used for date calculations. */
    Calendar mTempCalendar;

    ViewPager* mViewPager;
    ImageButton* mPrevButton;
    ImageButton* mNextButton;

    class DayPickerPagerAdapter* mAdapter;
    OnDaySelectedListener mOnDaySelectedListener;
private:
    void updateButtonVisibility(int position);
    void setDate(long timeInMillis, bool animate, bool setSelected);
    int getDiffMonths(Calendar& start, Calendar& end);
    int getPositionFromDay(long timeInMillis);
    Calendar getTempCalendarForTime(long timeInMillis);
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int left, int top, int width, int height)override;

public:
    DayPickerView(Context* context,const AttributeSet&atts);
    void setDate(long timeInMillis);
    void setDate(long timeInMillis, bool animate);
    long getDate();
    bool getBoundsForDate(long timeInMillis,RECT& outBounds);
    void setFirstDayOfWeek(int firstDayOfWeek);
    int getFirstDayOfWeek();
    void setMinDate(long timeInMillis);
    long getMinDate();
    void setMaxDate(long timeInMillis);
    long getMaxDate();
    void onRangeChanged();
    void setOnDaySelectedListener(OnDaySelectedListener listener);
    int getMostVisiblePosition();
    void setPosition(int position);

};


class DayPickerPagerAdapter:public PagerAdapter{
public:
    struct ViewHolder{
        int position;
        View* container;
        SimpleMonthView* calendar;
    };
    DECLARE_UIEVENT(void,OnDaySelectedListener,DayPickerPagerAdapter&,Calendar& day);
private:
    static constexpr int MONTHS_IN_YEAR = 12;
    int mCount;
    int mFirstDayOfWeek;
    OnDaySelectedListener mOnDaySelectedListener;
    SparseArray<ViewHolder*,nullptr> mItems;
    Calendar mMinDate ;
    Calendar mMaxDate ;
    Calendar* mSelectedDay;
    
    ColorStateList* mCalendarTextColor;
    ColorStateList* mDaySelectorColor;
    ColorStateList* mDayHighlightColor;

    int getMonthForPosition(int position);
    int getYearForPosition(int position);
    int getPositionForDay(Calendar* day);
public:
    void setRange(Calendar&min,Calendar&max);
    void setSelectedDay(Calendar&);
    int getFirstDayOfWeek();
    void setFirstDayOfWeek(int);
    bool getBoundsForDate(Calendar&,RECT&);
    bool isViewFromObject(View* view, void* object)override;
    void* instantiateItem(ViewGroup*container, int position)override;
    void destroyItem(ViewGroup* container, int position, void* object);
    int getItemPosition(void* object)override;
    std::string getPageTitle(int position)override;
    void setCalendarTextColor(ColorStateList* calendarTextColor) ;
    void setDaySelectorColor(ColorStateList* selectorColor);
    void setOnDaySelectedListener(OnDaySelectedListener listener);
};
}//namespace

#endif
