namespace cdroid{
class TimePickerClockDelegate:public TimePicker::AbstractTimePickerDelegate {
private:
    static constexpr long DELAY_COMMIT_MILLIS = 2000;

    static constexpr int FROM_EXTERNAL_API = 0;
    static constexpr int FROM_RADIAL_PICKER = 1;
    static constexpr int FROM_INPUT_PICKER = 2;

    // Index used by RadialPickerLayout
    static constexpr int HOUR_INDEX = RadialTimePickerView.HOURS;
    static constexpr int MINUTE_INDEX = RadialTimePickerView.MINUTES;

    static final int[] ATTRS_TEXT_COLOR = new int[] {R.attr.textColor};
    static final int[] ATTRS_DISABLED_ALPHA = new int[] {R.attr.disabledAlpha};

    static constexpr int AM = 0;
    static constexpr int PM = 1;

    static constexpr int HOURS_IN_HALF_DAY = 12;

    NumericTextView* mHourView;
    NumericTextView* mMinuteView;
    View* mAmPmLayout;
    RadioButton* mAmLabel;
    RadioButton* mPmLabel;
    RadialTimePickerView* mRadialTimePickerView;
    TextView* mSeparatorView;

    bool mRadialPickerModeEnabled = true;
    ImageButton* mRadialTimePickerModeButton;
    std::string mRadialTimePickerModeEnabledDescription;
    std::string mTextInputPickerModeEnabledDescription;
    View* mRadialTimePickerHeader;
    View* mTextInputPickerHeader;

    TextInputTimePickerView* mTextInputPickerView;

    Calendar mTempCalendar;

    // Accessibility strings.
    std::string mSelectHours;
    std::string mSelectMinutes;

    bool mIsEnabled = true;
    bool mAllowAutoAdvance;
    int mCurrentHour;
    int mCurrentMinute;
    bool mIs24Hour;

    // The portrait layout puts AM/PM at the right by default.
    bool mIsAmPmAtLeft = false;
    // The landscape layouts put AM/PM at the bottom by default.
    bool mIsAmPmAtTop = false;

    // Localization data.
    bool mHourFormatShowLeadingZero;
    bool mHourFormatStartsAtZero;
public:
    public TimePickerClockDelegate(TimePicker* delegator, Context* context,const AttributeSet& attrs);

    private void toggleRadialPickerMode();

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
    private static void ensureMinimumTextWidth(TextView* v);
    private void updateHourFormat();

    static final CharSequence obtainVerbatim(String text) {
        return new SpannableStringBuilder().append(text,
                new TtsSpan.VerbatimBuilder(text).build(), 0);
    }

    private ColorStateList* applyLegacyColorFixes(ColorStateList* color);

    private int multiplyAlphaComponent(int color, float alphaMod);

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

    private void initialize(int hourOfDay, int minute, bool is24HourView, int index);

    private void updateUI(int index);

    private void updateTextInputPicker() {
        mTextInputPickerView.updateTextInputValues(getLocalizedHour(mCurrentHour), mCurrentMinute,
                mCurrentHour < 12 ? AM : PM, mIs24Hour, mHourFormatStartsAtZero);
    }

    private void updateRadialPicker(int index) {
        mRadialTimePickerView.initialize(mCurrentHour, mCurrentMinute, mIs24Hour);
        setCurrentItemShowing(index, false);
    }

    private void updateHeaderAmPm();

    private void setAmPmStart(bool isAmPmAtStart);

    public void setDate(int hour, int minute) override;

    public void setHour(int hour) override;

    private void setHourInternal(int hour, int source, bool announce,bool notify);

    /**
     * @return the current hour in the range (0-23)
     */
    @Override
    public int getHour() override;

    @Override
    public void setMinute(int minute) override{
        setMinuteInternal(minute, FROM_EXTERNAL_API, true);
    }

    private void setMinuteInternal(int minute, @ChangeSource int source, bool notify) override;
    @Override
    public int getMinute() override;

    public void setIs24Hour(bool is24Hour);

    @Override
    public bool is24Hour() override;

    public void setEnabled(bool enabled) override;

    @Override
    public bool isEnabled() const{
        return mIsEnabled;
    }

    @Override
    public int getBaseline() {
        // does not support baseline alignment
        return -1;
    }

    public Parcelable onSaveInstanceState(Parcelable superState) override;
    public void onRestoreInstanceState(Parcelable state) override;
    public bool dispatchPopulateAccessibilityEvent(AccessibilityEvent& event) override;

    public void onPopulateAccessibilityEvent(AccessibilityEvent& event)override;

    public View* getHourView() override{
        return mHourView;
    }

    public View* getMinuteView() override;

    public View* getAmView() override;

    public View* getPmView() override;

    private int getCurrentItemShowing() {
        return mRadialTimePickerView.getCurrentItemShowing();
    }

    private void onTimeChanged();

    private void tryVibrate();

    private void updateAmPmLabelStates(int amOrPm);

    private int getLocalizedHour(int hourOfDay);

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

    private static String getHourMinSeparatorFromPattern(String dateTimePattern);

    static private int lastIndexOfAny(String str, char[] any);

    private void setCurrentItemShowing(int index, bool animateCircle);

    private void setAmOrPm(int amOrPm);

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
