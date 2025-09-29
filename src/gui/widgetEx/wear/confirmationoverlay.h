#ifndef __CONFIRMATION_OVERLAY_H__
#define __CONFIRMATION_OVERLAY_H__
#include <view/viewgroup.h>
/**
 * Displays a full-screen confirmation animation with optional text and then hides it.
 *
 * <p>This is a lighter-weight version of {@link androidx.wear.activity.ConfirmationActivity}
 * and should be preferred when constructed from an {@link Activity}.
 *
 * <p>Sample usage:
 *
 * <pre>
 *   // Defaults to SUCCESS_ANIMATION
 *   new ConfirmationOverlay().showOn(myActivity);
 *
 *   new ConfirmationOverlay()
 *      .setType(ConfirmationOverlay.OPEN_ON_PHONE_ANIMATION)
 *      .setDuration(3000)
 *      .setMessage("Opening...")
 *      .setFinishedAnimationListener(new ConfirmationOverlay.OnAnimationFinishedListener() {
 *          {@literal @}Override
 *          public void onAnimationFinished() {
 *              // Finished animating and the content view has been removed from myActivity.
 *          }
 *      }).showOn(myActivity);
 *
 *   // Default duration is {@link #DEFAULT_ANIMATION_DURATION_MS}
 *   new ConfirmationOverlay()
 *      .setType(ConfirmationOverlay.FAILURE_ANIMATION)
 *      .setMessage("Failed")
 *      .setFinishedAnimationListener(new ConfirmationOverlay.OnAnimationFinishedListener() {
 *          {@literal @}Override
 *          public void onAnimationFinished() {
 *              // Finished animating and the view has been removed from myView.getRootView().
 *          }
 *      }).showAbove(myView);
 * </pre>
 */
namespace cdroid{
class ConfirmationOverlay {
public:
    /**
     * Interface for listeners to be notified when the {@link ConfirmationOverlay} animation has
     * finished and its {@link View} has been removed.
     */
    DECLARE_UIEVENT(void,OnAnimationFinishedListener);

    /** Default animation duration in ms. **/
    static constexpr int DEFAULT_ANIMATION_DURATION_MS = 1000;

    /** Types of animations to display in the overlay. */

    /** {@link OverlayType} indicating the success animation overlay should be displayed. */
    static constexpr int SUCCESS_ANIMATION = 0;

    /**
     * {@link OverlayType} indicating the failure overlay should be shown. The icon associated with
     * this type, unlike the others, does not animate.
     */
    static constexpr int FAILURE_ANIMATION = 1;

    /** {@link OverlayType} indicating the "Open on Phone" animation overlay should be displayed. */
    static constexpr int OPEN_ON_PHONE_ANIMATION = 2;
private:
    int mType;
    int mDurationMillis;
    bool mIsShowing;
    OnAnimationFinishedListener mListener;
    std::string mMessage;
    View* mOverlayView;
    Drawable* mOverlayDrawable;
    Handler* mMainThreadHandler;
    Runnable mHideRunnable;
private:
    void animateAndHideAfterDelay();
    void updateOverlayView(Context* context);
    void updateMessageView(Context* context, View* overlayView);
    void updateImageView(Context* context, View* overlayView);
public:
    ConfirmationOverlay();
    ConfirmationOverlay(const std::string&message);
    virtual ~ConfirmationOverlay();
    /**
     * Sets a message which will be displayed at the same time as the animation.
     *
     * @return {@code this} object for method chaining.
     */
    ConfirmationOverlay& setMessage(const std::string&message);

    /**
     * Sets the {@link OverlayType} which controls which animation is displayed.
     *
     * @return {@code this} object for method chaining.
     */
    ConfirmationOverlay& setType(int type);

    /**
     * Sets the duration in milliseconds which controls how long the animation will be displayed.
     * Default duration is {@link #DEFAULT_ANIMATION_DURATION_MS}.
     *
     * @return {@code this} object for method chaining.
     */
    ConfirmationOverlay& setDuration(int millis);

    /**
     * Sets the {@link OnAnimationFinishedListener} which will be invoked once the overlay is no
     * longer visible.
     *
     * @return {@code this} object for method chaining.
     */
    ConfirmationOverlay& setFinishedAnimationListener(const OnAnimationFinishedListener& listener);

    /**
     * Adds the overlay as a child of {@code view.getRootView()}, removing it when complete. While
     * it is shown, all touches will be intercepted to prevent accidental taps on obscured views.
     */
    void showAbove(View* view);

    /**
     * Adds the overlay as a content view to the {@code activity}, removing it when complete. While
     * it is shown, all touches will be intercepted to prevent accidental taps on obscured views.
     */
    void showOn(Window* activity);

    /**
     * Starts a fadeout animation and removes the view once finished. This is invoked by {@link
     * #mHideRunnable} after {@link #mDurationMillis} milliseconds.
     *
     * @hide
     */
    void hide();
};
}/*endof namespace*/
#endif/*__CONFIRMATION_OVERLAY_H__*/
