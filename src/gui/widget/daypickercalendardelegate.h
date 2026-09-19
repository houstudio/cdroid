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

    // AOSP ATTRS_TEXT_COLOR / ATTRS_DISABLED_ALPHA single-attribute sets live in the .cc.

    //DateFormat mYearFormat;       // DEFERRED: CDROID has no DateFormat; header formatted manually.
    //DateFormat mMonthDayFormat;

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
    RefPtr<ColorStateList> applyLegacyColorFixes(RefPtr<ColorStateList> color);
    int multiplyAlphaComponent(int color, float alphaMod);
    void onYearChanged(YearPickerView& view, int year);
    void onCurrentDateChanged();
    void setCurrentView(int viewIndex);
    void setDate(int year, int month, int dayOfMonth);
    void onDateChanged(bool fromUser, bool callbackToClient);
    static int getDaysInMonth(int month, int year);
    void tryVibrate();
protected:
    void onLocaleChanged(const Locale& locale)override;
public:
    DatePickerCalendarDelegate(DatePicker* delegator, Context* context,const AttributeSet* attrs,
        int defStyleAttr,int defStyleRes);

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

    void onConfigurationChanged(Configuration& newConfig)override;

    Parcelable* onSaveInstanceState(Parcelable& superState)override;
    void onRestoreInstanceState(Parcelable& state)override;

    bool dispatchPopulateAccessibilityEvent(AccessibilityEvent& event) override;

    //std::string getAccessibilityClassName()const override; // DEFERRED: DatePicker owns this.
};
}/*endof namespace*/
#endif/*__DATEPICKER_CALENDAR_DELEGATE_H__*/
