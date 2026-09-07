#include <timersetupview.h>

#include <R.h>
#include <widget/internal_R.h>

#include <drawable/colorstatelist.h>
#include <core/bundle.h>
#include <view/keyevent.h>
#include <view/layoutinflater.h>
#include <content/resources.h>
#include <core/context.h>

#include <uidata.h>
#include <utils.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {

static constexpr int64_t SECOND_IN_MILLIS = 1000;
static constexpr int64_t MINUTE_IN_MILLIS = 60 * SECOND_IN_MILLIS;
static constexpr int64_t HOUR_IN_MILLIS = 60 * MINUTE_IN_MILLIS;

TimerSetupView::TimerSetupView(Context* context, const AttributeSet* attrs)
    : LinearLayout(context, attrs) {
    mHoursLabel = context->getString(R::string::hours_label);
    mMinutesLabel = context->getString(R::string::minutes_label);
    mSecondsLabel = context->getString(R::string::seconds_label);

    // Upstream: LayoutInflater.from(context).inflate(R.layout.timer_setup_container, this)
    // — attachToRoot=true, the guttered keypad becomes this view's children.
    LayoutInflater::from(context)->inflate(R::layout::timer_setup_container, this, true);
}

void TimerSetupView::onFinishInflate() {
    LinearLayout::onFinishInflate();

    mTimeView = (TextView*) findViewById(R::id::timer_setup_time);
    mDeleteView = findViewById(R::id::timer_setup_delete);
    mDividerView = findViewById(R::id::timer_setup_divider);
    mDigitViews[0] = (TextView*) findViewById(R::id::timer_setup_digit_0);
    mDigitViews[1] = (TextView*) findViewById(R::id::timer_setup_digit_1);
    mDigitViews[2] = (TextView*) findViewById(R::id::timer_setup_digit_2);
    mDigitViews[3] = (TextView*) findViewById(R::id::timer_setup_digit_3);
    mDigitViews[4] = (TextView*) findViewById(R::id::timer_setup_digit_4);
    mDigitViews[5] = (TextView*) findViewById(R::id::timer_setup_digit_5);
    mDigitViews[6] = (TextView*) findViewById(R::id::timer_setup_digit_6);
    mDigitViews[7] = (TextView*) findViewById(R::id::timer_setup_digit_7);
    mDigitViews[8] = (TextView*) findViewById(R::id::timer_setup_digit_8);
    mDigitViews[9] = (TextView*) findViewById(R::id::timer_setup_digit_9);

    // Tint the divider to match the disabled control color by default and use the
    // activated control color when there is valid input.
    Context& dividerContext = *mDividerView->getContext();
    int colorControlActivated = 0xFFFF5722;
    int colorControlDisabled = 0xFF757575;
    {
        TypedValue value;
        if (dividerContext.getTheme().resolveAttribute(
                0x01010436 /* android:colorControlActivated */, &value, true)) {
            colorControlActivated = value.data;
        }
        if (dividerContext.getTheme().resolveAttribute(
                0x01010437 /* android:colorControlNormal */, &value, true)) {
            colorControlDisabled = value.data;
        }
    }
    auto dividerTint = std::make_shared<ColorStateList>(
            std::vector<std::vector<int>>{{internal::R::attr::state_activated}, {}},
            std::vector<int>{colorControlActivated, colorControlDisabled});
    mDividerView->setBackgroundTintList(dividerTint);

    // Initialize the digit buttons.
    uidata::UiDataModel& uidm = uidata::UiDataModel::getUiDataModel();
    for (TextView* digitView : mDigitViews) {
        const int digit = getDigitForId(digitView->getId());
        digitView->setText(uidm.getFormattedNumber(digit, 1));
        digitView->setOnClickListener([this](View& v) { onClick(v); });
    }

    mDeleteView->setOnClickListener([this](View& v) { onClick(v); });
    mDeleteView->setOnLongClickListener([this](View& v) -> bool { return onLongClick(v); });

    updateTime();
    updateDeleteAndDivider();
}

bool TimerSetupView::onKeyDown(int keyCode, KeyEvent& event) {
    View* view = nullptr;
    if (keyCode == KeyEvent::KEYCODE_DEL) {
        view = mDeleteView;
    } else if (keyCode >= KeyEvent::KEYCODE_0 && keyCode <= KeyEvent::KEYCODE_9) {
        view = mDigitViews[keyCode - KeyEvent::KEYCODE_0];
    }

    if (view != nullptr) {
        const bool result = view->performClick();
        if (result && hasValidInput()) {
            mFabContainer->updateFab(FabContainer::FAB_REQUEST_FOCUS);
        }
        return result;
    }

    return false;
}

void TimerSetupView::onClick(View& view) {
    if (&view == mDeleteView) {
        deleteDigit();
    } else {
        append(getDigitForId(view.getId()));
    }
}

bool TimerSetupView::onLongClick(View& view) {
    if (&view == mDeleteView) {
        reset();
        updateFab();
        return true;
    }
    return false;
}

int TimerSetupView::getDigitForId(int id) const {
    if (id == R::id::timer_setup_digit_0) return 0;
    if (id == R::id::timer_setup_digit_1) return 1;
    if (id == R::id::timer_setup_digit_2) return 2;
    if (id == R::id::timer_setup_digit_3) return 3;
    if (id == R::id::timer_setup_digit_4) return 4;
    if (id == R::id::timer_setup_digit_5) return 5;
    if (id == R::id::timer_setup_digit_6) return 6;
    if (id == R::id::timer_setup_digit_7) return 7;
    if (id == R::id::timer_setup_digit_8) return 8;
    if (id == R::id::timer_setup_digit_9) return 9;
    return -1; // upstream throws; unreachable from the inflated keypad
}

void TimerSetupView::updateTime() {
    const int seconds = mInput[1] * 10 + mInput[0];
    const int minutes = mInput[3] * 10 + mInput[2];
    const int hours = mInput[5] * 10 + mInput[4];

    uidata::UiDataModel& uidm = uidata::UiDataModel::getUiDataModel();
    // Upstream expands a spanned template; see the header note about the labels.
    mTimeView->setText(uidm.getFormattedNumber(hours, 2) + mHoursLabel + " "
            + uidm.getFormattedNumber(minutes, 2) + mMinutesLabel + " "
            + uidm.getFormattedNumber(seconds, 2) + mSecondsLabel);

    Resources& r = getContext()->getResources();
    mTimeView->setContentDescription(r.getString(R::string::timer_setup_description,
            {r.getQuantityString(R::plurals::hours, hours,
                    {uidm.getFormattedNumber(hours)}),
             r.getQuantityString(R::plurals::minutes, minutes,
                    {uidm.getFormattedNumber(minutes)}),
             r.getQuantityString(R::plurals::seconds, seconds,
                    {uidm.getFormattedNumber(seconds)})}));
}

void TimerSetupView::updateDeleteAndDivider() {
    const bool enabled = hasValidInput();
    mDeleteView->setEnabled(enabled);
    mDividerView->setActivated(enabled);
}

void TimerSetupView::updateFab() {
    mFabContainer->updateFab(FabContainer::FAB_SHRINK_AND_EXPAND);
}

void TimerSetupView::append(int digit) {
    // Pressing "0" as the first digit does nothing.
    if (mInputPointer == -1 && digit == 0) {
        return;
    }

    // No space for more digits, so ignore input.
    if (mInputPointer == (int) (sizeof(mInput) / sizeof(mInput[0])) - 1) {
        return;
    }

    // Append the new digit.
    for (int i = mInputPointer + 1; i > 0; i--) {
        mInput[i] = mInput[i - 1];
    }
    mInput[0] = digit;
    mInputPointer++;
    updateTime();

    // Update TalkBack to read the number being deleted.
    mDeleteView->setContentDescription(getContext()->getResources().getString(
            R::string::timer_descriptive_delete,
            {uidata::UiDataModel::getUiDataModel().getFormattedNumber(digit)}));

    // Update the fab, delete, and divider when we have valid input.
    if (mInputPointer == 0) {
        updateFab();
        updateDeleteAndDivider();
    }
}

void TimerSetupView::deleteDigit() {
    // Nothing exists to delete so return.
    if (mInputPointer < 0) {
        return;
    }

    for (int i = 0; i < mInputPointer; i++) {
        mInput[i] = mInput[i + 1];
    }
    mInput[mInputPointer] = 0;
    mInputPointer--;
    updateTime();

    // Update TalkBack to read the number being deleted or its original description.
    if (mInputPointer >= 0) {
        mDeleteView->setContentDescription(getContext()->getResources().getString(
                R::string::timer_descriptive_delete,
                {uidata::UiDataModel::getUiDataModel().getFormattedNumber(mInput[0])}));
    } else {
        mDeleteView->setContentDescription(getContext()->getString(R::string::timer_delete));
    }

    // Update the fab, delete, and divider when we no longer have valid input.
    if (mInputPointer == -1) {
        updateFab();
        updateDeleteAndDivider();
    }
}

void TimerSetupView::reset() {
    if (mInputPointer != -1) {
        for (int& d : mInput) d = 0;
        mInputPointer = -1;
        updateTime();
        updateDeleteAndDivider();
    }
}

int64_t TimerSetupView::getTimeInMillis() const {
    const int seconds = mInput[1] * 10 + mInput[0];
    const int minutes = mInput[3] * 10 + mInput[2];
    const int hours = mInput[5] * 10 + mInput[4];
    return seconds * SECOND_IN_MILLIS + minutes * MINUTE_IN_MILLIS + hours * HOUR_IN_MILLIS;
}

std::vector<int> TimerSetupView::getState() const {
    return std::vector<int>(mInput, mInput + sizeof(mInput) / sizeof(mInput[0]));
}

void TimerSetupView::setState(const std::vector<int>& state) {
    const size_t size = sizeof(mInput) / sizeof(mInput[0]);
    if (state.size() == size) {
        for (size_t i = 0; i < size; i++) {
            mInput[i] = state[i];
            if (mInput[i] != 0) {
                mInputPointer = (int) i;
            }
        }
        updateTime();
        updateDeleteAndDivider();
    }
}

} // namespace deskclock

typedef cdroid::deskclock::TimerSetupView TimerSetupView;
DECLARE_WIDGET2(TimerSetupView, "TimerSetupView");

} // namespace cdroid
