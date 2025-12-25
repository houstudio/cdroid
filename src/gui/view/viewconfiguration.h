#ifndef __VIEW_CONFIGURATION_H__
#define __VIEW_CONFIGURATION_H__
#include <climits>
namespace cdroid{

class Context;

class ViewConfiguration{
private:
    static constexpr int SCROLL_BAR_SIZE = 4;

    /** Duration of the fade when scrollbars fade away in milliseconds */
    static constexpr int SCROLL_BAR_FADE_DURATION = 250;

    /* Default delay before the scrollbars fade in milliseconds */
    static constexpr int SCROLL_BAR_DEFAULT_DELAY = 300;

    /* Defines the length of the fading edges in dips*/
    static constexpr int FADING_EDGE_LENGTH = 12;

    /*Defines the duration in milliseconds of the pressed state in child components.*/
    static constexpr int PRESSED_STATE_DURATION = 64;

    /* Defines the default duration in milliseconds before a press turns into a long press*/
    static constexpr int DEFAULT_LONG_PRESS_TIMEOUT = 500;

    /* Defines the default duration in milliseconds between the first tap's up event and the second
     * tap's down event for an interaction to be considered part of the same multi-press.*/
    static constexpr int DEFAULT_MULTI_PRESS_TIMEOUT = 300;

    /* Defines the time between successive key repeats in milliseconds. */
    static constexpr int KEY_REPEAT_DELAY = 50;

    /* Defines the duration in milliseconds a user needs to hold down the
     * appropriate button to bring up the global actions dialog (power off,
     * lock screen, etc).*/
    static constexpr int GLOBAL_ACTIONS_KEY_TIMEOUT = 500;

    /* Defines the duration in milliseconds a user needs to hold down the
     * appropriate button to bring up the accessibility shortcut for the first time */
    static constexpr int A11Y_SHORTCUT_KEY_TIMEOUT = 3000;

    /* Defines the duration in milliseconds a user needs to hold down the
     * appropriate button to enable the accessibility shortcut once it's configured.*/
    static constexpr int A11Y_SHORTCUT_KEY_TIMEOUT_AFTER_CONFIRMATION = 1000;

    /* Defines the duration in milliseconds we will wait to see if a touch event
     * is a tap or a scroll. If the user does not move within this interval, it is
     * considered to be a tap.*/
    static constexpr int TAP_TIMEOUT = 100;

    /* Defines the duration in milliseconds we will wait to see if a touch event
     * is a jump tap. If the user does not complete the jump tap within this interval, it is
     * considered to be a tap.*/
    static constexpr int JUMP_TAP_TIMEOUT = 500;

    /* Defines the duration in milliseconds between the first tap's up event and
     * the second tap's down event for an interaction to be considered a double-tap.*/
    static constexpr int DOUBLE_TAP_TIMEOUT = 300;

    /* Defines the minimum duration in milliseconds between the first tap's up event and
     * the second tap's down event for an interaction to be considered a double-tap.*/
    static constexpr int DOUBLE_TAP_MIN_TIME = 40;

    /* Defines the maximum duration in milliseconds between a touch pad
     * touch and release for a given touch to be considered a tap (click) as
     * opposed to a hover movement gesture.*/
    static constexpr int HOVER_TAP_TIMEOUT = 150;

    /* Defines the maximum distance in pixels that a touch pad touch can move
     * before being released for it to be considered a tap (click) as opposed
     * to a hover movement gesture.*/
    static constexpr int HOVER_TAP_SLOP = 20;

    /* Defines the duration in milliseconds we want to display zoom controls in response
     * to a user panning within an application.*/
    static constexpr int ZOOM_CONTROLS_TIMEOUT = 3000;

    /* Inset in dips to look for touchable content when the user touches the edge of the screen */
    static constexpr int EDGE_SLOP = 12;

    /*Distance a touch can wander before we think the user is scrolling in dips.
     * Note that this value defined here is only used as a fallback by legacy/misbehaving
     * applications that do not provide a Context for determining density/configuration-dependent
     * values.
     *
     * To alter this value, see the configuration resource config_viewConfigurationTouchSlop
     * in frameworks/base/core/res/res/values/config.xml or the appropriate device resource overlay.
     * It may be appropriate to tweak this on a device-specific basis in an overlay based on
     * the characteristics of the touch panel and firmware.*/
    static constexpr int TOUCH_SLOP = 8;

    /* Defines the minimum size of the touch target for a scrollbar in dips*/
    static constexpr int MIN_SCROLLBAR_TOUCH_TARGET = 48;

    /* Distance the first touch can wander before we stop considering this event a double tap (in dips)*/
    static constexpr int DOUBLE_TAP_TOUCH_SLOP = TOUCH_SLOP;

    /* Distance a touch can wander before we think the user is attempting a paged scroll
     * (in dips)
     *
     * Note that this value defined here is only used as a fallback by legacy/misbehaving
     * applications that do not provide a Context for determining density/configuration-dependent
     * values.
     *
     * See the note above on {@link #TOUCH_SLOP} regarding the dimen resource
     * config_viewConfigurationTouchSlop. ViewConfiguration will report a paging touch slop of
     * config_viewConfigurationTouchSlop * 2 when provided with a Context. */
    static constexpr int PAGING_TOUCH_SLOP = TOUCH_SLOP * 2;

    /* Distance in dips between the first touch and second touch to still be considered a double tap */
    static constexpr int DOUBLE_TAP_SLOP = 100;

    /* Distance in dips a touch needs to be outside of a window's bounds for it to
     * count as outside for purposes of dismissing the window. */
    static constexpr int WINDOW_TOUCH_SLOP = 16;

    /* Minimum velocity to initiate a fling, as measured in dips per second */
    static constexpr int MINIMUM_FLING_VELOCITY = 50;

    /* Maximum velocity to initiate a fling, as measured in dips per second */
    static constexpr int MAXIMUM_FLING_VELOCITY = 8000;
    static constexpr int NO_FLING_MIN_VELOCITY = INT_MAX;
    static constexpr int NO_FLING_MAX_VELOCITY = INT_MIN;
    static constexpr int NO_HAPTIC_SCROLL_TICK_INTERVAL = INT_MAX;

    /* Delay before dispatching a recurring accessibility event in milliseconds.
     * This delay guarantees that a recurring event will be send at most once
     * during the {@link #SEND_RECURRING_ACCESSIBILITY_EVENTS_INTERVAL_MILLIS} time frame.*/
    static constexpr long SEND_RECURRING_ACCESSIBILITY_EVENTS_INTERVAL_MILLIS = 100;

    /* The maximum size of View's drawing cache, expressed in bytes. This size
     * should be at least equal to the size of the screen in ARGB888 format. */
    static constexpr int MAXIMUM_DRAWING_CACHE_SIZE = 480 * 800 * 4; // ARGB8888

    /* The coefficient of friction applied to flings/scrolls.*/
    static constexpr float SCROLL_FRICTION = 0.015f;

    /* Max distance in dips to overscroll for edge effects */
    static constexpr int OVERSCROLL_DISTANCE = 0;

    /* Max distance in dips to overfling for edge effects */
    static constexpr int OVERFLING_DISTANCE = 6;

    /* Amount to scroll in response to a horizontal {@link MotionEvent#ACTION_SCROLL} event,
     * in dips per axis value.*/
    static constexpr float HORIZONTAL_SCROLL_FACTOR = 64;

    /* Amount to scroll in response to a vertical {@link MotionEvent#ACTION_SCROLL} event,
     * in dips per axis value. */
    static constexpr float VERTICAL_SCROLL_FACTOR = 64;

    /* Default duration to hide an action mode for. */
    static constexpr long ACTION_MODE_HIDE_DURATION_DEFAULT = 2000;

    /* Defines the duration in milliseconds before an end of a long press causes a tooltip to be hidden.*/
    static constexpr int LONG_PRESS_TOOLTIP_HIDE_TIMEOUT = 1500;

    /* Defines the duration in milliseconds before a hover event causes a tooltip to be shown. */
    static constexpr int HOVER_TOOLTIP_SHOW_TIMEOUT = 500;

    /**
     * Defines the duration in milliseconds before mouse inactivity causes a tooltip to be hidden.
     * (default variant to be used when {@link View#SYSTEM_UI_FLAG_LOW_PROFILE} is not set). */
    static constexpr int HOVER_TOOLTIP_HIDE_TIMEOUT = 15000;

    /**
     * Defines the duration in milliseconds before mouse inactivity causes a tooltip to be hidden
     * (short version to be used when {@link View#SYSTEM_UI_FLAG_LOW_PROFILE} is set).
     */
    static constexpr int HOVER_TOOLTIP_HIDE_SHORT_TIMEOUT = 3000;
    static constexpr float AMBIGUOUS_GESTURE_MULTIPLIER = 2.f;
private:
    int mEdgeSlop;
    int mFadingEdgeLength;
    int mMinimumFlingVelocity;
    int mMaximumFlingVelocity;
    int mMinimumRotaryEncoderFlingVelocity;
    int mMaximumRotaryEncoderFlingVelocity;
    int mRotaryEncoderHapticScrollFeedbackTickIntervalPixels;
    int mScrollbarSize;
    int mTouchSlop;
    int mMinScalingSpan;
    int mHoverSlop;
    int mMinScrollbarTouchTarget;
    int mDoubleTapTouchSlop;
    int mPagingTouchSlop;
    int mDoubleTapSlop;
    int mWindowTouchSlop;
    int mMaximumDrawingCacheSize;
    int mOverscrollDistance;
    int mOverflingDistance;
    bool mFadingMarqueeEnabled;
    bool mPreferKeepClearForFocusEnabled;
    bool mShowMenuShortcutsWhenKeyboardPresent;
    bool sHasPermanentMenuKey;
    bool sHasPermanentMenuKeySet;
    bool mIsScreenRound;
    bool mRotaryEncoderHapticScrollFeedbackEnabled;
    bool mViewTouchScreenHapticScrollFeedbackEnabled;
    bool mViewBasedRotaryEncoderScrollHapticsEnabledConfig;
    long mGlobalActionsKeyTimeout;
    float mAmbiguousGestureMultiplier;
    float mVerticalScrollFactor;
    float mHorizontalScrollFactor;
    static ViewConfiguration*mInst;
    ViewConfiguration(Context*context);
    bool isInputDeviceInfoValid(int id, int axis, int source);
public:
    ViewConfiguration();
    static ViewConfiguration&get(Context*context);
    static bool isScreenRound();

    int getScaledScrollBarSize()const;
    /**
     * @return the minimum size of the scrollbar thumb's touch target in pixels
     * @hide
     */
    int getScaledMinScrollbarTouchTarget()const;
    /**
     * @return Duration of the fade when scrollbars fade away in milliseconds
     */
    static constexpr int getScrollBarFadeDuration(){return SCROLL_BAR_FADE_DURATION;}
    /**
     * @return Default delay before the scrollbars fade in milliseconds
     */
    static constexpr int getScrollDefaultDelay(){return SCROLL_BAR_DEFAULT_DELAY;}

    /**
     * @return the length of the fading edges in pixels
     */
    int getScaledFadingEdgeLength()const;
    /**
     * @return the duration in milliseconds of the pressed state in child
     * components.
     */
    static constexpr int getPressedStateDuration(){return PRESSED_STATE_DURATION;}
    /**
     * @return the duration in milliseconds before a press turns into
     * a long press
     */
    static constexpr int getLongPressTimeout(){return DEFAULT_LONG_PRESS_TIMEOUT;}
    /**
     * @return the duration in milliseconds between the first tap's up event and the second tap's
     * down event for an interaction to be considered part of the same multi-press.
     * @hide
     */
    static constexpr int getMultiPressTimeout(){return DEFAULT_MULTI_PRESS_TIMEOUT;}

    /**
     * @return the time before the first key repeat in milliseconds.
     */
    static constexpr int getKeyRepeatTimeout(){return getLongPressTimeout();}
    /**
     * @return the time between successive key repeats in milliseconds.
     */
    static constexpr int getKeyRepeatDelay(){return KEY_REPEAT_DELAY;}
    /**
     * @return the duration in milliseconds we will wait to see if a touch event
     * is a tap or a scroll. If the user does not move within this interval, it is
     * considered to be a tap.
     */
    static constexpr int getTapTimeout(){return TAP_TIMEOUT;}
    /**
     * @return the duration in milliseconds we will wait to see if a touch event
     * is a jump tap. If the user does not move within this interval, it is
     * considered to be a tap.
     */
    static constexpr int getJumpTapTimeout(){return JUMP_TAP_TIMEOUT;}
    /**
     * @return the duration in milliseconds between the first tap's up event and
     * the second tap's down event for an interaction to be considered a
     * double-tap.
     */
    static constexpr int getDoubleTapTimeout(){return DOUBLE_TAP_TIMEOUT;}
    /**
     * @return the minimum duration in milliseconds between the first tap's
     * up event and the second tap's down event for an interaction to be considered a
     * double-tap.
     *
     * @hide
     */
    static constexpr int getDoubleTapMinTime(){return DOUBLE_TAP_MIN_TIME;}
    /**
     * @return the maximum duration in milliseconds between a touch pad
     * touch and release for a given touch to be considered a tap (click) as
     * opposed to a hover movement gesture.
     * @hide
     */
    static constexpr int getHoverTapTimeout(){return HOVER_TAP_TIMEOUT;}
    /**
     * @return the maximum distance in pixels that a touch pad touch can move
     * before being released for it to be considered a tap (click) as opposed
     * to a hover movement gesture.
     * @hide
     */
    static constexpr int getHoverTapSlop(){return HOVER_TAP_SLOP;}

    /**
     * @return Inset in pixels to look for touchable content when the user touches the edge of the
     *         screen
     */
    int getScaledEdgeSlop()const;
    
    static constexpr int getTouchSlop(){ return TOUCH_SLOP; }
    /**
     * @return Distance in pixels a touch can wander before we think the user is scrolling
     */
    int getScaledTouchSlop()const;

    /**
     * @return Distance in pixels a hover can wander while it is still considered "stationary".
     *
     */
    int getScaledHoverSlop()const;

    static constexpr int getDoubleTapTouchSlop(){return DOUBLE_TAP_SLOP;}
    /**
     * @return Distance in pixels the first touch can wander before we do not consider this a
     * potential double tap event
     * @hide
     */
    int getScaledDoubleTapTouchSlop()const;

    /**
     * @return Distance in pixels a touch can wander before we think the user is scrolling a full
     * page
     */
    int getScaledPagingTouchSlop()const;

    static constexpr int getDoubleTapSlop(){return DOUBLE_TAP_SLOP;}
    /**
     * @return Distance in pixels between the first touch and second touch to still be
     *         considered a double tap
     */
    int getScaledDoubleTapSlop()const;
    /**
     * Interval for dispatching a recurring accessibility event in milliseconds.
     * This interval guarantees that a recurring event will be send at most once
     * during the {@link #getSendRecurringAccessibilityEventsInterval()} time frame.
     *
     * @return The delay in milliseconds.
     *
     * @hide
     */
    static long getSendRecurringAccessibilityEventsInterval();


    /**
     * @return Distance in pixels a touch must be outside the bounds of a window for it
     * to be counted as outside the window for purposes of dismissing that window.
     */
    int getScaledWindowTouchSlop()const;

    static constexpr int getMinimumFlingVelocity() {return MINIMUM_FLING_VELOCITY;}
    /**
     * @return Minimum velocity to initiate a fling, as measured in pixels per second.
     */
    int getScaledMinimumFlingVelocity()const;

    static constexpr int getMaximumFlingVelocity() { return MAXIMUM_FLING_VELOCITY;}
    /**
     * @return Maximum velocity to initiate a fling, as measured in pixels per second.
     */
    int getScaledMaximumFlingVelocity()const;
    int getScaledMinimumFlingVelocity(int inputDeviceId, int axis, int source);
    int getScaledMaximumFlingVelocity(int inputDeviceId, int axis, int source);
   /**
     * Checks if any kind of scroll haptic feedback is enabled for a motion generated by a specific
     * input device configuration and motion axis.
     *
     * <p>See {@link ScrollFeedbackProvider} for details on the arguments that should be passed to
     * the methods in this class.
     *
     * <p>If the provided input device ID, source, and motion axis are not supported by this Android
     * device, this method returns {@code false}. In other words, if the {@link InputDevice}
     * represented by the provided {code inputDeviceId} does not have a
     * {@link InputDevice.MotionRange} with the provided {@code axis} and {@code source}, the method
     * returns {@code false}.
     *
     * <p>If the provided input device ID, source, and motion axis are supported by this Android
     * device, this method returns {@code true} only if the provided arguments are supported for
     * scroll haptics. Otherwise, this method returns {@code false}.
     *
     * @param inputDeviceId the ID of the {@link InputDevice} that generated the motion that may
     *      produce scroll haptics.
     * @param source the input source of the motion that may produce scroll haptics.
     * @param axis the axis of the motion that may produce scroll haptics.
     * @return {@code true} if motions generated by the provided input and motion configuration
     *      can produce scroll haptics. {@code false} otherwise.
     *
     * @see #getHapticScrollFeedbackTickInterval(int, int, int)
     * @see InputDevice#getMotionRanges()
     * @see InputDevice#getMotionRange(int)
     * @see InputDevice#getMotionRange(int, int)
     *
     * @hide
     */
    bool isHapticScrollFeedbackEnabled(int inputDeviceId, int axis, int source);
    /**
     * Provides the minimum scroll interval (in pixels) between consecutive scroll tick haptics for
     * motions generated by a specific input device configuration and motion axis.
     *
     * <p><b>Scroll tick</b> here refers to an interval-based, consistent scroll feedback provided
     * to the user as the user scrolls through a scrollable view.
     *
     * <p>If you are supporting scroll tick haptics, use this interval as the minimum pixel scroll
     * distance between consecutive scroll ticks. That is, once your view has scrolled for at least
     * this interval, play a haptic, and wait again until the view has further scrolled with this
     * interval in the same direction before playing the next scroll haptic.
     *
     * <p>Some devices may support other types of scroll haptics but not interval based tick
     * haptics. In those cases, this method will return {@code Integer.MAX_VALUE}. The same value
     * will be returned if the device does not support scroll haptics at all (which can be checked
     * via {@link #isHapticScrollFeedbackEnabled(int, int, int)}).
     *
     * <p>See {@link #isHapticScrollFeedbackEnabled(int, int, int)} for more details about obtaining
     * the correct arguments for this method.
     *
     * @param inputDeviceId the ID of the {@link InputDevice} that generated the motion that may
     *      produce scroll haptics.
     * @param source the input source of the motion that may produce scroll haptics.
     * @param axis the axis of the motion that may produce scroll haptics.
     * @return the absolute value of the minimum scroll interval, in pixels, between consecutive
     *      scroll feedback haptics for motions generated by the provided input and motion
     *      configuration. If scroll haptics is disabled for the given configuration, or if the
     *      device does not support scroll tick haptics for the given configuration, this method
     *      returns {@code Integer.MAX_VALUE}.
     *
     * @see #isHapticScrollFeedbackEnabled(int, int, int)
     *
     * @hide
     */
    int getHapticScrollFeedbackTickInterval(int inputDeviceId, int axis, int source);
   /**
    * Checks if the View-based haptic scroll feedback implementation is enabled for
    * {@link InputDevice#SOURCE_ROTARY_ENCODER}s.
    *
    * @hide
    */
    bool isViewBasedRotaryEncoderHapticScrollFeedbackEnabled()const;
    /**
     * @return Amount to scroll in response to a {@link MotionEvent#ACTION_SCROLL} event. Multiply
     * this by the event's axis value to obtain the number of pixels to be scrolled.
     *
     * @removed
     */
    int getScaledScrollFactor()const;
    /**
     * @return Amount to scroll in response to a horizontal {@link MotionEvent#ACTION_SCROLL} event.
     * Multiply this by the event's axis value to obtain the number of pixels to be scrolled.
     */
    float getScaledHorizontalScrollFactor()const;

    /**
     * @return Amount to scroll in response to a vertical {@link MotionEvent#ACTION_SCROLL} event.
     * Multiply this by the event's axis value to obtain the number of pixels to be scrolled.
     */
    float getScaledVerticalScrollFactor()const;

    /**
     * The maximum drawing cache size expressed in bytes.
     *
     * @return the maximum size of View's drawing cache expressed in bytes
     */
    int getScaledMaximumDrawingCacheSize()const;

    /**
     * @return The maximum distance a View should overscroll by when showing edge effects (in
     * pixels).
     */
    int getScaledOverscrollDistance()const;
    /**
     * @return The maximum distance a View should overfling by when showing edge effects (in
     * pixels).
     */
    int getScaledOverflingDistance()const;
    /**
     * The amount of time that the zoom controls should be
     * displayed on the screen expressed in milliseconds.
     *
     * @return the time the zoom controls should be visible expressed
     * in milliseconds.
     */
    static long getZoomControlsTimeout();

    /**
     * The amount of time a user needs to press the relevant key to bring up
     * the global actions dialog.
     *
     * @return how long a user needs to press the relevant key to bring up
     *   the global actions dialog.
     * @hide
     */
    long getDeviceGlobalActionKeyTimeout()const;
    /**
     * The amount of time a user needs to press the relevant keys to activate the accessibility
     * shortcut.
     *
     * @return how long a user needs to press the relevant keys to activate the accessibility
     *   shortcut.
     * @hide
     */
    long getAccessibilityShortcutKeyTimeout()const;
    /**
     * @return The amount of time a user needs to press the relevant keys to activate the
     *   accessibility shortcut after it's confirmed that accessibility shortcut is used.
     * @hide
     */
    long getAccessibilityShortcutKeyTimeoutAfterConfirmation()const;

    /**
     * The amount of friction applied to scrolls and flings.
     *
     * @return A scalar dimensionless value representing the coefficient of
     *         friction.
     */
    static constexpr float getScrollFriction(){return SCROLL_FRICTION;}
    /**
     * The multiplication factor for inhibiting default gestures.
     *
     * If a MotionEvent has {@link android.view.MotionEvent#CLASSIFICATION_AMBIGUOUS_GESTURE} set,
     * then certain actions, such as scrolling, will be inhibited. However, to account for the
     * possibility of an incorrect classification, existing gesture thresholds (e.g. scrolling
     * touch slop and the long-press timeout) should be scaled by this factor and remain in effect.
     *
     * @deprecated Use {@link #getScaledAmbiguousGestureMultiplier()}.
     */
    static constexpr float getAmbiguousGestureMultiplier() {return AMBIGUOUS_GESTURE_MULTIPLIER;}
    float getScaledAmbiguousGestureMultiplier()const;
    /**
     * @return the default duration in milliseconds for {@link ActionMode#hide(long)}.
     */
    static constexpr long getDefaultActionModeHideDuration(){return ACTION_MODE_HIDE_DURATION_DEFAULT;}
    /**
     * Report if the device has a permanent menu key available to the user.
     *
     * <p>As of Android 3.0, devices may not have a permanent menu key available.
     * Apps should use the action bar to present menu options to users.
     * However, there are some apps where the action bar is inappropriate
     * or undesirable. This method may be used to detect if a menu key is present.
     * If not, applications should provide another on-screen affordance to access
     * functionality.
     *
     * @return true if a permanent menu key is present, false otherwise.
     */
    bool hasPermanentMenuKey()const;
    /**
     * Check if shortcuts should be displayed in menus.
     *
     * @return {@code True} if shortcuts should be displayed in menus.
     */
    bool shouldShowMenuShortcutsWhenKeyboardPresent()const;
    int getScaledMinimumScalingSpan()const;
    /**
     * @hide
     * @return Whether or not marquee should use fading edges.
     */
    bool isFadingMarqueeEnabled()const;
    bool isPreferKeepClearForFocusEnabled()const{return mPreferKeepClearForFocusEnabled;}
    /**
     * @return the duration in milliseconds before an end of a long press causes a tooltip to be
     * hidden
     * @hide
     */
    static constexpr int getLongPressTooltipHideTimeout(){return LONG_PRESS_TOOLTIP_HIDE_TIMEOUT;}
    /**
     * @return the duration in milliseconds before a hover event causes a tooltip to be shown
     * @hide
     */
    static constexpr int getHoverTooltipShowTimeout(){return HOVER_TOOLTIP_SHOW_TIMEOUT;}
    /**
     * @return the duration in milliseconds before mouse inactivity causes a tooltip to be hidden
     * (default variant to be used when {@link View#SYSTEM_UI_FLAG_LOW_PROFILE} is not set).
     * @hide
     */
    static constexpr int getHoverTooltipHideTimeout(){return HOVER_TOOLTIP_HIDE_TIMEOUT;}
    /**
     * @return the duration in milliseconds before mouse inactivity causes a tooltip to be hidden
     * (shorter variant to be used when {@link View#SYSTEM_UI_FLAG_LOW_PROFILE} is set).
     * @hide
     */
    static constexpr int getHoverTooltipHideShortTimeout(){return HOVER_TOOLTIP_HIDE_SHORT_TIMEOUT;}
};
}//end namespace
#endif
