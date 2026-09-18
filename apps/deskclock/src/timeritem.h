#ifndef __DESKCLOCK_TIMERITEM_H__
#define __DESKCLOCK_TIMERITEM_H__
/*********************************************************************************
 * Port of com.android.deskclock.timer.TimerItem — the visual representation of
 * one Timer: time text (blinks while paused), circle, reset/add-minute button,
 * and the label button.
 *********************************************************************************/
#include <widget/button.h>
#include <widget/linearlayout.h>
#include <widget/textview.h>

#include <timercircleview.h>
#include <timer.h>
#include <timertextcontroller.h>

namespace cdroid {
namespace deskclock {

class TimerItem : public LinearLayout {
private:
    /** Displays the remaining time or time since expiration. */
    TextView* mTimerText = nullptr;

    /** Formats and displays the text in the timer. */
    TimerTextController* mTimerTextController = nullptr;

    /** Displays timer progress as a color circle that changes from white to red. */
    TimerCircleView* mCircleView = nullptr;

    /** A button that either resets the timer or adds time to it, depending on its state. */
    Button* mResetAddButton = nullptr;

    /** Displays the label associated with the timer. Tapping it presents an edit dialog. */
    TextView* mLabelView = nullptr;

    /** The last state of the timer that was rendered; used to avoid expensive operations. */
    data::Timer::State mLastState = data::Timer::State::RESET;

    /** The last label rendered into mLabelView (see TimerItem::update). */
    std::string mLastLabel;

public:
    TimerItem(Context* context, const AttributeSet* attrs);
    ~TimerItem() override;

    /** Updates this view to display the latest state of the `timer`. */
    void update(const data::Timer& timer);

protected:
    void onFinishInflate() override;
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_TIMERITEM_H__
