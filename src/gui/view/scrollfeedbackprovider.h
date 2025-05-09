#ifndef __SCROLL_FEEDBACK_PROVIDER_H__
#define __SCROLL_FEEDBACK_PROVIDER_H__
namespace cdroid{
class View;
class ViewConfiguration;
class ScrollFeedbackProvider {
public:
    virtual ~ScrollFeedbackProvider()=default;
    /**
     * Creates a {@link ScrollFeedbackProvider} implementation for this device.
     *
     * <p>Use a feedback provider created by this method, unless you intend to use your custom
     * scroll feedback providing logic. This allows your use cases to generate scroll feedback that
     * is consistent with the rest of the use cases on the device.
     *
     * @param view the {@link View} for which to provide scroll feedback.
     * @return the default {@link ScrollFeedbackProvider} implementation for the device.
     */
    static ScrollFeedbackProvider* createProvider(View* view);

    /**
     * Call this when the view has snapped to an item.
     *
     * @param inputDeviceId the ID of the {@link InputDevice} that generated the motion triggering
     *          the snap.
     * @param source the input source of the motion causing the snap.
     * @param axis the axis of {@code event} that caused the item to snap.
     */
    virtual void onSnapToItem(int inputDeviceId, int source, int axis)=0;

    /**
     * Call this when the view has reached the scroll limit.
     *
     * <p>Note that a feedback may not be provided on every call to this method. This interface, for
     * instance, may provide feedback on every `N`th scroll limit event. For the interface to
     * properly provide feedback when needed, call this method for each scroll limit event that you
     * want to be accounted to scroll limit feedback.
     *
     * @param inputDeviceId the ID of the {@link InputDevice} that caused scrolling to hit limit.
     * @param source the input source of the motion that caused scrolling to hit the limit.
     * @param axis the axis of {@code event} that caused scrolling to hit the limit.
     * @param isStart {@code true} if scrolling hit limit at the start of the scrolling list, and
     *                {@code false} if the scrolling hit limit at the end of the scrolling list.
     *                <i>start</i> and <i>end<i> in this context are not geometrical references.
     *                Instead, they refer to the start and end of a scrolling experience. As such,
     *                "start" for some views may be at the bottom of a scrolling list, while it may
     *                be at the top of scrolling list for others.
     */
    virtual void onScrollLimit(int inputDeviceId, int source, int axis, bool isStart)=0;

    /**
     * Call this when the view has scrolled.
     *
     * <p>Different axes have different ways to map their raw axis values to pixels for scrolling.
     * When calling this method, use the scroll values in pixels by which the view was scrolled; do
     * not use the raw axis values. That is, use whatever value is passed to one of View's scrolling
     * methods (example: {@link View#scrollBy(int, int)}). For example, for vertical scrolling on
     * {@link MotionEvent#AXIS_SCROLL}, convert the raw axis value to the equivalent pixels by using
     * {@link ViewConfiguration#getScaledVerticalScrollFactor()}, and use that value for this method
     * call.
     *
     * <p>Note that a feedback may not be provided on every call to this method. This interface, for
     * instance, may provide feedback for every `x` pixels scrolled. For the interface to properly
     * track scroll progress and provide feedback when needed, call this method for each scroll
     * event that you want to be accounted to scroll feedback.
     *
     * @param inputDeviceId the ID of the {@link InputDevice} that caused scroll progress.
     * @param source the input source of the motion that caused scroll progress.
     * @param axis the axis of {@code event} that caused scroll progress.
     * @param deltaInPixels the amount of scroll progress, in pixels.
     */
    virtual void onScrollProgress(int inputDeviceId, int source, int axis, int deltaInPixels)=0;
};

}/*endof namespacec*/
#endif/*__SCROLL_FEEDBACK_PROVIDER_H__*/
