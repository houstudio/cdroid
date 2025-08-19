#ifndef __TIMEPICKER_CLOCK_DELEGATE_H__
#define __TIMEPICKER_CLOCK_DELEGATE_H__
#include <core/calendar.h>
#include <widget/imagebutton.h>
#include <widget/radiobutton.h>
#include <widget/timepicker.h>
#include <widget/numberictextview.h>
namespace cdroid{
class RadialTimePickerView;
class TextInputTimePickerView;
class TimePickerClockDelegate:public TimePicker::AbstractTimePickerDelegate {
private:
    static constexpr long DELAY_COMMIT_MILLIS = 2000;

    static constexpr int FROM_EXTERNAL_API = 0;
    static constexpr int FROM_RADIAL_PICKER = 1;
    static constexpr int FROM_INPUT_PICKER = 2;

    // Index used by RadialPickerLayout
    static constexpr int HOUR_INDEX  = 0;//RadialTimePickerView.HOURS;
    static constexpr int MINUTE_INDEX= 1;// RadialTimePickerView.MINUTES;

    //static final int[] ATTRS_TEXT_COLOR = new int[] {R.attr.textColor};
    //static final int[] ATTRS_DISABLED_ALPHA = new int[] {R.attr.disabledAlpha};

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

    int mCurrentHour;
    int mCurrentMinute;
    bool mIsEnabled = true;
    bool mAllowAutoAdvance;
    bool mIs24Hour;

    // The portrait layout puts AM/PM at the right by default.
    bool mIsAmPmAtLeft = false;
    // The landscape layouts put AM/PM at the bottom by default.
    bool mIsAmPmAtTop = false;

    // Localization data.
    bool mHourFormatShowLeadingZero;
    bool mHourFormatStartsAtZero;
    Runnable mCommitHour;
    Runnable mCommitMinute;
private:
    void toggleRadialPickerMode();
    static void ensureMinimumTextWidth(TextView* v);
    void updateHourFormat();
    ColorStateList* applyLegacyColorFixes(ColorStateList* color);
    int  multiplyAlphaComponent(int color, float alphaMod);
    void initialize(int hourOfDay, int minute, bool is24HourView, int index);
    void updateUI(int index);
    void updateTextInputPicker();
    void updateRadialPicker(int index);
    void updateHeaderAmPm();
    void setAmPmStart(bool isAmPmAtStart);
    void setHourInternal(int hour, int source, bool announce,bool notify);
    void setMinuteInternal(int minute,int source, bool notify);
    int getCurrentItemShowing();
    void onTimeChanged();
    void tryVibrate();
    void updateAmPmLabelStates(int amOrPm);
    int getLocalizedHour(int hourOfDay);
    void updateHeaderHour(int hourOfDay, bool announce);
    void updateHeaderMinute(int minuteOfHour, bool announce);
    void updateHeaderSeparator();
    static std::string getHourMinSeparatorFromPattern(const std::string& dateTimePattern);
    static int lastIndexOfAny(const std::string& str, const std::string& any);
    void setCurrentItemShowing(int index, bool animateCircle);
    void setAmOrPm(int amOrPm);
    void onViewClick(View& v);
    void onViewFocusChange(View& v, bool focused);
public:
    TimePickerClockDelegate(TimePicker* delegator, Context* context,const AttributeSet& attrs);


    bool validateInput() override;


    static std::string obtainVerbatim(const std::string& text);


    /*private static class ClickActionDelegate extends AccessibilityDelegate {
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
    }*/


    void setDate(int hour, int minute) override;

    void setHour(int hour) override;
    int getHour() override;

    void setMinute(int minute) override;
    int getMinute() override;

    void setIs24Hour(bool is24Hour);

    bool is24Hour() override;

    void setEnabled(bool enabled) override;
    bool isEnabled() const;

    int getBaseline() override;

    Parcelable* onSaveInstanceState(Parcelable& superState) override;
    void onRestoreInstanceState(Parcelable& state) override;
    bool dispatchPopulateAccessibilityEvent(AccessibilityEvent& event) override;

    void onPopulateAccessibilityEvent(AccessibilityEvent& event)override;

    View* getHourView() override;
    View* getMinuteView() override;
    View* getAmView() override;
    View* getPmView() override;

#if 0
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
#endif
};
}/*endof namespace*/
#endif/*__TIMEPICKER_CLOCK_DELEGATE_H__*/
