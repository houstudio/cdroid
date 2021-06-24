#ifndef __VIEW_CONFIGURATION_H__
#define __VIEW_CONFIGURATION_H__
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
    static constexpr int MAXIMUM_FLING_VELOCITY = 4000;

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
private:
    int mEdgeSlop;
    int mFadingEdgeLength;
    int mMinimumFlingVelocity;
    int mMaximumFlingVelocity;
    int mScrollbarSize;
    int mTouchSlop;
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
    long mGlobalActionsKeyTimeout;
    float mVerticalScrollFactor;
    float mHorizontalScrollFactor;
    bool mShowMenuShortcutsWhenKeyboardPresent;
    bool sHasPermanentMenuKey;
    bool sHasPermanentMenuKeySet;
    static ViewConfiguration*mInst;
    ViewConfiguration(Context*context);
public:
    ViewConfiguration();
    static ViewConfiguration&get(Context*context);
    static bool isScreenRound();
    static int getThumbLength(int size, int thickness, int extent, int range);
    static int getThumbOffset(int size, int thumbLength, int extent, int range, int offset);

    int getScaledScrollBarSize();
    /**
     * @return the minimum size of the scrollbar thumb's touch target in pixels
     * @hide
     */
    int getScaledMinScrollbarTouchTarget();
    /**
     * @return Duration of the fade when scrollbars fade away in milliseconds
     */
    static int getScrollBarFadeDuration();
    /**
     * @return Default delay before the scrollbars fade in milliseconds
     */
    static int getScrollDefaultDelay();

    /**
     * @return the length of the fading edges in pixels
     */
    int getScaledFadingEdgeLength();
    /**
     * @return the duration in milliseconds of the pressed state in child
     * components.
     */
    static int getPressedStateDuration();
    /**
     * @return the duration in milliseconds before a press turns into
     * a long press
     */
    static int getLongPressTimeout();
    /**
     * @return the duration in milliseconds between the first tap's up event and the second tap's
     * down event for an interaction to be considered part of the same multi-press.
     * @hide
     */
    static int getMultiPressTimeout();

    /**
     * @return the time before the first key repeat in milliseconds.
     */
    static int getKeyRepeatTimeout();
    /**
     * @return the time between successive key repeats in milliseconds.
     */
    static int getKeyRepeatDelay();
    /**
     * @return the duration in milliseconds we will wait to see if a touch event
     * is a tap or a scroll. If the user does not move within this interval, it is
     * considered to be a tap.
     */
    static int getTapTimeout();
    /**
     * @return the duration in milliseconds we will wait to see if a touch event
     * is a jump tap. If the user does not move within this interval, it is
     * considered to be a tap.
     */
    static int getJumpTapTimeout();
    /**
     * @return the duration in milliseconds between the first tap's up event and
     * the second tap's down event for an interaction to be considered a
     * double-tap.
     */
    static int getDoubleTapTimeout();
    /**
     * @return the minimum duration in milliseconds between the first tap's
     * up event and the second tap's down event for an interaction to be considered a
     * double-tap.
     *
     * @hide
     */
    static int getDoubleTapMinTime();
    /**
     * @return the maximum duration in milliseconds between a touch pad
     * touch and release for a given touch to be considered a tap (click) as
     * opposed to a hover movement gesture.
     * @hide
     */
    static int getHoverTapTimeout();
    /**
     * @return the maximum distance in pixels that a touch pad touch can move
     * before being released for it to be considered a tap (click) as opposed
     * to a hover movement gesture.
     * @hide
     */
    static int getHoverTapSlop();

    /**
     * @return Inset in pixels to look for touchable content when the user touches the edge of the
     *         screen
     */
    int getScaledEdgeSlop();

    /**
     * @return Distance in pixels a touch can wander before we think the user is scrolling
     */
    int getScaledTouchSlop();

    /**
     * @return Distance in pixels a hover can wander while it is still considered "stationary".
     *
     */
    int getScaledHoverSlop();

    /**
     * @return Distance in pixels the first touch can wander before we do not consider this a
     * potential double tap event
     * @hide
     */
    int getScaledDoubleTapTouchSlop();

    /**
     * @return Distance in pixels a touch can wander before we think the user is scrolling a full
     * page
     */
    int getScaledPagingTouchSlop();

    /**
     * @return Distance in pixels between the first touch and second touch to still be
     *         considered a double tap
     */
    int getScaledDoubleTapSlop();
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
    int getScaledWindowTouchSlop();

    /**
     * @return Minimum velocity to initiate a fling, as measured in pixels per second.
     */
    int getScaledMinimumFlingVelocity();


    /**
     * @return Maximum velocity to initiate a fling, as measured in pixels per second.
     */
    int getScaledMaximumFlingVelocity();
    /**
     * @return Amount to scroll in response to a {@link MotionEvent#ACTION_SCROLL} event. Multiply
     * this by the event's axis value to obtain the number of pixels to be scrolled.
     *
     * @removed
     */
    int getScaledScrollFactor();
    /**
     * @return Amount to scroll in response to a horizontal {@link MotionEvent#ACTION_SCROLL} event.
     * Multiply this by the event's axis value to obtain the number of pixels to be scrolled.
     */
    float getScaledHorizontalScrollFactor();

    /**
     * @return Amount to scroll in response to a vertical {@link MotionEvent#ACTION_SCROLL} event.
     * Multiply this by the event's axis value to obtain the number of pixels to be scrolled.
     */
    float getScaledVerticalScrollFactor();

    /**
     * The maximum drawing cache size expressed in bytes.
     *
     * @return the maximum size of View's drawing cache expressed in bytes
     */
    int getScaledMaximumDrawingCacheSize();

    /**
     * @return The maximum distance a View should overscroll by when showing edge effects (in
     * pixels).
     */
    int getScaledOverscrollDistance();
    /**
     * @return The maximum distance a View should overfling by when showing edge effects (in
     * pixels).
     */
    int getScaledOverflingDistance();
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
    long getDeviceGlobalActionKeyTimeout();
    /**
     * The amount of time a user needs to press the relevant keys to activate the accessibility
     * shortcut.
     *
     * @return how long a user needs to press the relevant keys to activate the accessibility
     *   shortcut.
     * @hide
     */
    long getAccessibilityShortcutKeyTimeout();
    /**
     * @return The amount of time a user needs to press the relevant keys to activate the
     *   accessibility shortcut after it's confirmed that accessibility shortcut is used.
     * @hide
     */
    long getAccessibilityShortcutKeyTimeoutAfterConfirmation();

    /**
     * The amount of friction applied to scrolls and flings.
     *
     * @return A scalar dimensionless value representing the coefficient of
     *         friction.
     */
    static float getScrollFriction();

    /**
     * @return the default duration in milliseconds for {@link ActionMode#hide(long)}.
     */
    static long getDefaultActionModeHideDuration();
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
    bool hasPermanentMenuKey();
    /**
     * Check if shortcuts should be displayed in menus.
     *
     * @return {@code True} if shortcuts should be displayed in menus.
     */
    bool shouldShowMenuShortcutsWhenKeyboardPresent();
    /**
     * @hide
     * @return Whether or not marquee should use fading edges.
     */
    bool isFadingMarqueeEnabled();
    /**
     * @return the duration in milliseconds before an end of a long press causes a tooltip to be
     * hidden
     * @hide
     */
    static int getLongPressTooltipHideTimeout();
    /**
     * @return the duration in milliseconds before a hover event causes a tooltip to be shown
     * @hide
     */
    static int getHoverTooltipShowTimeout();
    /**
     * @return the duration in milliseconds before mouse inactivity causes a tooltip to be hidden
     * (default variant to be used when {@link View#SYSTEM_UI_FLAG_LOW_PROFILE} is not set).
     * @hide
     */
    static int getHoverTooltipHideTimeout();
    /**
     * @return the duration in milliseconds before mouse inactivity causes a tooltip to be hidden
     * (shorter variant to be used when {@link View#SYSTEM_UI_FLAG_LOW_PROFILE} is set).
     * @hide
     */
    static int getHoverTooltipHideShortTimeout();
};
}//end namespace
#endif
