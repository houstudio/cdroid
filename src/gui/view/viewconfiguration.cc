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
    mIsScreenRound = false;
    mFadingEdgeLength = FADING_EDGE_LENGTH;
    mMinimumFlingVelocity = MINIMUM_FLING_VELOCITY;
    mMaximumFlingVelocity = MAXIMUM_FLING_VELOCITY;
    mMinimumRotaryEncoderFlingVelocity = MINIMUM_FLING_VELOCITY;
    mMaximumRotaryEncoderFlingVelocity = MAXIMUM_FLING_VELOCITY;
    mScrollbarSize = SCROLL_BAR_SIZE;
    mTouchSlop = TOUCH_SLOP;
    mHoverSlop = TOUCH_SLOP / 2;
    mMinScalingSpan = 0;
    mMinScrollbarTouchTarget = MIN_SCROLLBAR_TOUCH_TARGET;
    mDoubleTapTouchSlop = DOUBLE_TAP_TOUCH_SLOP;
    mPagingTouchSlop = PAGING_TOUCH_SLOP;
    mDoubleTapSlop = DOUBLE_TAP_SLOP;
    mWindowTouchSlop = WINDOW_TOUCH_SLOP;
    mAmbiguousGestureMultiplier = AMBIGUOUS_GESTURE_MULTIPLIER;
    //noinspection deprecation
    mMaximumDrawingCacheSize = MAXIMUM_DRAWING_CACHE_SIZE;
    mOverscrollDistance = OVERSCROLL_DISTANCE;
    mOverflingDistance  = OVERFLING_DISTANCE;
    mFadingMarqueeEnabled = true;
    mPreferKeepClearForFocusEnabled = false;
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

    atts = context->obtainStyledAttributes(context->getPackageName()+":style/view_Configuration");
    if(atts.getAttributeCount()==0)
        atts = context->obtainStyledAttributes("cdroid:style/view_Configuration");

    mEdgeSlop = (int) (sizeAndDensity * EDGE_SLOP + 0.5f);
    mFadingEdgeLength = int(sizeAndDensity*FADING_EDGE_LENGTH + 0.5f);
	
    mDoubleTapSlop = (int) (sizeAndDensity * DOUBLE_TAP_SLOP + 0.5f);
    mWindowTouchSlop = (int) (sizeAndDensity * WINDOW_TOUCH_SLOP + 0.5f);
    mMaximumDrawingCacheSize = 4 * metrics.widthPixels * metrics.heightPixels;
    mOverscrollDistance = (int) (sizeAndDensity * OVERSCROLL_DISTANCE + 0.5f);
    mOverflingDistance = (int) (sizeAndDensity * OVERFLING_DISTANCE + 0.5f);
    mAmbiguousGestureMultiplier = atts.getFloat("config_ambiguousGestureMultiplier",AMBIGUOUS_GESTURE_MULTIPLIER);

    if(atts.getAttributeCount()){
        mIsScreenRound = atts.getBoolean("config_isScreenRound",false);
        mScrollbarSize = atts.getDimensionPixelSize("config_scrollbarSize",mScrollbarSize);
        mFadingMarqueeEnabled = atts.getBoolean("config_ui_enableFadingMarquee",mFadingMarqueeEnabled);
        mTouchSlop = atts.getDimensionPixelSize("config_viewConfigurationTouchSlop",mTouchSlop);
        mHoverSlop = atts.getDimensionPixelSize("config_viewConfigurationHoverSlop",mHoverSlop);
        mMinScalingSpan=atts.getDimensionPixelSize("config_minScalingSpan",mMinScalingSpan);
        mMinScrollbarTouchTarget = atts.getDimensionPixelSize("config_minScrollbarTouchTarget",mMinScrollbarTouchTarget);
        mGlobalActionsKeyTimeout = atts.getInt("config_globalActionsKeyTimeout",mGlobalActionsKeyTimeout);
    }

    mPagingTouchSlop = mTouchSlop * 2;
    mDoubleTapTouchSlop = mTouchSlop;
    if(atts.getAttributeCount()){
        mMinimumFlingVelocity = atts.getDimensionPixelSize("config_viewMinFlingVelocity",mMinimumFlingVelocity);
        mMaximumFlingVelocity = atts.getDimensionPixelSize("config_viewMaxFlingVelocity",mMaximumFlingVelocity);
        mHorizontalScrollFactor = atts.getDimensionPixelSize("config_horizontalScrollFactor",mHorizontalScrollFactor);
        mVerticalScrollFactor = atts.getDimensionPixelSize("config_verticalScrollFactor",mVerticalScrollFactor);
    }
}

ViewConfiguration& ViewConfiguration::get(Context*context){
    if(mInst == nullptr)
        mInst = new ViewConfiguration(context);
    return *mInst;
}

bool ViewConfiguration::isScreenRound(){
    return mInst && mInst->mIsScreenRound;
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

int ViewConfiguration::getScaledScrollBarSize() const{
    return mScrollbarSize;
}

/**
 * @return the minimum size of the scrollbar thumb's touch target in pixels
 * @hide
 */
int ViewConfiguration::getScaledMinScrollbarTouchTarget() const{
    return mMinScrollbarTouchTarget;
}

/**
 * @return the length of the fading edges in pixels
 */
int ViewConfiguration::getScaledFadingEdgeLength() const{
    return mFadingEdgeLength;
}

/**
 * @return Inset in pixels to look for touchable content when the user touches the edge of the
 *         screen
 */
int ViewConfiguration::getScaledEdgeSlop() const{
    return mEdgeSlop;
}


/**
 * @return Distance in pixels a touch can wander before we think the user is scrolling
 */
int ViewConfiguration::getScaledTouchSlop() const{
    return mTouchSlop;
}

/**
 * @return Distance in pixels a hover can wander while it is still considered "stationary".
 *
 */
int ViewConfiguration::getScaledHoverSlop() const{
    return mHoverSlop;
}

/**
 * @return Distance in pixels the first touch can wander before we do not consider this a
 * potential double tap event
 * @hide
 */
int ViewConfiguration::getScaledDoubleTapTouchSlop() const{
    return mDoubleTapTouchSlop;
}

/**
 * @return Distance in pixels a touch can wander before we think the user is scrolling a full
 * page
 */
int ViewConfiguration::getScaledPagingTouchSlop() const{
    return mPagingTouchSlop;
}


/**
 * @return Distance in pixels between the first touch and second touch to still be
 *         considered a double tap
 */
int ViewConfiguration::getScaledDoubleTapSlop() const{
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
long ViewConfiguration::getSendRecurringAccessibilityEventsInterval(){
    return SEND_RECURRING_ACCESSIBILITY_EVENTS_INTERVAL_MILLIS;
}


/**
 * @return Distance in pixels a touch must be outside the bounds of a window for it
 * to be counted as outside the window for purposes of dismissing that window.
 */
int ViewConfiguration::getScaledWindowTouchSlop() const{
    return mWindowTouchSlop;
}


/**
 * @return Minimum velocity to initiate a fling, as measured in pixels per second.
 */
int ViewConfiguration::getScaledMinimumFlingVelocity() const{
    return mMinimumFlingVelocity;
}


/**
 * @return Maximum velocity to initiate a fling, as measured in pixels per second.
 */
int ViewConfiguration::getScaledMaximumFlingVelocity() const{
    return mMaximumFlingVelocity;
}

int ViewConfiguration::getScaledMinimumFlingVelocity(int inputDeviceId, int axis, int source) {
    if (!isInputDeviceInfoValid(inputDeviceId, axis, source)) return NO_FLING_MIN_VELOCITY;
    if (source == InputDevice::SOURCE_ROTARY_ENCODER) return mMinimumRotaryEncoderFlingVelocity;
    return mMinimumFlingVelocity;
}

int ViewConfiguration::getScaledMaximumFlingVelocity(int inputDeviceId, int axis, int source){
    if (!isInputDeviceInfoValid(inputDeviceId, axis, source)) return NO_FLING_MAX_VELOCITY;
    if (source == InputDevice::SOURCE_ROTARY_ENCODER) return mMaximumRotaryEncoderFlingVelocity;
    return mMaximumFlingVelocity;
}

bool ViewConfiguration::isHapticScrollFeedbackEnabled(int inputDeviceId, int axis, int source) {
    if (!isInputDeviceInfoValid(inputDeviceId, axis, source)) return false;

    if (source == InputDevice::SOURCE_ROTARY_ENCODER && axis == MotionEvent::AXIS_SCROLL) {
        return mRotaryEncoderHapticScrollFeedbackEnabled;
    }

    if ((source & InputDevice::SOURCE_TOUCHSCREEN) != 0) {
        return mViewTouchScreenHapticScrollFeedbackEnabled;
    }

    return false;
}

int ViewConfiguration::getHapticScrollFeedbackTickInterval(int inputDeviceId, int axis, int source) {
    if (!mRotaryEncoderHapticScrollFeedbackEnabled) {
        return NO_HAPTIC_SCROLL_TICK_INTERVAL;
    }

    if (!isInputDeviceInfoValid(inputDeviceId, axis, source)) {
        return NO_HAPTIC_SCROLL_TICK_INTERVAL;
    }

    if (source == InputDevice::SOURCE_ROTARY_ENCODER && axis == MotionEvent::AXIS_SCROLL) {
        return mRotaryEncoderHapticScrollFeedbackTickIntervalPixels;
    }

    return NO_HAPTIC_SCROLL_TICK_INTERVAL;
}

bool ViewConfiguration::isViewBasedRotaryEncoderHapticScrollFeedbackEnabled() {
    return mViewBasedRotaryEncoderScrollHapticsEnabledConfig;//&& Flags.useViewBasedRotaryEncoderScrollHaptics();
}

bool ViewConfiguration::isInputDeviceInfoValid(int id, int axis, int source) {
#if 0
    InputDevice* device = InputManagerGlobal.getInstance().getInputDevice(id);
    return device != nullptr && device->getMotionRange(axis, source) != nullptr;
#else
    return false;
#endif
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

float ViewConfiguration::getScaledAmbiguousGestureMultiplier() const{
    return mAmbiguousGestureMultiplier;
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

int ViewConfiguration::getScaledMinimumScalingSpan()const{
    return mMinScalingSpan;
}

/**
 * @hide
 * @return Whether or not marquee should use fading edges.
 */
bool ViewConfiguration::isFadingMarqueeEnabled()const {
    return mFadingMarqueeEnabled;
}

}//namespace

