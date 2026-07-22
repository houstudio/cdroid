#include <climits>
#include <widget/R.h>
#include <widget/timepicker.h>
#include <widget/timepickerclockdelegate.h>
#include <widget/radialtimepickerview.h>
#include <widget/textinputtimepickerview.h>
#include <widget/relativelayout.h>
#include <view/layoutinflater.h>
#include <view/viewgroup.h>
#include <utils/textutils.h>

namespace cdroid {

namespace {
// Ported from Java private static class NearestTouchDelegate implements View.OnTouchListener.
// Routes a touch on a transparent container (e.g. the AM/PM layout) to the nearest child,
// so tapping between two radio buttons hits the closer one. Each instance holds its own
// mInitialTouchTarget across the gesture; std::function (View::OnTouchListener) owns the copy.
class NearestTouchDelegate {
private:
    View* mInitialTouchTarget = nullptr;
    static View* findNearestChild(ViewGroup* v, int x, int y) {
        View* bestChild = nullptr;
        int bestDist = INT_MAX;
        for (int i = 0, count = v->getChildCount(); i < count; i++) {
            View* child = v->getChildAt(i);
            const int dX = x - (child->getLeft() + child->getWidth() / 2);
            const int dY = y - (child->getTop() + child->getHeight() / 2);
            const int dist = dX * dX + dY * dY;
            if (bestDist > dist) {
                bestChild = child;
                bestDist = dist;
            }
        }
        return bestChild;
    }
public:
    bool operator()(View& view, MotionEvent& motionEvent) {
        const int actionMasked = motionEvent.getActionMasked();
        if (actionMasked == MotionEvent::ACTION_DOWN) {
            ViewGroup* vg = dynamic_cast<ViewGroup*>(&view);
            if (vg != nullptr) {
                mInitialTouchTarget = findNearestChild(vg,
                        (int) motionEvent.getX(), (int) motionEvent.getY());
            } else {
                mInitialTouchTarget = nullptr;
            }
        }

        View* child = mInitialTouchTarget;
        if (child == nullptr) {
            return false;
        }

        const float offsetX = view.getScrollX() - child->getLeft();
        const float offsetY = view.getScrollY() - child->getTop();
        motionEvent.offsetLocation(offsetX, offsetY);
        const bool handled = child->dispatchTouchEvent(motionEvent);
        motionEvent.offsetLocation(-offsetX, -offsetY);

        if (actionMasked == MotionEvent::ACTION_UP
                || actionMasked == MotionEvent::ACTION_CANCEL) {
            mInitialTouchTarget = nullptr;
        }

        return handled;
    }
};
} // namespace

TimePickerClockDelegate::TimePickerClockDelegate(TimePicker* delegator, Context* context,const AttributeSet& attrs)
    :AbstractTimePickerDelegate(delegator, context){

    // Accessibility contentDescription strings are not wired (deferred); the values are only
    // used by onPopulateAccessibilityEvent / setContentDescription, which are accessibility-only.
    mSelectHours = "";
    mSelectMinutes = "";

    LayoutInflater* inflater = LayoutInflater::from(mContext);

    // CDROID reads style attributes by name from AttributeSet; the legacy TypedArray
    // path (R.styleable.TimePicker_*) is not wired here.
    const std::string layoutResourceId = attrs.getString("internalLayout",
            "cdroid:layout/time_picker_material");
    View* mainView = inflater->inflate(layoutResourceId, delegator);
    mainView->setSaveFromParentEnabled(false);
    mRadialTimePickerHeader = mainView->findViewById(R::id::time_header);
    mRadialTimePickerHeader->setOnTouchListener(NearestTouchDelegate());

    // Set up hour/minute labels.
    mHourView = (NumericTextView*) mainView->findViewById(R::id::hours);

    mClickListener = [this](View& v){
        onViewClick(v);
    };
    mFocusListener = [this](View& v, bool focused){
        onViewFocusChange(v, focused);
    };

    // Ported from Java anonymous OnValueSelectedListener (radial picker).
    mOnValueSelectedListener = [this](int pickerType, int newValue, bool autoAdvance){
        bool valueChanged = false;
        switch (pickerType) {
        case RadialTimePickerView::HOURS:
            if (getHour() != newValue) {
                valueChanged = true;
            }
            {
                const bool isTransition = mAllowAutoAdvance && autoAdvance;
                setHourInternal(newValue, FROM_RADIAL_PICKER, !isTransition, true);
                if (isTransition) {
                    setCurrentItemShowing(MINUTE_INDEX, true);
                }
            }
            break;
        case RadialTimePickerView::MINUTES:
            if (getMinute() != newValue) {
                valueChanged = true;
            }
            setMinuteInternal(newValue, FROM_RADIAL_PICKER, true);
            break;
        default:
            break;
        }

        if (mOnTimeChangedListener && valueChanged) {
            mOnTimeChangedListener(*mDelegator, getHour(), getMinute());
        }
    };

    // Ported from Java anonymous OnValueTypedListener (text-input picker).
    mOnValueTypedListener = [this](int pickerType, int newValue){
        switch (pickerType) {
        case TextInputTimePickerView::HOURS:
            setHourInternal(newValue, FROM_INPUT_PICKER, false, true);
            break;
        case TextInputTimePickerView::MINUTES:
            setMinuteInternal(newValue, FROM_INPUT_PICKER, true);
            break;
        case TextInputTimePickerView::AMPM:
            setAmOrPm(newValue);
            break;
        default:
            break;
        }
    };

    // Ported from Java anonymous OnValueChangedListener (NumericTextView digit entry).
    // mCommitHour / mCommitMinute are assigned below; lambdas capture this and read them
    // at invocation time. Runnable (CallbackBase) copies share identity, so removeCallbacks
    // matches the earlier postDelayed provided the same member is used for both.
    mDigitEnteredListener = [this](NumericTextView& view, int /*value*/, bool isValid, bool isFinished){
        Runnable* commitCallback = nullptr;
        View* nextFocusTarget = nullptr;
        if (&view == mHourView) {
            commitCallback = &mCommitHour;
            nextFocusTarget = view.isFocused() ? mMinuteView : nullptr;
        } else if (&view == mMinuteView) {
            commitCallback = &mCommitMinute;
            nextFocusTarget = nullptr;
        } else {
            return;
        }

        view.removeCallbacks(*commitCallback);

        if (isValid) {
            if (isFinished) {
                // Done with hours entry, make visual updates immediately and move focus.
                (*commitCallback)();
                if (nextFocusTarget != nullptr) {
                    nextFocusTarget->requestFocus();
                }
            } else {
                // May still be making changes. Postpone visual updates.
                view.postDelayed(*commitCallback, DELAY_COMMIT_MILLIS);
            }
        }
    };

    mHourView->setOnClickListener(mClickListener);
    mHourView->setOnFocusChangeListener(mFocusListener);
    mHourView->setOnDigitEnteredListener(mDigitEnteredListener);
    // DEFERRED: mHourView->setAccessibilityDelegate(new ClickActionDelegate(context, R.string.select_hours));
    mHourView->setAccessibilityLiveRegion(View::ACCESSIBILITY_LIVE_REGION_POLITE);
    mSeparatorView = (TextView*) mainView->findViewById(R::id::separator);
    mMinuteView = (NumericTextView*) mainView->findViewById(R::id::minutes);
    mMinuteView->setOnClickListener(mClickListener);
    mMinuteView->setOnFocusChangeListener(mFocusListener);
    mMinuteView->setOnDigitEnteredListener(mDigitEnteredListener);
    // DEFERRED: accessibility delegate.
    mMinuteView->setAccessibilityLiveRegion(View::ACCESSIBILITY_LIVE_REGION_POLITE);
    mMinuteView->setRange(0, 59);

    // Set up AM/PM labels.
    mAmPmLayout = mainView->findViewById(R::id::ampm_layout);
    mAmPmLayout->setOnTouchListener(NearestTouchDelegate());

    const std::vector<std::string> amPmStrings = TimePicker::getAmPmStrings(context);
    mAmLabel = (RadioButton*) mAmPmLayout->findViewById(R::id::am_label);
    mAmLabel->setText(obtainVerbatim(amPmStrings[0]));
    mAmLabel->setOnClickListener(mClickListener);
    ensureMinimumTextWidth(mAmLabel);

    mPmLabel = (RadioButton*) mAmPmLayout->findViewById(R::id::pm_label);
    mPmLabel->setText(obtainVerbatim(amPmStrings[1]));
    mPmLabel->setOnClickListener(mClickListener);
    ensureMinimumTextWidth(mPmLabel);

    // DEFERRED: legacy header text color extracted from headerTimeTextAppearance and
    // R.styleable.TimePicker_headerTextColor / headerBackground styling. Depends on
    // ColorStateList / obtainStyledAttributes not wired; views keep their XML styling.
    mTextInputPickerHeader = mainView->findViewById(R::id::input_header);

    mRadialTimePickerView = (RadialTimePickerView*) mainView->findViewById(R::id::radial_picker);
    mRadialTimePickerView->applyAttributes(attrs);
    mRadialTimePickerView->setOnValueSelectedListener(mOnValueSelectedListener);

    mTextInputPickerView = (TextInputTimePickerView*) mainView->findViewById(R::id::input_mode);
    mTextInputPickerView->setListener(mOnValueTypedListener);

    mRadialTimePickerModeButton = (ImageButton*) mainView->findViewById(R::id::toggle_mode);
    mRadialTimePickerModeButton->setOnClickListener([this](View& /*v*/) {
         toggleRadialPickerMode();
    });
    // Accessibility contentDescription strings are not wired (deferred).
    mRadialTimePickerModeEnabledDescription = "";
    mTextInputPickerModeEnabledDescription = "";

    mAllowAutoAdvance = true;

    updateHourFormat();

    // Initialize with current time (mTempCalendar defaults to now).
    // AOSP uses Calendar.getInstance(locale) (local TZ); mirror the local-TZ
    // part here since the default ctor leaves zone=0/UTC.
    mTempCalendar.setTimeZone(Calendar::getInstance()->getTimeZone());
    const int currentHour = mTempCalendar.get(Calendar::HOUR_OF_DAY);
    const int currentMinute = mTempCalendar.get(Calendar::MINUTE);
    initialize(currentHour, currentMinute, mIs24Hour, HOUR_INDEX);

    mCommitHour = [this](){ setHour(mHourView->getValue()); };
    mCommitMinute = [this](){ setMinute(mMinuteView->getValue()); };
}

void TimePickerClockDelegate::toggleRadialPickerMode() {
    if (mRadialPickerModeEnabled) {
        mRadialTimePickerView->setVisibility(View::GONE);
        mRadialTimePickerHeader->setVisibility(View::GONE);
        mTextInputPickerHeader->setVisibility(View::VISIBLE);
        mTextInputPickerView->setVisibility(View::VISIBLE);
        mRadialTimePickerModeButton->setImageResource("cdroid:drawable/btn_clock_material");
        mRadialTimePickerModeButton->setContentDescription(mRadialTimePickerModeEnabledDescription);
        mRadialPickerModeEnabled = false;
    } else {
        mRadialTimePickerView->setVisibility(View::VISIBLE);
        mRadialTimePickerHeader->setVisibility(View::VISIBLE);
        mTextInputPickerHeader->setVisibility(View::GONE);
        mTextInputPickerView->setVisibility(View::GONE);
        mRadialTimePickerModeButton->setImageResource("cdroid:drawable/btn_keyboard_key_material");
        mRadialTimePickerModeButton->setContentDescription(mTextInputPickerModeEnabledDescription);
        updateTextInputPicker();
        // DEFERRED: InputMethodManager.hideSoftInputFromWindow not wired.
        mRadialPickerModeEnabled = true;
    }
}

bool TimePickerClockDelegate::validateInput() {
    return mTextInputPickerView->validateInput();
}

void TimePickerClockDelegate::ensureMinimumTextWidth(TextView* v) {
    v->measure(MeasureSpec::UNSPECIFIED, MeasureSpec::UNSPECIFIED);

    // Set both the TextView and the View version of minimum
    // width because they are subtly different.
    const int minWidth = v->getMeasuredWidth();
    v->setMinWidth(minWidth);
    v->setMinimumWidth(minWidth);
}

void TimePickerClockDelegate::updateHourFormat() {
    // DEFERRED: android.text.format.DateFormat.getBestDateTimePattern(Locale, skeleton) not
    // ported. Default to a two-digit pattern that also yields a sensible ':' separator in
    // updateHeaderSeparator / getHourMinSeparatorFromPattern.
    const std::string bestDateTimePattern = mIs24Hour ? "HH:mm" : "hh:mm";
    const int lengthPattern = (int) bestDateTimePattern.length();
    bool showLeadingZero = false;
    char hourFormat = '\0';

    for (int i = 0; i < lengthPattern; i++) {
        const char c = bestDateTimePattern.at(i);
        if (c == 'H' || c == 'h' || c == 'K' || c == 'k') {
            hourFormat = c;
            if (i + 1 < lengthPattern && c == bestDateTimePattern.at(i + 1)) {
                showLeadingZero = true;
            }
            break;
        }
    }

    mHourFormatShowLeadingZero = showLeadingZero;
    mHourFormatStartsAtZero = hourFormat == 'K' || hourFormat == 'H';

    // Update hour text field.
    const int minHour = mHourFormatStartsAtZero ? 0 : 1;
    const int maxHour = (mIs24Hour ? 23 : 11) + minHour;
    mHourView->setRange(minHour, maxHour);
    mHourView->setShowLeadingZeroes(mHourFormatShowLeadingZero);

    // DEFERRED: DecimalFormatSymbols.getInstance(Locale).getDigitStrings() not ported;
    // assume ASCII digits (max char length 1).
    mTextInputPickerView->setHourFormat(1 * 2);
}

std::string TimePickerClockDelegate::obtainVerbatim(const std::string& text) {
    // DEFERRED: TtsSpan.VerbatimBuilder is accessibility-only metadata; return the text as-is.
    return text;
}

ColorStateList* TimePickerClockDelegate::applyLegacyColorFixes(ColorStateList* color) {
    // DEFERRED: legacy header-text-color fixes depend on ColorStateList state manipulation.
    return color;
}

int TimePickerClockDelegate::multiplyAlphaComponent(int color, float alphaMod) {
    const int srcRgb = color & 0xFFFFFF;
    const int srcAlpha = (color >> 24) & 0xFF;
    const int dstAlpha = (int) (srcAlpha * alphaMod + 0.5f);
    return srcRgb | (dstAlpha << 24);
}

void TimePickerClockDelegate::initialize(int hourOfDay, int minute, bool is24HourView, int index) {
    mCurrentHour = hourOfDay;
    mCurrentMinute = minute;
    mIs24Hour = is24HourView;
    updateUI(index);
}

 void TimePickerClockDelegate::updateUI(int index) {
    updateHeaderAmPm();
    updateHeaderHour(mCurrentHour, false);
    updateHeaderSeparator();
    updateHeaderMinute(mCurrentMinute, false);
    updateRadialPicker(index);
    updateTextInputPicker();

    mDelegator->invalidate();
}

void TimePickerClockDelegate::updateTextInputPicker() {
    mTextInputPickerView->updateTextInputValues(getLocalizedHour(mCurrentHour), mCurrentMinute,
            mCurrentHour < 12 ? AM : PM, mIs24Hour, mHourFormatStartsAtZero);
}

void TimePickerClockDelegate::updateRadialPicker(int index) {
    mRadialTimePickerView->initialize(mCurrentHour, mCurrentMinute, mIs24Hour);
    setCurrentItemShowing(index, false);
}

void TimePickerClockDelegate::updateHeaderAmPm() {
    if (mIs24Hour) {
        mAmPmLayout->setVisibility(View::GONE);
    } else {
        // Find the location of AM/PM based on locale information.
        // DEFERRED: DateFormat.getBestDateTimePattern(Locale, "hm"); assume am/pm at end.
        const bool isAmPmAtStart = false;
        setAmPmStart(isAmPmAtStart);
        updateAmPmLabelStates(mCurrentHour < 12 ? AM : PM);
    }
}

void TimePickerClockDelegate::setAmPmStart(bool isAmPmAtStart) {
    RelativeLayout::LayoutParams* params = (RelativeLayout::LayoutParams*) mAmPmLayout->getLayoutParams();
    if (params->getRule(RelativeLayout::RIGHT_OF) != 0
            || params->getRule(RelativeLayout::LEFT_OF) != 0) {
        const int margin = (int) (mContext->getDisplayMetrics().density * 8);
        // Horizontal mode, with AM/PM appearing to left/right of hours and minutes.
        bool isAmPmAtLeft;
        // DEFERRED: TextUtils.getLayoutDirectionFromLocale(mLocale); assume LTR.
        const int layoutDirection = View::LAYOUT_DIRECTION_LTR;
        if (layoutDirection == View::LAYOUT_DIRECTION_LTR) {
            isAmPmAtLeft = isAmPmAtStart;
        } else {
            isAmPmAtLeft = !isAmPmAtStart;
        }

        if (isAmPmAtLeft) {
            params->removeRule(RelativeLayout::RIGHT_OF);
            params->addRule(RelativeLayout::LEFT_OF, mHourView->getId());
        } else {
            params->removeRule(RelativeLayout::LEFT_OF);
            params->addRule(RelativeLayout::RIGHT_OF, mMinuteView->getId());
        }

        if (isAmPmAtStart) {
            params->setMarginStart(0);
            params->setMarginEnd(margin);
        } else {
            params->setMarginStart(margin);
            params->setMarginEnd(0);
        }
        mIsAmPmAtLeft = isAmPmAtLeft;
    } else if (params->getRule(RelativeLayout::BELOW) != 0
            || params->getRule(RelativeLayout::ABOVE) != 0) {
        // Vertical mode, with AM/PM appearing to top/bottom of hours and minutes.
        if (mIsAmPmAtTop == isAmPmAtStart) {
            // AM/PM is already at the correct location. No change needed.
            return;
        }

        int otherViewId;
        if (isAmPmAtStart) {
            otherViewId = params->getRule(RelativeLayout::BELOW);
            params->removeRule(RelativeLayout::BELOW);
            params->addRule(RelativeLayout::ABOVE, otherViewId);
        } else {
            otherViewId = params->getRule(RelativeLayout::ABOVE);
            params->removeRule(RelativeLayout::ABOVE);
            params->addRule(RelativeLayout::BELOW, otherViewId);
        }

        // Switch the top and bottom paddings on the other view.
        View* otherView = mRadialTimePickerHeader->findViewById(otherViewId);
        const int top = otherView->getPaddingTop();
        const int bottom = otherView->getPaddingBottom();
        const int left = otherView->getPaddingLeft();
        const int right = otherView->getPaddingRight();
        otherView->setPadding(left, bottom, right, top);

        mIsAmPmAtTop = isAmPmAtStart;
    }

    mAmPmLayout->setLayoutParams(params);
}

void TimePickerClockDelegate::setDate(int hour, int minute) {
    setHourInternal(hour, FROM_EXTERNAL_API, true, false);
    setMinuteInternal(minute, FROM_EXTERNAL_API, false);

    onTimeChanged();
}

void TimePickerClockDelegate::setHour(int hour) {
    setHourInternal(hour, FROM_EXTERNAL_API, true, true);
}

void TimePickerClockDelegate::setHourInternal(int hour, int source, bool announce,bool notify) {
    if (mCurrentHour == hour) {
        return;
    }

    //resetAutofilledValue();
    mCurrentHour = hour;
    updateHeaderHour(hour, announce);
    updateHeaderAmPm();

    if (source != FROM_RADIAL_PICKER) {
        mRadialTimePickerView->setCurrentHour(hour);
        mRadialTimePickerView->setAmOrPm(hour < 12 ? AM : PM);
    }
    if (source != FROM_INPUT_PICKER) {
        updateTextInputPicker();
    }

    mDelegator->invalidate();
    if (notify) {
        onTimeChanged();
    }
}

int TimePickerClockDelegate::getHour() {
    const int currentHour = mRadialTimePickerView->getCurrentHour();
    if (mIs24Hour) {
        return currentHour;
    }

    if (mRadialTimePickerView->getAmOrPm() == PM) {
        return (currentHour % HOURS_IN_HALF_DAY) + HOURS_IN_HALF_DAY;
    } else {
        return currentHour % HOURS_IN_HALF_DAY;
    }
    return currentHour;/*make gcc happy*/
}

void TimePickerClockDelegate::setMinute(int minute) {
    setMinuteInternal(minute, FROM_EXTERNAL_API, true);
}

void TimePickerClockDelegate::setMinuteInternal(int minute,int source, bool notify) {
    if (mCurrentMinute == minute) {
        return;
    }

    //resetAutofilledValue();
    mCurrentMinute = minute;
    updateHeaderMinute(minute, true);

    if (source != FROM_RADIAL_PICKER) {
        mRadialTimePickerView->setCurrentMinute(minute);
    }
    if (source != FROM_INPUT_PICKER) {
        updateTextInputPicker();
    }

    mDelegator->invalidate();
    if (notify) {
        onTimeChanged();
    }
}

int TimePickerClockDelegate::getMinute() {
    return mRadialTimePickerView->getCurrentMinute();
}

void TimePickerClockDelegate::setIs24Hour(bool is24Hour) {
    if (mIs24Hour != is24Hour) {
        mIs24Hour = is24Hour;
        mCurrentHour = getHour();

        updateHourFormat();
        updateUI(mRadialTimePickerView->getCurrentItemShowing());
    }
}

bool TimePickerClockDelegate::is24Hour(){
    return mIs24Hour;
}

void TimePickerClockDelegate::setEnabled(bool enabled) {
    mHourView->setEnabled(enabled);
    mMinuteView->setEnabled(enabled);
    mAmLabel->setEnabled(enabled);
    mPmLabel->setEnabled(enabled);
    mRadialTimePickerView->setEnabled(enabled);
    mIsEnabled = enabled;
}

bool TimePickerClockDelegate::isEnabled() const{
    return mIsEnabled;
}

int TimePickerClockDelegate::getBaseline() {
    // does not support baseline alignment
    return -1;
}

Parcelable* TimePickerClockDelegate::onSaveInstanceState(Parcelable& superState) {
    return new AbstractTimePickerDelegate::SavedState(&superState, getHour(), getMinute(),
            is24Hour(), getCurrentItemShowing());
}

void TimePickerClockDelegate::onRestoreInstanceState(Parcelable& state) {
    auto* ss = dynamic_cast<AbstractTimePickerDelegate::SavedState*>(&state);
    if (ss != nullptr) {
        initialize(ss->getHour(), ss->getMinute(), ss->is24HourMode(), ss->getCurrentItemShowing());
        mRadialTimePickerView->invalidate();
    }
}

bool TimePickerClockDelegate::dispatchPopulateAccessibilityEvent(AccessibilityEvent& event) {
    onPopulateAccessibilityEvent(event);
    return true;
}

void TimePickerClockDelegate::onPopulateAccessibilityEvent(AccessibilityEvent& /*event*/) {
    // DEFERRED: DateUtils.formatDateTime not ported.
}

View* TimePickerClockDelegate::getHourView() {
    return mHourView;
}

View* TimePickerClockDelegate::getMinuteView() {
    return mMinuteView;
}

View* TimePickerClockDelegate::getAmView() {
    return mAmLabel;
}

View* TimePickerClockDelegate::getPmView() {
    return mPmLabel;
}

int TimePickerClockDelegate::getCurrentItemShowing() {
    return mRadialTimePickerView->getCurrentItemShowing();
}

void TimePickerClockDelegate::onTimeChanged() {
    mDelegator->sendAccessibilityEvent(AccessibilityEvent::TYPE_VIEW_SELECTED);
    if (mOnTimeChangedListener) {
        mOnTimeChangedListener(*mDelegator, getHour(), getMinute());
    }
    if (mAutoFillChangeListener) {
        mAutoFillChangeListener(*mDelegator, getHour(), getMinute());
    }
}

void TimePickerClockDelegate::tryVibrate() {
    mDelegator->performHapticFeedback(HapticFeedbackConstants::CLOCK_TICK);
}

void TimePickerClockDelegate::updateAmPmLabelStates(int amOrPm) {
    const bool isAm = amOrPm == AM;
    mAmLabel->setActivated(isAm);
    mAmLabel->setChecked(isAm);

    const bool isPm = amOrPm == PM;
    mPmLabel->setActivated(isPm);
    mPmLabel->setChecked(isPm);
}

int TimePickerClockDelegate::getLocalizedHour(int hourOfDay) {
    if (!mIs24Hour) {
        // Convert to hour-of-am-pm.
        hourOfDay %= 12;
    }

    if (!mHourFormatStartsAtZero && hourOfDay == 0) {
        // Convert to clock-hour (either of-day or of-am-pm).
        hourOfDay = mIs24Hour ? 24 : 12;
    }

    return hourOfDay;
}

void TimePickerClockDelegate::updateHeaderHour(int hourOfDay, bool /*announce*/) {
    const int localizedHour = getLocalizedHour(hourOfDay);
    mHourView->setValue(localizedHour);
}

void TimePickerClockDelegate::updateHeaderMinute(int minuteOfHour, bool /*announce*/) {
    mMinuteView->setValue(minuteOfHour);
}

void TimePickerClockDelegate::updateHeaderSeparator() {
    // DEFERRED: DateFormat.getBestDateTimePattern(Locale, "Hm"/"hm"); see updateHourFormat.
    const std::string bestDateTimePattern = mIs24Hour ? "HH:mm" : "hh:mm";
    const std::string separatorText = getHourMinSeparatorFromPattern(bestDateTimePattern);
    mSeparatorView->setText(separatorText);
    mTextInputPickerView->updateSeparator(separatorText);
}

std::string TimePickerClockDelegate::getHourMinSeparatorFromPattern(const std::string& dateTimePattern) {
    const std::string defaultSeparator = ":";
    bool foundHourPattern = false;
    for (int i = 0; i < (int) dateTimePattern.length(); i++) {
        switch (dateTimePattern.at(i)) {
        // See http://www.unicode.org/reports/tr35/tr35-dates.html for hour formats.
        case 'H':
        case 'h':
        case 'K':
        case 'k':
            foundHourPattern = true;
            continue;
        case ' ': // skip spaces
            continue;
        case '\'':
            // DEFERRED: quoted-separator parsing needs DateFormat.appendQuotedText; current
            // locale patterns contain no quotes.
            if (!foundHourPattern) {
                continue;
            }
            return defaultSeparator;
        default:
            if (!foundHourPattern) {
                continue;
            }
            return std::string(1, dateTimePattern.at(i));
        }
    }
    return defaultSeparator;
}

int TimePickerClockDelegate::lastIndexOfAny(const std::string& str, const std::string& any) {
    const int lengthAny = (int) any.length();
    if (lengthAny > 0) {
        for (int i = (int) str.length() - 1; i >= 0; i--) {
            char c = str.at(i);
            for (int j = 0; j < lengthAny; j++) {
                if (c == any[j]) {
                    return i;
                }
            }
        }
    }
    return -1;
}

void TimePickerClockDelegate::setCurrentItemShowing(int index, bool animateCircle) {
    mRadialTimePickerView->setCurrentItemShowing(index, animateCircle);

    mHourView->setActivated(index == HOUR_INDEX);
    mMinuteView->setActivated(index == MINUTE_INDEX);
}

void TimePickerClockDelegate::setAmOrPm(int amOrPm) {
    updateAmPmLabelStates(amOrPm);

    if (mRadialTimePickerView->setAmOrPm(amOrPm)) {
        mCurrentHour = getHour();
        updateTextInputPicker();
        if (mOnTimeChangedListener) {
            mOnTimeChangedListener(*mDelegator, getHour(), getMinute());
        }
    }
}

void TimePickerClockDelegate::onViewFocusChange(View& v, bool focused) {
    if (focused) {
        switch (v.getId()) {
        case R::id::am_label: setAmOrPm(AM); break;
        case R::id::pm_label: setAmOrPm(PM); break;
        case R::id::hours:
            setCurrentItemShowing(HOUR_INDEX, true);
            break;
        case R::id::minutes:
            setCurrentItemShowing(MINUTE_INDEX, true);
            break;
        default:
            // Failed to handle this click, don't vibrate.
            return;
        }
        tryVibrate();
    }
}

void TimePickerClockDelegate::onViewClick(View& v) {
    switch (v.getId()) {
    case R::id::am_label: setAmOrPm(AM);  break;
    case R::id::pm_label: setAmOrPm(PM);  break;
    case R::id::hours:
        setCurrentItemShowing(HOUR_INDEX, true);
        break;
    case R::id::minutes:
        setCurrentItemShowing(MINUTE_INDEX, true);
        break;
    default:
        // Failed to handle this click, don't vibrate.
        return;
    }
    tryVibrate();
}

}/*endof namespace*/
