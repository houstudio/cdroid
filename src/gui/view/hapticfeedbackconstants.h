#ifndef __HAPTIC_FEEDBACK_CONSTANTS_H__
#define __HAPTIC_FEEDBACK_CONSTANTS_H__
namespace cdroid{
	
class HapticFeedbackConstants {
public:

    /**
     * The user has performed a long press on an object that is resulting
     * in an action being performed.
     */
     static constexpr int LONG_PRESS = 0;

    /**
     * The user has pressed on a virtual on-screen key.
     */
     static constexpr int VIRTUAL_KEY = 1;

    /**
     * The user has pressed a soft keyboard key.
     */
     static constexpr int KEYBOARD_TAP = 3;

    /**
     * The user has pressed either an hour or minute tick of a Clock.
     */
     static constexpr int CLOCK_TICK = 4;

    /**
     * The user has pressed either a day or month or year date of a Calendar.
     * @hide
     */
     static constexpr int CALENDAR_DATE = 5;

    /**
     * The user has performed a context click on an object.
     */
     static constexpr int CONTEXT_CLICK = 6;

    /**
     * The user has pressed a virtual or software keyboard key.
     */
     static constexpr int KEYBOARD_PRESS = KEYBOARD_TAP;

    /**
     * The user has released a virtual keyboard key.
     */
     static constexpr int KEYBOARD_RELEASE = 7;

    /**
     * The user has released a virtual key.
     */
     static constexpr int VIRTUAL_KEY_RELEASE = 8;

    /**
     * The user has performed a selection/insertion handle move on text field.
     */
     static constexpr int TEXT_HANDLE_MOVE = 9;

    /**
     * The user unlocked the device
     * @hide
     */
     static constexpr int ENTRY_BUMP = 10;

    /**
     * The user has moved the dragged object within a droppable area.
     * @hide
     */
     static constexpr int DRAG_CROSSING = 11;

    /**
     * The user has started a gesture (e.g. on the soft keyboard).
     */
     static constexpr int GESTURE_START = 12;

    /**
     * The user has finished a gesture (e.g. on the soft keyboard).
     */
     static constexpr int GESTURE_END = 13;

    /**
     * The user's squeeze crossed the gesture's initiation threshold.
     * @hide
     */
     static constexpr int EDGE_SQUEEZE = 14;

    /**
     * The user's squeeze crossed the gesture's release threshold.
     * @hide
     */
     static constexpr int EDGE_RELEASE = 15;

    /**
     * A haptic effect to signal the confirmation or successful completion of a user
     * interaction.
     */
     static constexpr int CONFIRM = 16;

    /**
     * A haptic effect to signal the rejection or failure of a user interaction.
     */
     static constexpr int REJECT = 17;

    /**
     * A haptic effect to provide texture while a rotary input device is being scrolled.
     *
     * @hide
     */
     static constexpr int ROTARY_SCROLL_TICK = 18;

    /**
     * A haptic effect to signal that a list element has been focused while scrolling using a rotary
     * input device.
     *
     * @hide
     */
     static constexpr int ROTARY_SCROLL_ITEM_FOCUS = 19;

    /**
     * A haptic effect to signal reaching the scrolling limits of a list while scrolling using a
     * rotary input device.
     *
     * @hide
     */
     static constexpr int ROTARY_SCROLL_LIMIT = 20;

    /**
     * The phone has booted with safe mode enabled.
     * This is a private constant.  Feel free to renumber as desired.
     * @hide
     */
     static constexpr int SAFE_MODE_ENABLED = 10001;

    /**
     * Invocation of the voice assistant via hardware button.
     * This is a private constant.  Feel free to renumber as desired.
     * @hide
     */
     static constexpr int ASSISTANT_BUTTON = 10002;

    /**
     * The user has performed a long press on the power button hardware that is resulting
     * in an action being performed.
     * This is a private constant.  Feel free to renumber as desired.
     * @hide
     */
     static constexpr int LONG_PRESS_POWER_BUTTON = 10003;

    /**
     * Flag for {@link View#performHapticFeedback(int, int)
     * View.performHapticFeedback(int, int)}: Ignore the setting in the
     * view for whether to perform haptic feedback, do it always.
     */
     static constexpr int FLAG_IGNORE_VIEW_SETTING = 0x0001;

    /**
     * Flag for {@link View#performHapticFeedback(int, int)
     * View.performHapticFeedback(int, int)}: Ignore the global setting
     * for whether to perform haptic feedback, do it always.
     *
     * @deprecated Starting from {@link android.os.Build.VERSION_CODES#TIRAMISU} only privileged
     * apps can ignore user settings for touch feedback.
     */
     static constexpr int FLAG_IGNORE_GLOBAL_SETTING = 0x0002;
};
}/*endof namespace*/
#endif
