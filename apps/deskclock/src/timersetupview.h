#ifndef __DESKCLOCK_TIMERSETUPVIEW_H__
#define __DESKCLOCK_TIMERSETUPVIEW_H__
/*********************************************************************************
 * Port of com.android.deskclock.timer.TimerSetupView — the 4x3 keypad used to
 * enter a new timer's length ("00h 00m 00s" with a delete key).
 *
 * CDROID note: upstream builds the time display as a CharSequence whose h/m/s
 * labels carry RelativeSizeSpan(0.5); cdroid TextView has no public spanned
 * setText (see the TextClock am/pm precedent), so the template renders as a
 * plain string. TODO: restore the half-size labels with the spanned face.
 *********************************************************************************/
#include <vector>

#include <widget/linearlayout.h>
#include <widget/textview.h>

#include <fabcontainer.h>

namespace cdroid {
namespace deskclock {

class TimerSetupView : public LinearLayout {
private:
    int mInput[6] = {0, 0, 0, 0, 0, 0};

    int mInputPointer = -1;

    /** "00h 00m 00s" — the h/m/s labels come from the (short) label resources. */
    std::string mHoursLabel, mMinutesLabel, mSecondsLabel;

    TextView* mTimeView = nullptr;
    View* mDeleteView = nullptr;
    View* mDividerView = nullptr;
    TextView* mDigitViews[10];

    /** Updates to the fab are requested via this container. */
    FabContainer* mFabContainer = nullptr;

public:
    TimerSetupView(Context* context, const AttributeSet* attrs);

    void setFabContainer(FabContainer* fabContainer) { mFabContainer = fabContainer; }

    bool onKeyDown(int keyCode, KeyEvent& event) override;

    void reset();

    bool hasValidInput() const { return mInputPointer != -1; }

    int64_t getTimeInMillis() const;

    /**
     * @return an opaque representation of the state of timer setup
     *         (upstream Serializable; a copy of the digit input).
     */
    std::vector<int> getState() const;

    /** @param state an opaque state previously produced by getState(). */
    void setState(const std::vector<int>& state);

protected:
    void onFinishInflate() override;

private:
    void onClick(View& view);
    bool onLongClick(View& view);

    int getDigitForId(int id) const;

    void updateTime();
    void updateDeleteAndDivider();
    void updateFab();
    void append(int digit);
    void deleteDigit();
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_TIMERSETUPVIEW_H__
