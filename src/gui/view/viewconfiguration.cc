#include <math.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <view/viewconfiguration.h>
#include <core/displaymetrics.h>
#include <core/windowmanager.h>

namespace cdroid{
ViewConfiguration*ViewConfiguration::mInst=nullptr;
ViewConfiguration::ViewConfiguration(){
    mEdgeSlop = EDGE_SLOP;
    mFadingEdgeLength = FADING_EDGE_LENGTH;
    mMinimumFlingVelocity = MINIMUM_FLING_VELOCITY;
    mMaximumFlingVelocity = MAXIMUM_FLING_VELOCITY;
    mScrollbarSize = SCROLL_BAR_SIZE;
    mTouchSlop = TOUCH_SLOP;
    mHoverSlop = TOUCH_SLOP / 2;
    mMinScrollbarTouchTarget = MIN_SCROLLBAR_TOUCH_TARGET;
    mDoubleTapTouchSlop = DOUBLE_TAP_TOUCH_SLOP;
    mPagingTouchSlop = PAGING_TOUCH_SLOP;
    mDoubleTapSlop = DOUBLE_TAP_SLOP;
    mWindowTouchSlop = WINDOW_TOUCH_SLOP;
    //noinspection deprecation
    mMaximumDrawingCacheSize = MAXIMUM_DRAWING_CACHE_SIZE;
    mOverscrollDistance = OVERSCROLL_DISTANCE;
    mOverflingDistance  = OVERFLING_DISTANCE;
    mFadingMarqueeEnabled = true;
    mGlobalActionsKeyTimeout = GLOBAL_ACTIONS_KEY_TIMEOUT;
    mHorizontalScrollFactor = HORIZONTAL_SCROLL_FACTOR;
    mVerticalScrollFactor = VERTICAL_SCROLL_FACTOR;
    mShowMenuShortcutsWhenKeyboardPresent = false;
}

ViewConfiguration::ViewConfiguration(Context* context):ViewConfiguration(){
    DisplayMetrics metrics;
	AttributeSet atts(context,"");
    WindowManager::getInstance().getDefaultDisplay().getMetrics(metrics);
    const float sizeAndDensity = metrics.density;
    mEdgeSlop = (int) (sizeAndDensity * EDGE_SLOP + 0.5f);
	mFadingEdgeLength = int(sizeAndDensity*FADING_EDGE_LENGTH + 0.5f);
	
    mDoubleTapSlop = (int) (sizeAndDensity * DOUBLE_TAP_SLOP + 0.5f);
    mWindowTouchSlop = (int) (sizeAndDensity * WINDOW_TOUCH_SLOP + 0.5f);	
	mMaximumDrawingCacheSize = 4 * metrics.widthPixels * metrics.heightPixels;
    mOverscrollDistance = (int) (sizeAndDensity * OVERSCROLL_DISTANCE + 0.5f);
    mOverflingDistance = (int) (sizeAndDensity * OVERFLING_DISTANCE + 0.5f);

    if(atts.size()){
	    mScrollbarSize = atts.getDimensionPixelSize("config_scrollbarSize");
        mFadingMarqueeEnabled = atts.getBoolean("config_ui_enableFadingMarquee");
        mTouchSlop = atts.getDimensionPixelSize("config_viewConfigurationTouchSlop");
        mHoverSlop = atts.getDimensionPixelSize("config_viewConfigurationHoverSlop");
        mMinScrollbarTouchTarget = atts.getDimensionPixelSize("config_minScrollbarTouchTarget");
	}
	
	mPagingTouchSlop = mTouchSlop * 2;
	mDoubleTapTouchSlop = mTouchSlop;
    if(atts.size()){
        mMinimumFlingVelocity = atts.getDimensionPixelSize("config_viewMinFlingVelocity");
        mMaximumFlingVelocity = atts.getDimensionPixelSize("config_viewMaxFlingVelocity");
        mHorizontalScrollFactor = atts.getDimensionPixelSize("config_horizontalScrollFactor");
        mVerticalScrollFactor = atts.getDimensionPixelSize("config_verticalScrollFactor");
    }
}

ViewConfiguration& ViewConfiguration::get(Context*context){
    if(mInst==nullptr)
        mInst=new ViewConfiguration(context);
    return *mInst;
}

bool ViewConfiguration::isScreenRound(){
    return false;
}

int ViewConfiguration::getThumbLength(int size, int thickness, int extent, int range) {
    // Avoid the tiny thumb.
    int minLength = thickness * 2;
    int length = round((float) size * extent / range);
    if (length < minLength) {
        length = minLength;
    }
    return length;
}

int ViewConfiguration::getThumbOffset(int size, int thumbLength, int extent, int range, int offset) {
    // Avoid the too-big thumb.
    int thumbOffset = round((float) (size - thumbLength) * offset / (range - extent));
    if (thumbOffset > size - thumbLength) {
        thumbOffset = size - thumbLength;
    }
    return thumbOffset;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int ViewConfiguration::getScaledScrollBarSize() {
    return mScrollbarSize;
}

/**
 * @return the minimum size of the scrollbar thumb's touch target in pixels
 * @hide
 */
int ViewConfiguration::getScaledMinScrollbarTouchTarget() {
    return mMinScrollbarTouchTarget;
}

/**
 * @return Duration of the fade when scrollbars fade away in milliseconds
 */
int ViewConfiguration::getScrollBarFadeDuration() {
    return SCROLL_BAR_FADE_DURATION;
}

/**
 * @return Default delay before the scrollbars fade in milliseconds
 */
int ViewConfiguration::getScrollDefaultDelay() {
    return SCROLL_BAR_DEFAULT_DELAY;
}


/**
 * @return the length of the fading edges in pixels
 */
int ViewConfiguration::getScaledFadingEdgeLength() {
    return mFadingEdgeLength;
}

/**
 * @return the duration in milliseconds of the pressed state in child
 * components.
 */
int ViewConfiguration::getPressedStateDuration() {
    return PRESSED_STATE_DURATION;
}

/**
 * @return the duration in milliseconds before a press turns into
 * a long press
 */
int ViewConfiguration::getLongPressTimeout() {
    return DEFAULT_LONG_PRESS_TIMEOUT;
    //return AppGlobals.getIntCoreSetting(Settings.Secure.LONG_PRESS_TIMEOUT,DEFAULT_LONG_PRESS_TIMEOUT);
}

/**
 * @return the duration in milliseconds between the first tap's up event and the second tap's
 * down event for an interaction to be considered part of the same multi-press.
 * @hide
 */
int ViewConfiguration::getMultiPressTimeout() {
    return DEFAULT_MULTI_PRESS_TIMEOUT;
    //return AppGlobals.getIntCoreSetting(Settings.Secure.MULTI_PRESS_TIMEOUT, DEFAULT_MULTI_PRESS_TIMEOUT);
}

/**
 * @return the time before the first key repeat in milliseconds.
 */
int ViewConfiguration::getKeyRepeatTimeout() {
    return getLongPressTimeout();
}

/**
 * @return the time between successive key repeats in milliseconds.
 */
int ViewConfiguration::getKeyRepeatDelay() {
    return KEY_REPEAT_DELAY;
}

/**
 * @return the duration in milliseconds we will wait to see if a touch event
 * is a tap or a scroll. If the user does not move within this interval, it is
 * considered to be a tap.
 */
int ViewConfiguration::getTapTimeout() {
    return TAP_TIMEOUT;
}

/**
 * @return the duration in milliseconds we will wait to see if a touch event
 * is a jump tap. If the user does not move within this interval, it is
 * considered to be a tap.
 */
int ViewConfiguration::getJumpTapTimeout() {
    return JUMP_TAP_TIMEOUT;
}

/**
 * @return the duration in milliseconds between the first tap's up event and
 * the second tap's down event for an interaction to be considered a
 * double-tap.
 */
int ViewConfiguration::getDoubleTapTimeout() {
    return DOUBLE_TAP_TIMEOUT;
}

/**
 * @return the minimum duration in milliseconds between the first tap's
 * up event and the second tap's down event for an interaction to be considered a
 * double-tap.
 *
 * @hide
 */
int ViewConfiguration::getDoubleTapMinTime() {
    return DOUBLE_TAP_MIN_TIME;
}

/**
 * @return the maximum duration in milliseconds between a touch pad
 * touch and release for a given touch to be considered a tap (click) as
 * opposed to a hover movement gesture.
 * @hide
 */
int ViewConfiguration::getHoverTapTimeout() {
    return HOVER_TAP_TIMEOUT;
}

/**
 * @return the maximum distance in pixels that a touch pad touch can move
 * before being released for it to be considered a tap (click) as opposed
 * to a hover movement gesture.
 * @hide
 */
int ViewConfiguration::getHoverTapSlop() {
    return HOVER_TAP_SLOP;
}

/**
 * @return Inset in pixels to look for touchable content when the user touches the edge of the
 *         screen
 */
int ViewConfiguration::getScaledEdgeSlop() {
    return mEdgeSlop;
}


/**
 * @return Distance in pixels a touch can wander before we think the user is scrolling
 */
int ViewConfiguration::getScaledTouchSlop() {
    return mTouchSlop;
}

/**
 * @return Distance in pixels a hover can wander while it is still considered "stationary".
 *
 */
int ViewConfiguration::getScaledHoverSlop() {
    return mHoverSlop;
}

/**
 * @return Distance in pixels the first touch can wander before we do not consider this a
 * potential double tap event
 * @hide
 */
int ViewConfiguration::getScaledDoubleTapTouchSlop() {
    return mDoubleTapTouchSlop;
}

/**
 * @return Distance in pixels a touch can wander before we think the user is scrolling a full
 * page
 */
int ViewConfiguration::getScaledPagingTouchSlop() {
    return mPagingTouchSlop;
}


/**
 * @return Distance in pixels between the first touch and second touch to still be
 *         considered a double tap
 */
int ViewConfiguration::getScaledDoubleTapSlop() {
    return mDoubleTapSlop;
}

/**
 * Interval for dispatching a recurring accessibility event in milliseconds.
 * This interval guarantees that a recurring event will be send at most once
 * during the {@link #getSendRecurringAccessibilityEventsInterval()} time frame.
 *
 * @return The delay in milliseconds.
 *
 * @hide
 */
long ViewConfiguration::getSendRecurringAccessibilityEventsInterval() {
    return SEND_RECURRING_ACCESSIBILITY_EVENTS_INTERVAL_MILLIS;
}


/**
 * @return Distance in pixels a touch must be outside the bounds of a window for it
 * to be counted as outside the window for purposes of dismissing that window.
 */
int ViewConfiguration::getScaledWindowTouchSlop() {
    return mWindowTouchSlop;
}


/**
 * @return Minimum velocity to initiate a fling, as measured in pixels per second.
 */
int ViewConfiguration::getScaledMinimumFlingVelocity() {
    return mMinimumFlingVelocity;
}


/**
 * @return Maximum velocity to initiate a fling, as measured in pixels per second.
 */
int ViewConfiguration::getScaledMaximumFlingVelocity() {
    return mMaximumFlingVelocity;
}

/**
 * @return Amount to scroll in response to a {@link MotionEvent#ACTION_SCROLL} event. Multiply
 * this by the event's axis value to obtain the number of pixels to be scrolled.
 *
 * @removed
 */
int ViewConfiguration::getScaledScrollFactor() {
    return (int) mVerticalScrollFactor;
}

/**
 * @return Amount to scroll in response to a horizontal {@link MotionEvent#ACTION_SCROLL} event.
 * Multiply this by the event's axis value to obtain the number of pixels to be scrolled.
 */
float ViewConfiguration::getScaledHorizontalScrollFactor() {
    return mHorizontalScrollFactor;
}

/**
 * @return Amount to scroll in response to a vertical {@link MotionEvent#ACTION_SCROLL} event.
 * Multiply this by the event's axis value to obtain the number of pixels to be scrolled.
 */
float ViewConfiguration::getScaledVerticalScrollFactor() {
    return mVerticalScrollFactor;
}


/**
 * The maximum drawing cache size expressed in bytes.
 *
 * @return the maximum size of View's drawing cache expressed in bytes
 */
int ViewConfiguration::getScaledMaximumDrawingCacheSize() {
    return mMaximumDrawingCacheSize;
}

/**
 * @return The maximum distance a View should overscroll by when showing edge effects (in
 * pixels).
 */
int ViewConfiguration::getScaledOverscrollDistance() {
    return mOverscrollDistance;
}

/**
 * @return The maximum distance a View should overfling by when showing edge effects (in
 * pixels).
 */
int ViewConfiguration::getScaledOverflingDistance() {
    return mOverflingDistance;
}

/**
 * The amount of time that the zoom controls should be
 * displayed on the screen expressed in milliseconds.
 *
 * @return the time the zoom controls should be visible expressed
 * in milliseconds.
 */
long ViewConfiguration::getZoomControlsTimeout() {
    return ZOOM_CONTROLS_TIMEOUT;
}


/**
 * The amount of time a user needs to press the relevant key to bring up
 * the global actions dialog.
 *
 * @return how long a user needs to press the relevant key to bring up
 *   the global actions dialog.
 * @hide
 */
long ViewConfiguration::getDeviceGlobalActionKeyTimeout() {
    return mGlobalActionsKeyTimeout;
}

/**
 * The amount of time a user needs to press the relevant keys to activate the accessibility
 * shortcut.
 *
 * @return how long a user needs to press the relevant keys to activate the accessibility
 *   shortcut.
 * @hide
 */
long ViewConfiguration::getAccessibilityShortcutKeyTimeout() {
    return A11Y_SHORTCUT_KEY_TIMEOUT;
}

/**
 * @return The amount of time a user needs to press the relevant keys to activate the
 *   accessibility shortcut after it's confirmed that accessibility shortcut is used.
 * @hide
 */
long ViewConfiguration::getAccessibilityShortcutKeyTimeoutAfterConfirmation() {
    return A11Y_SHORTCUT_KEY_TIMEOUT_AFTER_CONFIRMATION;
}

/**
 * The amount of friction applied to scrolls and flings.
 *
 * @return A scalar dimensionless value representing the coefficient of
 *         friction.
 */
float ViewConfiguration::getScrollFriction() {
    return SCROLL_FRICTION;
}

/**
 * @return the default duration in milliseconds for {@link ActionMode#hide(long)}.
 */
long ViewConfiguration::getDefaultActionModeHideDuration() {
    return ACTION_MODE_HIDE_DURATION_DEFAULT;
}

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
bool ViewConfiguration::hasPermanentMenuKey() {
    return sHasPermanentMenuKey;
}

/**
 * Check if shortcuts should be displayed in menus.
 *
 * @return {@code True} if shortcuts should be displayed in menus.
 */
bool ViewConfiguration::shouldShowMenuShortcutsWhenKeyboardPresent() {
    return mShowMenuShortcutsWhenKeyboardPresent;
}

/**
 * @hide
 * @return Whether or not marquee should use fading edges.
 */
bool ViewConfiguration::isFadingMarqueeEnabled() {
    return mFadingMarqueeEnabled;
}

/**
 * @return the duration in milliseconds before an end of a long press causes a tooltip to be
 * hidden
 * @hide
 */
int ViewConfiguration::getLongPressTooltipHideTimeout() {
    return LONG_PRESS_TOOLTIP_HIDE_TIMEOUT;
}

/**
 * @return the duration in milliseconds before a hover event causes a tooltip to be shown
 * @hide
 */
int ViewConfiguration::getHoverTooltipShowTimeout() {
    return HOVER_TOOLTIP_SHOW_TIMEOUT;
}

/**
 * @return the duration in milliseconds before mouse inactivity causes a tooltip to be hidden
 * (default variant to be used when {@link View#SYSTEM_UI_FLAG_LOW_PROFILE} is not set).
 * @hide
 */
int ViewConfiguration::getHoverTooltipHideTimeout() {
    return HOVER_TOOLTIP_HIDE_TIMEOUT;
}

/**
 * @return the duration in milliseconds before mouse inactivity causes a tooltip to be hidden
 * (shorter variant to be used when {@link View#SYSTEM_UI_FLAG_LOW_PROFILE} is set).
 * @hide
 */
int ViewConfiguration::getHoverTooltipHideShortTimeout() {
    return HOVER_TOOLTIP_HIDE_SHORT_TIMEOUT;
}

}//namespace

