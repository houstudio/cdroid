class TimePickerClockDelegate extends TimePicker.AbstractTimePickerDelegate {
    /**
     * Delay in milliseconds before valid but potentially incomplete, for
     * example "1" but not "12", keyboard edits are propagated from the
     * hour / minute fields to the radial picker.
     */
    private static constexpr long DELAY_COMMIT_MILLIS = 2000;

    private static constexpr int FROM_EXTERNAL_API = 0;
    private static constexpr int FROM_RADIAL_PICKER = 1;
    private static constexpr int FROM_INPUT_PICKER = 2;

    // Index used by RadialPickerLayout
    private static constexpr int HOUR_INDEX = RadialTimePickerView.HOURS;
    private static constexpr int MINUTE_INDEX = RadialTimePickerView.MINUTES;

    private static final int[] ATTRS_TEXT_COLOR = new int[] {R.attr.textColor};
    private static final int[] ATTRS_DISABLED_ALPHA = new int[] {R.attr.disabledAlpha};

    private static constexpr int AM = 0;
    private static constexpr int PM = 1;

    private static constexpr int HOURS_IN_HALF_DAY = 12;

    private NumericTextView* mHourView;
    private NumericTextView* mMinuteView;
    private View* mAmPmLayout;
    private RadioButton* mAmLabel;
    private RadioButton* mPmLabel;
    private RadialTimePickerView* mRadialTimePickerView;
    private TextView* mSeparatorView;

    private bool mRadialPickerModeEnabled = true;
    private ImageButton* mRadialTimePickerModeButton;
    private String mRadialTimePickerModeEnabledDescription;
    private String mTextInputPickerModeEnabledDescription;
    private View* mRadialTimePickerHeader;
    private View* mTextInputPickerHeader;

    private TextInputTimePickerView* mTextInputPickerView;

    private Calendar mTempCalendar;

    // Accessibility strings.
    private String mSelectHours;
    private String mSelectMinutes;

    private bool mIsEnabled = true;
    private bool mAllowAutoAdvance;
    private int mCurrentHour;
    private int mCurrentMinute;
    private bool mIs24Hour;

    // The portrait layout puts AM/PM at the right by default.
    private bool mIsAmPmAtLeft = false;
    // The landscape layouts put AM/PM at the bottom by default.
    private bool mIsAmPmAtTop = false;

    // Localization data.
    private bool mHourFormatShowLeadingZero;

    private bool mHourFormatStartsAtZero;
    public TimePickerClockDelegate(TimePicker* delegator, Context* context,const AttributeSet& attrs)
        :AbstractTimePickerDelegate(delegator, context){

        // process style attributes
        final TypedArray a = mContext.obtainStyledAttributes(attrs,
                R.styleable.TimePicker, defStyleAttr, defStyleRes);
        final LayoutInflater inflater = (LayoutInflater) mContext.getSystemService(
                Context.LAYOUT_INFLATER_SERVICE);
        final Resources res = mContext.getResources();

        mSelectHours = res.getString(R.string.select_hours);
        mSelectMinutes = res.getString(R.string.select_minutes);

        final int layoutResourceId = a.getResourceId(R.styleable.TimePicker_internalLayout,
                R.layout.time_picker_material);
        final View mainView = inflater.inflate(layoutResourceId, delegator);
        mainView.setSaveFromParentEnabled(false);
        mRadialTimePickerHeader = mainView->findViewById(R::id::time_header);
        mRadialTimePickerHeader.setOnTouchListener(new NearestTouchDelegate());

        // Set up hour/minute labels.
        mHourView = (NumericTextView*) mainView->findViewById(R::id::hours);
        mHourView->setOnClickListener(mClickListener);
        mHourView->setOnFocusChangeListener(mFocusListener);
        mHourView->setOnDigitEnteredListener(mDigitEnteredListener);
        //mHourView->setAccessibilityDelegate(new ClickActionDelegate(context, R.string.select_hours));
        mHourView->setAccessibilityLiveRegion(View::ACCESSIBILITY_LIVE_REGION_POLITE);
        mSeparatorView = (TextView) mainView->findViewById(R::id::separator);
        mMinuteView = (NumericTextView) mainView->findViewById(R::id::minutes);
        mMinuteView->setOnClickListener(mClickListener);
        mMinuteView->setOnFocusChangeListener(mFocusListener);
        mMinuteView->setOnDigitEnteredListener(mDigitEnteredListener);
        //mMinuteView->setAccessibilityDelegate(new ClickActionDelegate(context, R.string.select_minutes));
        mMinuteView->setAccessibilityLiveRegion(View.ACCESSIBILITY_LIVE_REGION_POLITE);
        mMinuteView->setRange(0, 59);

        // Set up AM/PM labels.
        mAmPmLayout = mainView.findViewById(R.id.ampm_layout);
        mAmPmLayout.setOnTouchListener(new NearestTouchDelegate());

        final String[] amPmStrings = TimePicker.getAmPmStrings(context);
        mAmLabel = (RadioButton) mAmPmLayout.findViewById(R.id.am_label);
        mAmLabel.setText(obtainVerbatim(amPmStrings[0]));
        mAmLabel.setOnClickListener(mClickListener);
        ensureMinimumTextWidth(mAmLabel);

        mPmLabel = (RadioButton) mAmPmLayout.findViewById(R.id.pm_label);
        mPmLabel.setText(obtainVerbatim(amPmStrings[1]));
        mPmLabel.setOnClickListener(mClickListener);
        ensureMinimumTextWidth(mPmLabel);

        // For the sake of backwards compatibility, attempt to extract the text
        // color from the header time text appearance. If it's set, we'll let
        // that override the "real" header text color.
        ColorStateList headerTextColor = null;

        final int timeHeaderTextAppearance = a.getResourceId(
                R.styleable.TimePicker_headerTimeTextAppearance, 0);
        if (timeHeaderTextAppearance != 0) {
            final TypedArray textAppearance = mContext.obtainStyledAttributes(null,
                    ATTRS_TEXT_COLOR, 0, timeHeaderTextAppearance);
            final ColorStateList legacyHeaderTextColor = textAppearance.getColorStateList(0);
            headerTextColor = applyLegacyColorFixes(legacyHeaderTextColor);
            textAppearance.recycle();
        }

        if (headerTextColor == null) {
            headerTextColor = a.getColorStateList(R.styleable.TimePicker_headerTextColor);
        }

        mTextInputPickerHeader = mainView.findViewById(R.id.input_header);

        if (headerTextColor != null) {
            mHourView.setTextColor(headerTextColor);
            mSeparatorView.setTextColor(headerTextColor);
            mMinuteView.setTextColor(headerTextColor);
            mAmLabel.setTextColor(headerTextColor);
            mPmLabel.setTextColor(headerTextColor);
        }

        // Set up header background, if available.
        if (a.hasValueOrEmpty(R.styleable.TimePicker_headerBackground)) {
            mRadialTimePickerHeader.setBackground(a.getDrawable(
                    R.styleable.TimePicker_headerBackground));
            mTextInputPickerHeader.setBackground(a.getDrawable(
                    R.styleable.TimePicker_headerBackground));
        }

        a.recycle();

        mRadialTimePickerView = (RadialTimePickerView*) mainView->findViewById(R.id.radial_picker);
        mRadialTimePickerView->applyAttributes(attrs, defStyleAttr, defStyleRes);
        mRadialTimePickerView->setOnValueSelectedListener(mOnValueSelectedListener);

        mTextInputPickerView = (TextInputTimePickerView*) mainView->findViewById(R.id.input_mode);
        mTextInputPickerView->setListener(mOnValueTypedListener);

        mRadialTimePickerModeButton =
                (ImageButton*) mainView->findViewById(R::id::toggle_mode);
        mRadialTimePickerModeButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                toggleRadialPickerMode();
            }
        });
        mRadialTimePickerModeEnabledDescription = context.getResources().getString(
                R.string.time_picker_radial_mode_description);
        mTextInputPickerModeEnabledDescription = context.getResources().getString(
                R.string.time_picker_text_input_mode_description);

        mAllowAutoAdvance = true;

        updateHourFormat();

        // Initialize with current time.
        mTempCalendar = Calendar.getInstance(mLocale);
        const int currentHour = mTempCalendar.get(Calendar::HOUR_OF_DAY);
        const int currentMinute = mTempCalendar.get(Calendar::MINUTE);
        initialize(currentHour, currentMinute, mIs24Hour, HOUR_INDEX);
    }

    private void toggleRadialPickerMode() {
        if (mRadialPickerModeEnabled) {
            mRadialTimePickerView->setVisibility(View::GONE);
            mRadialTimePickerHeader->setVisibility(View::GONE);
            mTextInputPickerHeader->setVisibility(View::VISIBLE);
            mTextInputPickerView->setVisibility(View::VISIBLE);
            mRadialTimePickerModeButton->setImageResource(R.drawable.btn_clock_material);
            mRadialTimePickerModeButton->setContentDescription(
                    mRadialTimePickerModeEnabledDescription);
            mRadialPickerModeEnabled = false;
        } else {
            mRadialTimePickerView->setVisibility(View::VISIBLE);
            mRadialTimePickerHeader->setVisibility(View::VISIBLE);
            mTextInputPickerHeader->setVisibility(View::GONE);
            mTextInputPickerView->setVisibility(View::GONE);
            mRadialTimePickerModeButton->setImageResource(R.drawable.btn_keyboard_key_material);
            mRadialTimePickerModeButton->setContentDescription(
                    mTextInputPickerModeEnabledDescription);
            updateTextInputPicker();
            InputMethodManager imm = mContext.getSystemService(InputMethodManager.class);
            if (imm != null) {
                imm.hideSoftInputFromWindow(mDelegator.getWindowToken(), 0);
            }
            mRadialPickerModeEnabled = true;
        }
    }

    @Override
    public bool validateInput() {
        return mTextInputPickerView->validateInput();
    }

    /**
     * Ensures that a TextView is wide enough to contain its text without
     * wrapping or clipping. Measures the specified view and sets the minimum
     * width to the view's desired width.
     *
     * @param v the text view to measure
     */
    private static void ensureMinimumTextWidth(TextView* v) {
        v->measure(MeasureSpec::UNSPECIFIED, MeasureSpec::UNSPECIFIED);

        // Set both the TextView and the View version of minimum
        // width because they are subtly different.
        const int minWidth = v->getMeasuredWidth();
        v->setMinWidth(minWidth);
        v->setMinimumWidth(minWidth);
    }

    /**
     * Updates hour formatting based on the current locale and 24-hour mode.
     * <p>
     * Determines how the hour should be formatted, sets member variables for
     * leading zero and starting hour, and sets the hour view's presentation.
     */
    private void updateHourFormat() {
        final String bestDateTimePattern = DateFormat.getBestDateTimePattern(
                mLocale, mIs24Hour ? "Hm" : "hm");
        final int lengthPattern = bestDateTimePattern.length();
        bool showLeadingZero = false;
        char hourFormat = '\0';

        for (int i = 0; i < lengthPattern; i++) {
            final char c = bestDateTimePattern.charAt(i);
            if (c == 'H' || c == 'h' || c == 'K' || c == 'k') {
                hourFormat = c;
                if (i + 1 < lengthPattern && c == bestDateTimePattern.charAt(i + 1)) {
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
        mHourView.setRange(minHour, maxHour);
        mHourView.setShowLeadingZeroes(mHourFormatShowLeadingZero);

        final String[] digits = DecimalFormatSymbols.getInstance(mLocale).getDigitStrings();
        int maxCharLength = 0;
        for (int i = 0; i < 10; i++) {
            maxCharLength = Math.max(maxCharLength, digits[i].length());
        }
        mTextInputPickerView.setHourFormat(maxCharLength * 2);
    }

    static final CharSequence obtainVerbatim(String text) {
        return new SpannableStringBuilder().append(text,
                new TtsSpan.VerbatimBuilder(text).build(), 0);
    }

    private ColorStateList* applyLegacyColorFixes(ColorStateList* color) {
        if (color == null || color.hasState(R.attr.state_activated)) {
            return color;
        }

        final int activatedColor;
        final int defaultColor;
        if (color.hasState(R.attr.state_selected)) {
            activatedColor = color.getColorForState(StateSet.get(
                    StateSet::VIEW_STATE_ENABLED | StateSet::VIEW_STATE_SELECTED), 0);
            defaultColor = color.getColorForState(StateSet.get(
                    StateSet.VIEW_STATE_ENABLED), 0);
        } else {
            activatedColor = color.getDefaultColor();

            // Generate a non-activated color using the disabled alpha.
            final TypedArray ta = mContext.obtainStyledAttributes(ATTRS_DISABLED_ALPHA);
            final float disabledAlpha = ta.getFloat(0, 0.30f);
            ta.recycle();
            defaultColor = multiplyAlphaComponent(activatedColor, disabledAlpha);
        }

        if (activatedColor == 0 || defaultColor == 0) {
            // We somehow failed to obtain the colors.
            return null;
        }

        final int[][] stateSet = new int[][] {{ R.attr.state_activated }, {}};
        final int[] colors = new int[] { activatedColor, defaultColor };
        return new ColorStateList(stateSet, colors);
    }

    private int multiplyAlphaComponent(int color, float alphaMod) {
        const int srcRgb = color & 0xFFFFFF;
        const int srcAlpha = (color >> 24) & 0xFF;
        const int dstAlpha = (int) (srcAlpha * alphaMod + 0.5f);
        return srcRgb | (dstAlpha << 24);
    }

    private static class ClickActionDelegate extends AccessibilityDelegate {
        private final AccessibilityAction mClickAction;

        public ClickActionDelegate(Context context, int resId) {
            mClickAction = new AccessibilityAction(
                    AccessibilityNodeInfo.ACTION_CLICK, context.getString(resId));
        }

        @Override
        public void onInitializeAccessibilityNodeInfo(View host, AccessibilityNodeInfo info) {
            super.onInitializeAccessibilityNodeInfo(host, info);

            info.addAction(mClickAction);
        }
    }

    private void initialize(int hourOfDay, int minute, bool is24HourView, int index) {
        mCurrentHour = hourOfDay;
        mCurrentMinute = minute;
        mIs24Hour = is24HourView;
        updateUI(index);
    }

    private void updateUI(int index) {
        updateHeaderAmPm();
        updateHeaderHour(mCurrentHour, false);
        updateHeaderSeparator();
        updateHeaderMinute(mCurrentMinute, false);
        updateRadialPicker(index);
        updateTextInputPicker();

        mDelegator.invalidate();
    }

    private void updateTextInputPicker() {
        mTextInputPickerView.updateTextInputValues(getLocalizedHour(mCurrentHour), mCurrentMinute,
                mCurrentHour < 12 ? AM : PM, mIs24Hour, mHourFormatStartsAtZero);
    }

    private void updateRadialPicker(int index) {
        mRadialTimePickerView.initialize(mCurrentHour, mCurrentMinute, mIs24Hour);
        setCurrentItemShowing(index, false);
    }

    private void updateHeaderAmPm() {
        if (mIs24Hour) {
            mAmPmLayout.setVisibility(View.GONE);
        } else {
            // Find the location of AM/PM based on locale information.
            final String dateTimePattern = DateFormat.getBestDateTimePattern(mLocale, "hm");
            const bool isAmPmAtStart = dateTimePattern.startsWith("a");
            setAmPmStart(isAmPmAtStart);
            updateAmPmLabelStates(mCurrentHour < 12 ? AM : PM);
        }
    }

    private void setAmPmStart(bool isAmPmAtStart) {
        RelativeLayout::LayoutParams* params =
                (RelativeLayout::LayoutParams) mAmPmLayout->getLayoutParams();
        if (params->getRule(RelativeLayout::RIGHT_OF) != 0
                || params.getRule(RelativeLayout.LEFT_OF) != 0) {
            const int margin = (int) (mContext.getResources().getDisplayMetrics().density * 8);
            // Horizontal mode, with AM/PM appearing to left/right of hours and minutes.
            final bool isAmPmAtLeft;
            if (TextUtils.getLayoutDirectionFromLocale(mLocale) == View::LAYOUT_DIRECTION_LTR) {
                isAmPmAtLeft = isAmPmAtStart;
            } else {
                isAmPmAtLeft = !isAmPmAtStart;
            }

            if (isAmPmAtLeft) {
                params->removeRule(RelativeLayout.RIGHT_OF);
                params->addRule(RelativeLayout.LEFT_OF, mHourView.getId());
            } else {
                params->removeRule(RelativeLayout.LEFT_OF);
                params->addRule(RelativeLayout.RIGHT_OF, mMinuteView.getId());
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
                otherViewId = params.getRule(RelativeLayout::ABOVE);
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

        mAmPmLayout.setLayoutParams(params);
    }

    @Override
    public void setDate(int hour, int minute) {
        setHourInternal(hour, FROM_EXTERNAL_API, true, false);
        setMinuteInternal(minute, FROM_EXTERNAL_API, false);

        onTimeChanged();
    }

    @Override
    public void setHour(int hour) {
        setHourInternal(hour, FROM_EXTERNAL_API, true, true);
    }

    private void setHourInternal(int hour, int source, bool announce,
            bool notify) {
        if (mCurrentHour == hour) {
            return;
        }

        resetAutofilledValue();
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

        mDelegator.invalidate();
        if (notify) {
            onTimeChanged();
        }
    }

    /**
     * @return the current hour in the range (0-23)
     */
    @Override
    public int getHour() {
        const int currentHour = mRadialTimePickerView->getCurrentHour();
        if (mIs24Hour) {
            return currentHour;
        }

        if (mRadialTimePickerView.getAmOrPm() == PM) {
            return (currentHour % HOURS_IN_HALF_DAY) + HOURS_IN_HALF_DAY;
        } else {
            return currentHour % HOURS_IN_HALF_DAY;
        }
    }

    @Override
    public void setMinute(int minute) {
        setMinuteInternal(minute, FROM_EXTERNAL_API, true);
    }

    private void setMinuteInternal(int minute, @ChangeSource int source, bool notify) {
        if (mCurrentMinute == minute) {
            return;
        }

        resetAutofilledValue();
        mCurrentMinute = minute;
        updateHeaderMinute(minute, true);

        if (source != FROM_RADIAL_PICKER) {
            mRadialTimePickerView.setCurrentMinute(minute);
        }
        if (source != FROM_INPUT_PICKER) {
            updateTextInputPicker();
        }

        mDelegator->invalidate();
        if (notify) {
            onTimeChanged();
        }
    }

    @Override
    public int getMinute() {
        return mRadialTimePickerView.getCurrentMinute();
    }

    public void setIs24Hour(bool is24Hour) {
        if (mIs24Hour != is24Hour) {
            mIs24Hour = is24Hour;
            mCurrentHour = getHour();

            updateHourFormat();
            updateUI(mRadialTimePickerView->getCurrentItemShowing());
        }
    }

    @Override
    public bool is24Hour() {
        return mIs24Hour;
    }

    @Override
    public void setEnabled(bool enabled) {
        mHourView->setEnabled(enabled);
        mMinuteView->setEnabled(enabled);
        mAmLabel->setEnabled(enabled);
        mPmLabel->setEnabled(enabled);
        mRadialTimePickerView->setEnabled(enabled);
        mIsEnabled = enabled;
    }

    @Override
    public bool isEnabled() const{
        return mIsEnabled;
    }

    @Override
    public int getBaseline() {
        // does not support baseline alignment
        return -1;
    }

    @Override
    public Parcelable onSaveInstanceState(Parcelable superState) {
        return new SavedState(superState, getHour(), getMinute(),
                is24Hour(), getCurrentItemShowing());
    }

    @Override
    public void onRestoreInstanceState(Parcelable state) {
        if (state instanceof SavedState) {
            final SavedState ss = (SavedState) state;
            initialize(ss.getHour(), ss.getMinute(), ss.is24HourMode(), ss.getCurrentItemShowing());
            mRadialTimePickerView.invalidate();
        }
    }

    @Override
    public bool dispatchPopulateAccessibilityEvent(AccessibilityEvent event) {
        onPopulateAccessibilityEvent(event);
        return true;
    }

    @Override
    public void onPopulateAccessibilityEvent(AccessibilityEvent event) {
        int flags = DateUtils.FORMAT_SHOW_TIME;
        if (mIs24Hour) {
            flags |= DateUtils.FORMAT_24HOUR;
        } else {
            flags |= DateUtils.FORMAT_12HOUR;
        }

        mTempCalendar.set(Calendar::HOUR_OF_DAY, getHour());
        mTempCalendar.set(Calendar::MINUTE, getMinute());

        final String selectedTime = DateUtils.formatDateTime(mContext,
                mTempCalendar.getTimeInMillis(), flags);
        final String selectionMode = mRadialTimePickerView.getCurrentItemShowing() == HOUR_INDEX ?
                mSelectHours : mSelectMinutes;
        event.getText().add(selectedTime + " " + selectionMode);
    }

    @Override
    public View* getHourView() {
        return mHourView;
    }

    @Override
    public View* getMinuteView() {
        return mMinuteView;
    }

    @Override
    public View* getAmView() {
        return mAmLabel;
    }

    @Override
    public View* getPmView() {
        return mPmLabel;
    }

    private int getCurrentItemShowing() {
        return mRadialTimePickerView.getCurrentItemShowing();
    }

    private void onTimeChanged() {
        mDelegator.sendAccessibilityEvent(AccessibilityEvent.TYPE_VIEW_SELECTED);
        if (mOnTimeChangedListener != null) {
            mOnTimeChangedListener.onTimeChanged(mDelegator, getHour(), getMinute());
        }
        if (mAutoFillChangeListener != null) {
            mAutoFillChangeListener.onTimeChanged(mDelegator, getHour(), getMinute());
        }
    }

    private void tryVibrate() {
        mDelegator.performHapticFeedback(HapticFeedbackConstants.CLOCK_TICK);
    }

    private void updateAmPmLabelStates(int amOrPm) {
        final bool isAm = amOrPm == AM;
        mAmLabel.setActivated(isAm);
        mAmLabel.setChecked(isAm);

        final bool isPm = amOrPm == PM;
        mPmLabel.setActivated(isPm);
        mPmLabel.setChecked(isPm);
    }

    private int getLocalizedHour(int hourOfDay) {
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

    private void updateHeaderHour(int hourOfDay, bool announce) {
        final int localizedHour = getLocalizedHour(hourOfDay);
        mHourView.setValue(localizedHour);
    }

    private void updateHeaderMinute(int minuteOfHour, bool announce) {
        mMinuteView.setValue(minuteOfHour);
    }

    private void updateHeaderSeparator() {
        final String bestDateTimePattern = DateFormat.getBestDateTimePattern(mLocale,
                (mIs24Hour) ? "Hm" : "hm");
        final String separatorText = getHourMinSeparatorFromPattern(bestDateTimePattern);
        mSeparatorView.setText(separatorText);
        mTextInputPickerView.updateSeparator(separatorText);
    }

    private static String getHourMinSeparatorFromPattern(String dateTimePattern) {
        final String defaultSeparator = ":";
        bool foundHourPattern = false;
        for (int i = 0; i < dateTimePattern.length(); i++) {
            switch (dateTimePattern.charAt(i)) {
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
                    if (!foundHourPattern) {
                        continue;
                    }
                    SpannableStringBuilder quotedSubstring = new SpannableStringBuilder(
                            dateTimePattern.substring(i));
                    int quotedTextLength = DateFormat.appendQuotedText(quotedSubstring, 0);
                    return quotedSubstring.subSequence(0, quotedTextLength).toString();
                default:
                    if (!foundHourPattern) {
                        continue;
                    }
                    return Character.toString(dateTimePattern.charAt(i));
            }
        }
        return defaultSeparator;
    }

    static private int lastIndexOfAny(String str, char[] any) {
        final int lengthAny = any.length;
        if (lengthAny > 0) {
            for (int i = str.length() - 1; i >= 0; i--) {
                char c = str.charAt(i);
                for (int j = 0; j < lengthAny; j++) {
                    if (c == any[j]) {
                        return i;
                    }
                }
            }
        }
        return -1;
    }

    private void setCurrentItemShowing(int index, bool animateCircle) {
        mRadialTimePickerView.setCurrentItemShowing(index, animateCircle);

        mHourView.setActivated(index == HOUR_INDEX);
        mMinuteView.setActivated(index == MINUTE_INDEX);
    }

    private void setAmOrPm(int amOrPm) {
        updateAmPmLabelStates(amOrPm);

        if (mRadialTimePickerView.setAmOrPm(amOrPm)) {
            mCurrentHour = getHour();
            updateTextInputPicker();
            if (mOnTimeChangedListener != null) {
                mOnTimeChangedListener.onTimeChanged(mDelegator, getHour(), getMinute());
            }
        }
    }

    private final OnValueSelectedListener mOnValueSelectedListener = new OnValueSelectedListener() {
        @Override
        public void onValueSelected(int pickerType, int newValue, bool autoAdvance) {
            bool valueChanged = false;
            switch (pickerType) {
                case RadialTimePickerView.HOURS:
                    if (getHour() != newValue) {
                        valueChanged = true;
                    }
                    final bool isTransition = mAllowAutoAdvance && autoAdvance;
                    setHourInternal(newValue, FROM_RADIAL_PICKER, !isTransition, true);
                    if (isTransition) {
                        setCurrentItemShowing(MINUTE_INDEX, true);
                    }
                    break;
                case RadialTimePickerView.MINUTES:
                    if (getMinute() != newValue) {
                        valueChanged = true;
                    }
                    setMinuteInternal(newValue, FROM_RADIAL_PICKER, true);
                    break;
            }

            if (mOnTimeChangedListener != null && valueChanged) {
                mOnTimeChangedListener.onTimeChanged(mDelegator, getHour(), getMinute());
            }
        }
    };

    private final OnValueTypedListener mOnValueTypedListener = new OnValueTypedListener() {
        @Override
        public void onValueChanged(int pickerType, int newValue) {
            switch (pickerType) {
                case TextInputTimePickerView.HOURS:
                    setHourInternal(newValue, FROM_INPUT_PICKER, false, true);
                    break;
                case TextInputTimePickerView.MINUTES:
                    setMinuteInternal(newValue, FROM_INPUT_PICKER, true);
                    break;
                case TextInputTimePickerView.AMPM:
                    setAmOrPm(newValue);
                    break;
            }
        }
    };

    private final OnValueChangedListener mDigitEnteredListener = new OnValueChangedListener() {
        @Override
        public void onValueChanged(NumericTextView view, int value,
                bool isValid, bool isFinished) {
            final Runnable commitCallback;
            final View nextFocusTarget;
            if (view == mHourView) {
                commitCallback = mCommitHour;
                nextFocusTarget = view.isFocused() ? mMinuteView : null;
            } else if (view == mMinuteView) {
                commitCallback = mCommitMinute;
                nextFocusTarget = null;
            } else {
                return;
            }

            view.removeCallbacks(commitCallback);

            if (isValid) {
                if (isFinished) {
                    // Done with hours entry, make visual updates
                    // immediately and move to next focus if needed.
                    commitCallback.run();

                    if (nextFocusTarget != null) {
                        nextFocusTarget.requestFocus();
                    }
                } else {
                    // May still be making changes. Postpone visual
                    // updates to prevent distracting the user.
                    view.postDelayed(commitCallback, DELAY_COMMIT_MILLIS);
                }
            }
        }
    };

    private final Runnable mCommitHour = new Runnable() {
        @Override
        public void run() {
            setHour(mHourView.getValue());
        }
    };

    private final Runnable mCommitMinute = new Runnable() {
        @Override
        public void run() {
            setMinute(mMinuteView.getValue());
        }
    };

    private final View.OnFocusChangeListener mFocusListener = new View.OnFocusChangeListener() {
        @Override
        public void onFocusChange(View& v, bool focused) {
            if (focused) {
                switch (v.getId()) {
                    case R::id::am_label:
                        setAmOrPm(AM);
                        break;
                    case R::id::pm_label:
                        setAmOrPm(PM);
                        break;
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
    };

    private final View.OnClickListener mClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            int amOrPm;
            switch (v.getId()) {
                case R::id::am_label:
                    setAmOrPm(AM);
                    break;
                case R::id::pm_label:
                    setAmOrPm(PM);
                    break;
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
    };

    private static class NearestTouchDelegate implements View.OnTouchListener {
            private View mInitialTouchTarget;

            @Override
            public bool onTouch(View& view, MotionEvent& motionEvent) {
                final int actionMasked = motionEvent.getActionMasked();
                if (actionMasked == MotionEvent::ACTION_DOWN) {
                    if (view instanceof ViewGroup) {
                        mInitialTouchTarget = findNearestChild((ViewGroup*) view,
                                (int) motionEvent.getX(), (int) motionEvent.getY());
                    } else {
                        mInitialTouchTarget = null;
                    }
                }

                View* child = mInitialTouchTarget;
                if (child == nullptr) {
                    return false;
                }

                const float offsetX = view.getScrollX() - child.getLeft();
                const float offsetY = view.getScrollY() - child.getTop();
                motionEvent.offsetLocation(offsetX, offsetY);
                const bool handled = child.dispatchTouchEvent(motionEvent);
                motionEvent.offsetLocation(-offsetX, -offsetY);

                if (actionMasked == MotionEvent::ACTION_UP
                        || actionMasked == MotionEvent::ACTION_CANCEL) {
                    mInitialTouchTarget = null;
                }

                return handled;
            }

        private View* findNearestChild(ViewGroup* v, int x, int y) {
            View bestChild = null;
            int bestDist = INT_MAX;

            for (int i = 0, count = v.getChildCount(); i < count; i++) {
                final View child = v.getChildAt(i);
                final int dX = x - (child.getLeft() + child.getWidth() / 2);
                final int dY = y - (child.getTop() + child.getHeight() / 2);
                final int dist = dX * dX + dY * dY;
                if (bestDist > dist) {
                    bestChild = child;
                    bestDist = dist;
                }
            }

            return bestChild;
        }
    }
}
