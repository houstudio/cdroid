#ifndef __DATEPICKER_CALENDAR_DELEGATE_H__
#define __DATEPICKER_CALENDAR_DELEGATE_H__
#include <widget/datepicker.h>
#include <widget/daypickerview.h>
#include <widget/viewanimator.h>
#include <widget/yearpickerview.h>
namespace cdroid{
class DatePickerCalendarDelegate:public DatePicker::AbstractDatePickerDelegate {
private:
    static constexpr int USE_LOCALE = 0;

    static constexpr int UNINITIALIZED = -1;
    static constexpr int VIEW_MONTH_DAY = 0;
    static constexpr int VIEW_YEAR = 1;

    static constexpr int DEFAULT_START_YEAR = 1900;
    static constexpr int DEFAULT_END_YEAR = 2100;
    static constexpr int ANIMATION_DURATION = 300;

    //static final int[] ATTRS_TEXT_COLOR = new int[] {com.android.internal.R.attr.textColor};
    //static final int[] ATTRS_DISABLED_ALPHA = new int[] {com.android.internal.R.attr.disabledAlpha};

    DateFormat mYearFormat;
    DateFormat mMonthDayFormat;

    // Top-level container.
    ViewGroup* mContainer;

    // Header views.
    TextView* mHeaderYear;
    TextView* mHeaderMonthDay;

    // Picker views.
    ViewAnimator* mAnimator;
    DayPickerView* mDayPickerView;
    YearPickerView* mYearPickerView;

    int mCurrentView = UNINITIALIZED;
    int mFirstDayOfWeek = USE_LOCALE;
    Calendar mTempDate;
    Calendar mMinDate;
    Calendar mMaxDate;
private:
    ColorStateList* applyLegacyColorFixes(ColorStateList* color);
    int multiplyAlphaComponent(int color, float alphaMod);
    void onYearChanged(YearPickerView& view, int year);
    void onCurrentDateChanged()override;
    void setCurrentView(int viewIndex);
    void setDate(int year, int month, int dayOfMonth);
    void onDateChanged(bool fromUser, bool callbackToClient);
    static int getDaysInMonth(int month, int year);
    void tryVibrate();
protected:
    void onLocaleChanged(Locale locale)override;
public:
    DatePickerCalendarDelegate(DatePicker* delegator, Context* context,const AttributeSet& attrs);

    void init(int year, int month, int dayOfMonth,const DatePicker::OnDateChangedListener& callBack) override;
    void updateDate(int year, int month, int dayOfMonth) override;

    int getYear() override;
    int getMonth()override;
    int getDayOfMonth()override;
    void setMinDate(int64_t minDate)override;
    Calendar getMinDate()override;
    void setMaxDate(int64_t maxDate)override;
    Calendar getMaxDate()override;
    void setFirstDayOfWeek(int firstDayOfWeek)override;
    int getFirstDayOfWeek()override;

    void setEnabled(bool enabled)override;
    bool isEnabled() const override;
    CalendarView* getCalendarView() override;

    void setCalendarViewShown(bool shown) override;
    bool getCalendarViewShown() override;

    void setSpinnersShown(bool shown) override;
    bool getSpinnersShown() override;

    //void onConfigurationChanged(Configuration newConfig)override;

    Parcelable* onSaveInstanceState(Parcelable& superState)override;
    void onRestoreInstanceState(Parcelable& state)override;

    bool dispatchPopulateAccessibilityEvent(AccessibilityEvent& event) override;

    std::string getAccessibilityClassName()const override;
};
}/*endof namespace*/
#endif/*__DATEPICKER_CALENDAR_DELEGATE_H__*/
