#if 0
#include <view/viewgroup.h>
#include <widget/daypickercalendardelegate.h>

namespace cdroid{

DatePickerCalendarDelegate::DatePickerCalendarDelegate(DatePickeri* delegator, Context* context,const AttributeSet& attrs)
  :AbstractDatePickerDelegate(delegator, context){

    //final Locale locale = mCurrentLocale;
    mCurrentDate = Calendar.getInstance(locale);
    mTempDate = Calendar.getInstance(locale);
    mMinDate = Calendar.getInstance(locale);
    mMaxDate = Calendar.getInstance(locale);

    mMinDate.set(DEFAULT_START_YEAR, Calendar::JANUARY, 1);
    mMaxDate.set(DEFAULT_END_YEAR, Calendar::DECEMBER, 31);

    final Resources res = mDelegator.getResources();
    final TypedArray a = mContext->obtainStyledAttributes(attrs,
            R.styleable.DatePicker, defStyleAttr, defStyleRes);
    LayoutInflater inflater = LayoutInflater::from(mContext);
    final int layoutResourceId = a.getResourceId(
            R.styleable.DatePicker_internalLayout, R.layout.date_picker_material);

    // Set up and attach container.
    mContainer = (ViewGroup*) inflater.inflate(layoutResourceId, mDelegator, false);
    mContainer->setSaveFromParentEnabled(false);
    mDelegator->addView(mContainer);

    // Set up header views.
    ViewGroup* header = mContainer->findViewById(R::id::date_picker_header);
    mHeaderYear = header->findViewById(R.id.date_picker_header_year);
    OnClickListener headerClickListener =[this](View&V) {
        tryVibrate();
        switch (v.getId()) {
            case R::id::date_picker_header_year:
                setCurrentView(VIEW_YEAR);
                break;
            case R::id::date_picker_header_date:
                setCurrentView(VIEW_MONTH_DAY);
                break;
        }
    };

    mHeaderYear->setOnClickListener(headerClickListener);
    //mHeaderYear->setAccessibilityDelegate(new ClickActionDelegate(context, R.string.select_year));
    mHeaderYear->setAccessibilityLiveRegion(View::ACCESSIBILITY_LIVE_REGION_POLITE);

    mHeaderMonthDay = header->findViewById(R::id::date_picker_header_date);
    mHeaderMonthDay->setOnClickListener(headerClickListener);
    //mHeaderMonthDay->setAccessibilityDelegate(new ClickActionDelegate(context, R.string.select_day));
    mHeaderMonthDay->setAccessibilityLiveRegion(View::ACCESSIBILITY_LIVE_REGION_POLITE);

    // For the sake of backwards compatibility, attempt to extract the text
    // color from the header month text appearance. If it's set, we'll let
    // that override the "real" header text color.
    ColorStateList* headerTextColor = null;

    const std::string monthHeaderTextAppearance = a.getResourceId(
            R.styleable.DatePicker_headerMonthTextAppearance, 0);
    if (monthHeaderTextAppearance != 0) {
        final TypedArray textAppearance = mContext->obtainStyledAttributes(null,
                ATTRS_TEXT_COLOR, 0, monthHeaderTextAppearance);
        ColorStateList* legacyHeaderTextColor = textAppearance.getColorStateList(0);
        headerTextColor = applyLegacyColorFixes(legacyHeaderTextColor);
    }

    if (headerTextColor == nullptr) {
        headerTextColor = a.getColorStateList("headerTextColor");
    }

    if (headerTextColor != nullptr) {
        mHeaderYear->setTextColor(headerTextColor);
        mHeaderMonthDay->setTextColor(headerTextColor);
    }

    // Set up header background, if available.
    if (a.hasValueOrEmpty("headerBackground")) {
        header->setBackground(a.getDrawable("headerBackground"));
    }

    // Set up picker container.
    mAnimator = mContainer->findViewById(R::id::animator);

    // Set up day picker view.
    mDayPickerView = mAnimator->findViewById(R::id::date_picker_day_picker);
    mDayPickerView->setFirstDayOfWeek(mFirstDayOfWeek);
    mDayPickerView->setMinDate(mMinDate.getTimeInMillis());
    mDayPickerView->setMaxDate(mMaxDate.getTimeInMillis());
    mDayPickerView->setDate(mCurrentDate.getTimeInMillis());
    mDayPickerView->setOnDaySelectedListener([this](DayPickerView& view, Calendar& day){
        mCurrentDate.setTimeInMillis(day.getTimeInMillis());
        onDateChanged(true, true);
    });//mOnDaySelectedListener);

    // Set up year picker view.
    mYearPickerView = mAnimator->findViewById(R.id.date_picker_year_picker);
    mYearPickerView->setRange(mMinDate, mMaxDate);
    mYearPickerView->setYear(mCurrentDate.get(Calendar.YEAR));
    OnYearSelectedListener ysl = [this](YearPickerView& view, int year){
        onYearChanged(view,year);
    };
    mYearPickerView->setOnYearSelectedListener(ysl);

    // Initialize for current locale. This also initializes the date, so no
    // need to call onDateChanged.
    onLocaleChanged(mCurrentLocale);

    setCurrentView(VIEW_MONTH_DAY);
}

ColorStateList* DatePickerCalendarDelegate::applyLegacyColorFixes(ColorStateList* color) {
    if (color == nullptr || color->hasState(R.attr.state_activated)) {
        return color;
    }

    int activatedColor;
    int defaultColor;
    if (color->hasState(R.attr.state_selected)) {
        activatedColor = color->getColorForState(StateSet::get(StateSet::VIEW_STATE_ENABLED | StateSet::VIEW_STATE_SELECTED), 0);
        defaultColor = color->getColorForState(StateSet::get(StateSet::VIEW_STATE_ENABLED), 0);
    } else {
        activatedColor = color->getDefaultColor();

        // Generate a non-activated color using the disabled alpha.
        final TypedArray ta = mContext->obtainStyledAttributes(ATTRS_DISABLED_ALPHA);
        const float disabledAlpha = ta.getFloat(0, 0.30f);
        ta.recycle();
        defaultColor = multiplyAlphaComponent(activatedColor, disabledAlpha);
    }

    if (activatedColor == 0 || defaultColor == 0) {
        // We somehow failed to obtain the colors.
        return nullptr;
    }

    final int[][] stateSet = new int[][] {{ R.attr.state_activated }, {}};
    final int[] colors = new int[] { activatedColor, defaultColor };
    return new ColorStateList(stateSet, colors);
}

int DatePickerCalendarDelegate::multiplyAlphaComponent(int color, float alphaMod) {
    const int srcRgb = color & 0xFFFFFF;
    const int srcAlpha = (color >> 24) & 0xFF;
    const int dstAlpha = (int) (srcAlpha * alphaMod + 0.5f);
    return srcRgb | (dstAlpha << 24);
}

/*static class ClickActionDelegate extends View.AccessibilityDelegate {
    private final AccessibilityNodeInfo.AccessibilityAction mClickAction;

    ClickActionDelegate(Context context, int resId) {
        mClickAction = new AccessibilityNodeInfo.AccessibilityAction(
                AccessibilityNodeInfo.ACTION_CLICK, context.getString(resId));
    }

    @Override
    public void onInitializeAccessibilityNodeInfo(View host, AccessibilityNodeInfo info) {
        super.onInitializeAccessibilityNodeInfo(host, info);

        info.addAction(mClickAction);
    }
}*/

void DatePickerCalendarDelegate::onYearChanged(YearPickerView& view, int year) {
    // If the newly selected month / year does not contain the
    // currently selected day number, change the selected day number
    // to the last day of the selected month or year.
    // e.g. Switching from Mar to Apr when Mar 31 is selected -> Apr 30
    // e.g. Switching from 2012 to 2013 when Feb 29, 2012 is selected -> Feb 28, 2013
    const int day = mCurrentDate.get(Calendar::DAY_OF_MONTH);
    const int month = mCurrentDate.get(Calendar::MONTH);
    const int daysInMonth = getDaysInMonth(month, year);
    if (day > daysInMonth) {
        mCurrentDate.set(Calendar::DAY_OF_MONTH, daysInMonth);
    }

    mCurrentDate.set(Calendar::YEAR, year);
    if (mCurrentDate.compareTo(mMinDate) < 0) {
        mCurrentDate.setTimeInMillis(mMinDate.getTimeInMillis());
    } else if (mCurrentDate.compareTo(mMaxDate) > 0) {
        mCurrentDate.setTimeInMillis(mMaxDate.getTimeInMillis());
    }
    onDateChanged(true, true);

    // Automatically switch to day picker.
    setCurrentView(VIEW_MONTH_DAY);

    // Switch focus back to the year text.
    mHeaderYear->requestFocus();
}

void DatePickerCalendarDelegate::onLocaleChanged(Locale locale) {
    TextView* headerYear = mHeaderYear;
    if (headerYear == nullptr) {
        // Abort, we haven't initialized yet. This method will get called
        // again later after everything has been set up.
        return;
    }

    // Update the date formatter.
    mMonthDayFormat = DateFormat.getInstanceForSkeleton("EMMMd", locale);
    // The use of CAPITALIZATION_FOR_BEGINNING_OF_SENTENCE instead of
    // CAPITALIZATION_FOR_STANDALONE is to address
    // https://unicode-org.atlassian.net/browse/ICU-21631
    // TODO(b/229287642): Switch back to CAPITALIZATION_FOR_STANDALONE
    mMonthDayFormat.setContext(DisplayContext.CAPITALIZATION_FOR_BEGINNING_OF_SENTENCE);
    mYearFormat = DateFormat.getInstanceForSkeleton("y", locale);

    // Update the header text.
    onCurrentDateChanged();
}

void DatePickerCalendarDelegate::onCurrentDateChanged() {
    if (mHeaderYear == nullptr) {
        // Abort, we haven't initialized yet. This method will get called
        // again later after everything has been set up.
        return;
    }

    std::string year = mYearFormat.format(mCurrentDate.getTime());
    mHeaderYear->setText(year);

    std::string monthDay = mMonthDayFormat.format(mCurrentDate.getTime());
    mHeaderMonthDay->setText(monthDay);
}

void DatePickerCalendarDelegate::setCurrentView(int viewIndex) {
    switch (viewIndex) {
    case VIEW_MONTH_DAY:
        mDayPickerView->setDate(mCurrentDate.getTimeInMillis());

        if (mCurrentView != viewIndex) {
            mHeaderMonthDay->setActivated(true);
            mHeaderYear->setActivated(false);
            mAnimator->setDisplayedChild(VIEW_MONTH_DAY);
            mCurrentView = viewIndex;
        }
        break;
    case VIEW_YEAR:
        mYearPickerView->setYear(mCurrentDate.get(Calendar::YEAR));
        mYearPickerView->post(() -> {
            mYearPickerView->requestFocus();
            View* selected = mYearPickerView->getSelectedView();
            if (selected != null) {
                selected->requestFocus();
            }
        });

        if (mCurrentView != viewIndex) {
            mHeaderMonthDay->setActivated(false);
            mHeaderYear->setActivated(true);
            mAnimator->setDisplayedChild(VIEW_YEAR);
            mCurrentView = viewIndex;
        }

        break;
    }
}

void DatePickerCalendarDelegate::init(int year, int month, int dayOfMonth,const DatePicker::OnDateChangedListener& callBack) {
    setDate(year, month, dayOfMonth);
    onDateChanged(false, false);

    mOnDateChangedListener = callBack;
}

void DatePickerCalendarDelegate::updateDate(int year, int month, int dayOfMonth) {
    setDate(year, month, dayOfMonth);
    onDateChanged(false, true);
}

void DatePickerCalendarDelegate::setDate(int year, int month, int dayOfMonth) {
    mCurrentDate.set(Calendar::YEAR, year);
    mCurrentDate.set(Calendar::MONTH, month);
    mCurrentDate.set(Calendar::DAY_OF_MONTH, dayOfMonth);
    resetAutofilledValue();
}

void DatePickerCalendarDelegate::onDateChanged(bool fromUser, bool callbackToClient) {
    const int year = mCurrentDate.get(Calendar::YEAR);

    if (callbackToClient && (mOnDateChangedListener != nullptr || mAutoFillChangeListener != nullptr)) {
        const int monthOfYear = mCurrentDate.get(Calendar::MONTH);
        const int dayOfMonth = mCurrentDate.get(Calendar::DAY_OF_MONTH);
        if (mOnDateChangedListener != nullptr) {
            mOnDateChangedListener/*.onDateChanged*/(*mDelegator, year, monthOfYear, dayOfMonth);
        }
        /*if (mAutoFillChangeListener != nullptr) {
            mAutoFillChangeListener.onDateChanged(mDelegator, year, monthOfYear, dayOfMonth);
        }*/
    }

    mDayPickerView->setDate(mCurrentDate.getTimeInMillis());
    mYearPickerView->setYear(year);

    onCurrentDateChanged();

    if (fromUser) {
        tryVibrate();
    }
}

int DatePickerCalendarDelegate::getYear() {
    return mCurrentDate.get(Calendar::YEAR);
}

int DatePickerCalendarDelegate::getMonth() {
    return mCurrentDate.get(Calendar::MONTH);
}

int DatePickerCalendarDelegate::getDayOfMonth() {
    return mCurrentDate.get(Calendar::DAY_OF_MONTH);
}

void DatePickerCalendarDelegate::setMinDate(int64_t minDate) {
    mTempDate.setTimeInMillis(minDate);
    if (mTempDate.get(Calendar::YEAR) == mMinDate.get(Calendar::YEAR)
            && mTempDate.get(Calendar::DAY_OF_YEAR) == mMinDate.get(Calendar::DAY_OF_YEAR)) {
        // Same day, no-op.
        return;
    }
    if (mCurrentDate.before(mTempDate)) {
        mCurrentDate.setTimeInMillis(minDate);
        onDateChanged(false, true);
    }
    mMinDate.setTimeInMillis(minDate);
    mDayPickerView->setMinDate(minDate);
    mYearPickerView->setRange(mMinDate, mMaxDate);
}

Calendar DatePickerCalendarDelegate::getMinDate() {
    return mMinDate;
}

void DatePickerCalendarDelegate::setMaxDate(int64_t maxDate) {
    mTempDate.setTimeInMillis(maxDate);
    if (mTempDate.get(Calendar::YEAR) == mMaxDate.get(Calendar::YEAR)
            && mTempDate.get(Calendar::DAY_OF_YEAR) == mMaxDate.get(Calendar::DAY_OF_YEAR)) {
        // Same day, no-op.
        return;
    }
    if (mCurrentDate.after(mTempDate)) {
        mCurrentDate.setTimeInMillis(maxDate);
        onDateChanged(false, true);
    }
    mMaxDate.setTimeInMillis(maxDate);
    mDayPickerView->setMaxDate(maxDate);
    mYearPickerView->setRange(mMinDate, mMaxDate);
}

Calendar DatePickerCalendarDelegate::getMaxDate() {
    return mMaxDate;
}

void DatePickerCalendarDelegate::setFirstDayOfWeek(int firstDayOfWeek) {
    mFirstDayOfWeek = firstDayOfWeek;

    mDayPickerView->setFirstDayOfWeek(firstDayOfWeek);
}

int DatePickerCalendarDelegate::getFirstDayOfWeek() {
    if (mFirstDayOfWeek != USE_LOCALE) {
        return mFirstDayOfWeek;
    }
    return mCurrentDate.getFirstDayOfWeek();
}

void DatePickerCalendarDelegate::setEnabled(bool enabled) {
    mContainer->setEnabled(enabled);
    mDayPickerView->setEnabled(enabled);
    mYearPickerView->setEnabled(enabled);
    mHeaderYear->setEnabled(enabled);
    mHeaderMonthDay->setEnabled(enabled);
}

bool DatePickerCalendarDelegate::isEnabled() const{
    return mContainer->isEnabled();
}

CalendarView* DatePickerCalendarDelegate::getCalendarView() {
    throw std::runtime_error("Not supported by calendar-mode DatePicker");
}

void DatePickerCalendarDelegate::setCalendarViewShown(bool shown) {
    // No-op for compatibility with the old DatePicker.
}

bool DatePickerCalendarDelegate::getCalendarViewShown() {
    return false;
}

void DatePickerCalendarDelegate::setSpinnersShown(bool shown) {
    // No-op for compatibility with the old DatePicker.
}

bool DatePickerCalendarDelegate::getSpinnersShown() {
    return false;
}

/*void DatePickerCalendarDelegate::onConfigurationChanged(Configuration newConfig) {
    setCurrentLocale(newConfig.locale);
}*/

Parcelable* DatePickerCalendarDelegate::onSaveInstanceState(Parcelable& superState) {
    const int year = mCurrentDate.get(Calendar::YEAR);
    const int month = mCurrentDate.get(Calendar::MONTH);
    const int day = mCurrentDate.get(Calendar::DAY_OF_MONTH);

    int listPosition = -1;
    int listPositionOffset = -1;

    if (mCurrentView == VIEW_MONTH_DAY) {
        listPosition = mDayPickerView->getMostVisiblePosition();
    } else if (mCurrentView == VIEW_YEAR) {
        listPosition = mYearPickerView->getFirstVisiblePosition();
        listPositionOffset = mYearPickerView->getFirstPositionOffset();
    }

    return new SavedState(superState, year, month, day, mMinDate.getTimeInMillis(),
            mMaxDate.getTimeInMillis(), mCurrentView, listPosition, listPositionOffset);
}

void DatePickerCalendarDelegate::onRestoreInstanceState(Parcelable& state) {
    if (dynamic_cast<SavedState*>(&state)) {
        SavedState& ss = (SavedState&) state;

        // TODO: Move instance state into DayPickerView, YearPickerView.
        mCurrentDate.set(ss.getSelectedYear(), ss.getSelectedMonth(), ss.getSelectedDay());
        mMinDate.setTimeInMillis(ss.getMinDate());
        mMaxDate.setTimeInMillis(ss.getMaxDate());

        onCurrentDateChanged();

        const int currentView = ss.getCurrentView();
        setCurrentView(currentView);

        const int listPosition = ss.getListPosition();
        if (listPosition != -1) {
            if (currentView == VIEW_MONTH_DAY) {
                mDayPickerView->setPosition(listPosition);
            } else if (currentView == VIEW_YEAR) {
                const int listPositionOffset = ss.getListPositionOffset();
                mYearPickerView->setSelectionFromTop(listPosition, listPositionOffset);
            }
        }
    }
}

bool DatePickerCalendarDelegate::dispatchPopulateAccessibilityEvent(AccessibilityEvent& event) {
    onPopulateAccessibilityEvent(event);
    return true;
}

std::string DatePickerCalendarDelegate::getAccessibilityClassName() const{
    return "DatePicker";
}

int DatePickerCalendarDelegate::getDaysInMonth(int month, int year) {
    switch (month) {
        case Calendar::JANUARY:
        case Calendar::MARCH:
        case Calendar::MAY:
        case Calendar::JULY:
        case Calendar::AUGUST:
        case Calendar::OCTOBER:
        case Calendar::DECEMBER:
            return 31;
        case Calendar::APRIL:
        case Calendar::JUNE:
        case Calendar::SEPTEMBER:
        case Calendar::NOVEMBER:
            return 30;
        case Calendar::FEBRUARY:
            return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 29 : 28;
        default:
            throw std::invalid_argument("Invalid Month");
    }
}

void DatePickerCalendarDelegate::tryVibrate() {
    mDelegator->performHapticFeedback(HapticFeedbackConstants::CALENDAR_DATE);
}
}/*endof namespace*/
#endif
