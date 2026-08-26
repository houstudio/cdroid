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
#include <widget/internal_R.h>
#include <widget/daypickercalendardelegate.h>
#include <widget/daypickerview.h>
#include <widget/yearpickerview.h>
#include <widget/viewanimator.h>
#include <widget/framework_styleable.h>
#include <view/layoutinflater.h>
#include <view/view.h>
#include <view/hapticfeedbackconstants.h>
#include <core/systemclock.h>
#include <content/simpledateformat.h>
#include <content/typedarray.h>
#include <drawable/colorstatelist.h>
#include <drawable/stateset.h>
#include <porting/cdlog.h>
#include <stdexcept>
#include <string>

namespace cdroid{
using namespace cdroid::internal;

// AOSP DatePickerCalendarDelegate ATTRS_TEXT_COLOR / ATTRS_DISABLED_ALPHA.
static const uint32_t ATTRS_TEXT_COLOR[] = { R::attr::textColor, 0 };
static const uint32_t ATTRS_DISABLED_ALPHA[] = { R::attr::disabledAlpha, 0 };

DatePickerCalendarDelegate::DatePickerCalendarDelegate(DatePicker* delegator, Context* context,
        const AttributeSet* attrs, int defStyleAttr, int defStyleRes)
    : AbstractDatePickerDelegate(delegator, context) {
    mCurrentDate.setTimeInMillis(SystemClock::currentTimeMillis());

    mMinDate.set(DEFAULT_START_YEAR, Calendar::JANUARY, 1);
    mMaxDate.set(DEFAULT_END_YEAR, Calendar::DECEMBER, 31);

    auto a = mContext->obtainStyledAttributes(attrs, R::styleable::DatePicker, defStyleAttr, defStyleRes);
    LayoutInflater* inflater = LayoutInflater::from(mContext);
    const int layoutResourceId = a ? a->getResourceId(R::styleable::DatePicker_internalLayout,
            R::layout::date_picker_material) : R::layout::date_picker_material;

    // Set up and attach container.
    mContainer = (ViewGroup*) inflater->inflate(layoutResourceId, mDelegator, false);
    mContainer->setSaveFromParentEnabled(false);
    mDelegator->addView(mContainer);

    // Set up header views.
    ViewGroup* header = (ViewGroup*) mContainer->findViewById(R::id::date_picker_header);
    if (header == nullptr) {
        LOGW("date_picker_material layout has no date_picker_header; header is inert");
    }
    mHeaderYear = header ? (TextView*) header->findViewById(R::id::date_picker_header_year) : nullptr;
    mHeaderMonthDay = header ? (TextView*) header->findViewById(R::id::date_picker_header_date) : nullptr;

    View::OnClickListener headerClickListener = [this](View& v) {
        tryVibrate();
        if (v.getId() == R::id::date_picker_header_year) {
            setCurrentView(VIEW_YEAR);
        } else if (v.getId() == R::id::date_picker_header_date) {
            setCurrentView(VIEW_MONTH_DAY);
        }
    };
    if (mHeaderYear) {
        mHeaderYear->setOnClickListener(headerClickListener);
        // DEFERRED: setAccessibilityDelegate(ClickActionDelegate(context, R.string.select_year)).
        mHeaderYear->setAccessibilityLiveRegion(View::ACCESSIBILITY_LIVE_REGION_POLITE);
    }
    if (mHeaderMonthDay) {
        mHeaderMonthDay->setOnClickListener(headerClickListener);
        // DEFERRED: setAccessibilityDelegate(ClickActionDelegate(context, R.string.select_day)).
        mHeaderMonthDay->setAccessibilityLiveRegion(View::ACCESSIBILITY_LIVE_REGION_POLITE);
    }

    // For the sake of backwards compatibility, attempt to extract the text
    // color from the header month text appearance. If it's set, we'll let
    // that override the "real" header text color.
    RefPtr<ColorStateList> headerTextColor;
    const int monthHeaderTextAppearance = a ? a->getResourceId(R::styleable::DatePicker_headerMonthTextAppearance, 0) : 0;
    if (monthHeaderTextAppearance != 0) {
        auto textAppearance = mContext->obtainStyledAttributes(nullptr, ATTRS_TEXT_COLOR, 0, monthHeaderTextAppearance);
        RefPtr<ColorStateList> legacyHeaderTextColor = textAppearance ? textAppearance->getColorStateList(0) : nullptr;
        headerTextColor = applyLegacyColorFixes(legacyHeaderTextColor);
    }

    if (!headerTextColor) {
        headerTextColor = a ? a->getColorStateList(R::styleable::DatePicker_headerTextColor) : nullptr;
    }

    if (headerTextColor) {
        if (mHeaderYear) mHeaderYear->setTextColor(headerTextColor);
        if (mHeaderMonthDay) mHeaderMonthDay->setTextColor(headerTextColor);
    }

    // Set up header background, if available.
    if (header && a && a->hasValueOrEmpty(R::styleable::DatePicker_headerBackground)) {
        header->setBackground(a->getDrawable(R::styleable::DatePicker_headerBackground));
    }

    // Set up picker container.
    mAnimator = (ViewAnimator*) mContainer->findViewById(R::id::animator);

    // Set up day picker view.
    mDayPickerView = (DayPickerView*) mContainer->findViewById(R::id::date_picker_day_picker);
    if (mDayPickerView) {
        mDayPickerView->setFirstDayOfWeek(mFirstDayOfWeek);
        mDayPickerView->setMinDate(mMinDate.getTimeInMillis());
        mDayPickerView->setMaxDate(mMaxDate.getTimeInMillis());
        mDayPickerView->setDate(mCurrentDate.getTimeInMillis());
        mDayPickerView->setOnDaySelectedListener(
            [this](DayPickerView&, Calendar& day) {
                mCurrentDate.setTimeInMillis(day.getTimeInMillis());
                onDateChanged(true, true);
            });
    }

    // Set up year picker view.
    mYearPickerView = (YearPickerView*) mContainer->findViewById(R::id::date_picker_year_picker);
    if (mYearPickerView) {
        mYearPickerView->setRange(mMinDate, mMaxDate);
        mYearPickerView->setYear(mCurrentDate.get(Calendar::YEAR));
        mYearPickerView->setOnYearSelectedListener(
            [this](YearPickerView& view, int year) { onYearChanged(view, year); });
    }

    // Initialize for current locale. This also initializes the date, so no
    // need to call onDateChanged.
    onLocaleChanged(mCurrentLocale);

    setCurrentView(VIEW_MONTH_DAY);
}

void DatePickerCalendarDelegate::onYearChanged(YearPickerView& /*view*/, int year) {
    // If the newly selected month / year does not contain the currently selected day
    // number, change it to the last day of the selected month.
    const int day = mCurrentDate.get(Calendar::DAY_OF_MONTH);
    const int month = mCurrentDate.get(Calendar::MONTH);
    const int daysInMonth = getDaysInMonth(month, year);
    if (day > daysInMonth) {
        mCurrentDate.set(Calendar::DAY_OF_MONTH, daysInMonth);
    }

    mCurrentDate.set(Calendar::YEAR, year);
    if (mCurrentDate.before(mMinDate)) {
        mCurrentDate.setTimeInMillis(mMinDate.getTimeInMillis());
    } else if (mCurrentDate.after(mMaxDate)) {
        mCurrentDate.setTimeInMillis(mMaxDate.getTimeInMillis());
    }
    onDateChanged(true, true);

    // Automatically switch to day picker and return focus to the year text.
    setCurrentView(VIEW_MONTH_DAY);
    if (mHeaderYear) mHeaderYear->requestFocus();
}

void DatePickerCalendarDelegate::setCurrentView(int viewIndex) {
    switch (viewIndex) {
    case VIEW_MONTH_DAY:
        if (mDayPickerView) mDayPickerView->setDate(mCurrentDate.getTimeInMillis());
        if (mCurrentView != viewIndex) {
            if (mHeaderMonthDay) mHeaderMonthDay->setActivated(true);
            if (mHeaderYear) mHeaderYear->setActivated(false);
            if (mAnimator) mAnimator->setDisplayedChild(VIEW_MONTH_DAY);
            mCurrentView = viewIndex;
        }
        break;
    case VIEW_YEAR:
        if (mYearPickerView) {
            mYearPickerView->setYear(mCurrentDate.get(Calendar::YEAR));
            mYearPickerView->post([this]() {
                mYearPickerView->requestFocus();
                View* selected = mYearPickerView->getSelectedView();
                if (selected != nullptr) {
                    selected->requestFocus();
                }
            });
        }
        if (mCurrentView != viewIndex) {
            if (mHeaderMonthDay) mHeaderMonthDay->setActivated(false);
            if (mHeaderYear) mHeaderYear->setActivated(true);
            if (mAnimator) mAnimator->setDisplayedChild(VIEW_YEAR);
            mCurrentView = viewIndex;
        }
        break;
    }
}

void DatePickerCalendarDelegate::init(int year, int month, int dayOfMonth,
        const DatePicker::OnDateChangedListener& callBack) {
    setDate(year, month, dayOfMonth);
    onDateChanged(false, false);
    mOnDateChangedListener = callBack;
}

void DatePickerCalendarDelegate::updateDate(int year, int month, int dayOfMonth) {
    setDate(year, month, dayOfMonth);
    onDateChanged(false, true);
}

void DatePickerCalendarDelegate::setDate(int year, int month, int dayOfMonth) {
    mCurrentDate.set(year, month, dayOfMonth);
    resetAutofilledValue();
}

void DatePickerCalendarDelegate::onDateChanged(bool fromUser, bool callbackToClient) {
    const int year = mCurrentDate.get(Calendar::YEAR);
    const int monthOfYear = mCurrentDate.get(Calendar::MONTH);
    const int dayOfMonth = mCurrentDate.get(Calendar::DAY_OF_MONTH);

    if (callbackToClient) {
        if (mOnDateChangedListener) {
            mOnDateChangedListener(*mDelegator, year, monthOfYear, dayOfMonth);
        }
        if (mAutoFillChangeListener) {
            mAutoFillChangeListener(*mDelegator, year, monthOfYear, dayOfMonth);
        }
    }

    if (mDayPickerView) mDayPickerView->setDate(mCurrentDate.getTimeInMillis());
    if (mYearPickerView) mYearPickerView->setYear(year);

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
        return; // Same day, no-op.
    }
    mMinDate.setTimeInMillis(minDate);
    if (mCurrentDate.before(mMinDate)) {
        mCurrentDate.setTimeInMillis(mMinDate.getTimeInMillis());
        onDateChanged(false, true);
    }
    if (mDayPickerView) mDayPickerView->setMinDate(minDate);
    if (mYearPickerView) mYearPickerView->setRange(mMinDate, mMaxDate);
}

Calendar DatePickerCalendarDelegate::getMinDate() {
    return mMinDate;
}

void DatePickerCalendarDelegate::setMaxDate(int64_t maxDate) {
    mTempDate.setTimeInMillis(maxDate);
    if (mTempDate.get(Calendar::YEAR) == mMaxDate.get(Calendar::YEAR)
            && mTempDate.get(Calendar::DAY_OF_YEAR) == mMaxDate.get(Calendar::DAY_OF_YEAR)) {
        return; // Same day, no-op.
    }
    mMaxDate.setTimeInMillis(maxDate);
    if (mCurrentDate.after(mMaxDate)) {
        mCurrentDate.setTimeInMillis(mMaxDate.getTimeInMillis());
        onDateChanged(false, true);
    }
    if (mDayPickerView) mDayPickerView->setMaxDate(maxDate);
    if (mYearPickerView) mYearPickerView->setRange(mMinDate, mMaxDate);
}

Calendar DatePickerCalendarDelegate::getMaxDate() {
    return mMaxDate;
}

void DatePickerCalendarDelegate::setFirstDayOfWeek(int firstDayOfWeek) {
    mFirstDayOfWeek = firstDayOfWeek;
    if (mDayPickerView) mDayPickerView->setFirstDayOfWeek(firstDayOfWeek);
}

int DatePickerCalendarDelegate::getFirstDayOfWeek() {
    if (mFirstDayOfWeek != USE_LOCALE) {
        return mFirstDayOfWeek;
    }
    return mCurrentDate.getFirstDayOfWeek();
}

void DatePickerCalendarDelegate::setEnabled(bool enabled) {
    if (mContainer) mContainer->setEnabled(enabled);
    if (mDayPickerView) mDayPickerView->setEnabled(enabled);
    if (mYearPickerView) mYearPickerView->setEnabled(enabled);
    if (mHeaderYear) mHeaderYear->setEnabled(enabled);
    if (mHeaderMonthDay) mHeaderMonthDay->setEnabled(enabled);
}

bool DatePickerCalendarDelegate::isEnabled() const {
    return mContainer && mContainer->isEnabled();
}

CalendarView* DatePickerCalendarDelegate::getCalendarView() {
    throw std::runtime_error("Not supported by calendar-mode DatePicker");
}

void DatePickerCalendarDelegate::setCalendarViewShown(bool /*shown*/) {
    // No-op for compatibility with the old DatePicker.
}

bool DatePickerCalendarDelegate::getCalendarViewShown() {
    return false;
}

void DatePickerCalendarDelegate::setSpinnersShown(bool /*shown*/) {
    // No-op for compatibility with the old DatePicker.
}

bool DatePickerCalendarDelegate::getSpinnersShown() {
    return false;
}

Parcelable* DatePickerCalendarDelegate::onSaveInstanceState(Parcelable& superState) {
    const int year = mCurrentDate.get(Calendar::YEAR);
    const int month = mCurrentDate.get(Calendar::MONTH);
    const int day = mCurrentDate.get(Calendar::DAY_OF_MONTH);

    int listPosition = -1;
    int listPositionOffset = -1;

    if (mCurrentView == VIEW_MONTH_DAY) {
        if (mDayPickerView) listPosition = mDayPickerView->getMostVisiblePosition();
    } else if (mCurrentView == VIEW_YEAR) {
        if (mYearPickerView) listPosition = mYearPickerView->getFirstVisiblePosition();
        listPositionOffset = mYearPickerView ? mYearPickerView->getFirstPositionOffset() : -1;
    }

    return new AbstractDatePickerDelegate::SavedState(superState, year, month, day, mMinDate.getTimeInMillis(),
            mMaxDate.getTimeInMillis(), mCurrentView, listPosition, listPositionOffset);
}

void DatePickerCalendarDelegate::onRestoreInstanceState(Parcelable& state) {
    auto* ss = dynamic_cast<AbstractDatePickerDelegate::SavedState*>(&state);
    if (ss) {
        mCurrentDate.set(ss->getSelectedYear(), ss->getSelectedMonth(), ss->getSelectedDay());
        mMinDate.setTimeInMillis(ss->getMinDate());
        mMaxDate.setTimeInMillis(ss->getMaxDate());

        onCurrentDateChanged();

        const int currentView = ss->getCurrentView();
        setCurrentView(currentView);

        const int listPosition = ss->getListPosition();
        if (listPosition != -1) {
            if (currentView == VIEW_MONTH_DAY) {
                if (mDayPickerView) mDayPickerView->setPosition(listPosition);
            } else if (currentView == VIEW_YEAR) {
                const int listPositionOffset = ss->getListPositionOffset();
                if (mYearPickerView) mYearPickerView->setSelectionFromTop(listPosition, listPositionOffset);
            }
        }
    }
}

bool DatePickerCalendarDelegate::dispatchPopulateAccessibilityEvent(AccessibilityEvent& event) {
    onPopulateAccessibilityEvent(event);
    return true;
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
            return 30;
    }
}

void DatePickerCalendarDelegate::tryVibrate() {
    mDelegator->performHapticFeedback(HapticFeedbackConstants::CALENDAR_DATE);
}

void DatePickerCalendarDelegate::onConfigurationChanged(Configuration& newConfig) {
    setCurrentLocale(newConfig.getLocales().get(0));
}

void DatePickerCalendarDelegate::onLocaleChanged(const Locale& locale) {
    // Skeletons "EMMMd"/"y" are approximated with explicit patterns in
    // onCurrentDateChanged (CDROID has no getBestDateTimePattern); the locale
    // itself only selects the symbol tables inside SimpleDateFormat.
    (void)locale;
    if (mHeaderYear == nullptr) {
        // Abort, we haven't initialized yet. This method will get called
        // again later after everything has been set up.
        return;
    }

    // Update the header text.
    onCurrentDateChanged();
}

void DatePickerCalendarDelegate::onCurrentDateChanged() {
    if (mHeaderYear == nullptr) {
        // Abort, we haven't initialized yet. This method will get called
        // again later after everything has been set up.
        return;
    }

    // AOSP formats via skeletons "y" and "EMMMd" (DateFormat.getBestDateTimePattern);
    // CDROID has no DTPG, so use the equivalent explicit en patterns and let
    // SimpleDateFormat resolve the per-locale names.
    SimpleDateFormat yearFormat("yyyy", mCurrentLocale);
    mHeaderYear->setText(yearFormat.format(mCurrentDate.getTimeInMillis()));
    SimpleDateFormat monthDayFormat("EEE, MMM d", mCurrentLocale);
    mHeaderMonthDay->setText(monthDayFormat.format(mCurrentDate.getTimeInMillis()));
}

RefPtr<ColorStateList> DatePickerCalendarDelegate::applyLegacyColorFixes(RefPtr<ColorStateList> color) {
    // The legacy text color might have been poorly defined. Ensures that it
    // has an appropriate activated state, using the selected state if one
    // exists or modifying the default text color otherwise.
    if (!color || color->hasState(R::attr::state_activated)) {
        return color;
    }

    int activatedColor;
    int defaultColor;
    if (color->hasState(R::attr::state_selected)) {
        activatedColor = color->getColorForState(StateSet::get(
                StateSet::VIEW_STATE_ENABLED | StateSet::VIEW_STATE_SELECTED), 0);
        defaultColor = color->getColorForState(StateSet::get(
                StateSet::VIEW_STATE_ENABLED), 0);
    } else {
        activatedColor = color->getDefaultColor();

        // Generate a non-activated color using the disabled alpha.
        auto ta = mContext->obtainStyledAttributes(ATTRS_DISABLED_ALPHA);
        const float disabledAlpha = ta ? ta->getFloat(0, 0.30f) : 0.30f;
        defaultColor = multiplyAlphaComponent(activatedColor, disabledAlpha);
    }

    if (activatedColor == 0 || defaultColor == 0) {
        // We somehow failed to obtain the colors.
        return nullptr;
    }

    const std::vector<std::vector<int>> states = { { R::attr::state_activated }, {} };
    const std::vector<int> colors = { activatedColor, defaultColor };
    return RefPtr<ColorStateList>(new ColorStateList(states, colors));
}

int DatePickerCalendarDelegate::multiplyAlphaComponent(int color, float alphaMod) {
    const int srcRgb = color & 0xFFFFFF;
    const int srcAlpha = (color >> 24) & 0xFF;
    const int dstAlpha = (int) (srcAlpha * alphaMod + 0.5f);
    return srcRgb | (dstAlpha << 24);
}

} // namespace cdroid
