#ifndef __DAY_PICKER_VIEW_H__
#define __DAY_PICKER_VIEW_H__
#include <view/viewgroup.h>
#include <widget/imagebutton.h>
#include <widget/adapter.h>
#include <widget/simplemonthview.h>
#include <widget/viewpager.h>
#include <widget/daypickerpageradapter.h>
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
    void setDayOfWeekTextAppearance(const std::string& resId);
    const std::string getDayOfWeekTextAppearance();
    void  setDayTextAppearance(const std::string& resId);
    const std::string getDayTextAppearance();
    void setDate(long timeInMillis);
    void setDate(long timeInMillis, bool animate);
    long getDate();
    bool getBoundsForDate(long timeInMillis,Rect& outBounds);
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

}//endof namespace

#endif
