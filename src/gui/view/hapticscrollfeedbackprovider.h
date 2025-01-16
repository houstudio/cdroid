#ifndef __HAPTICK_SCROLL_FEEDBACK_PROVIDER_H__
#define __HAPTICK_SCROLL_FEEDBACK_PROVIDER_H__
#include <view/scrollfeedbackprovider.h>

namespace cdroid{
class HapticScrollFeedbackProvider:public ScrollFeedbackProvider {
private:
    static constexpr int TICK_INTERVAL_NO_TICK = 0;
    static constexpr bool INITIAL_END_OF_LIST_HAPTICS_ENABLED = false;

    View* mView;
    ViewConfiguration* mViewConfig;
    /**
     * Flag to disable the logic in this class if the View-based scroll haptics implementation is
     * enabled. If {@code false}, this class will continue to run despite the View's scroll
     * haptics implementation being enabled. This value should be set to {@code true} when this
     * class is directly used by the View class.
     */
    bool mDisabledIfViewPlaysScrollHaptics;


    // Info about the cause of the latest scroll event.
    /** The ID of the {link @InputDevice} that caused the latest scroll event. */
    int mDeviceId = -1;
    /** The axis on which the latest scroll event happened. */
    int mAxis = -1;
    /** The {@link InputDevice} source from which the latest scroll event happened. */
    int mSource = -1;

    /** The tick interval corresponding to the current InputDevice/source/axis. */
    int mTickIntervalPixels = TICK_INTERVAL_NO_TICK;
    int mTotalScrollPixels = 0;
    bool mCanPlayLimitFeedback = INITIAL_END_OF_LIST_HAPTICS_ENABLED;
    bool mHapticScrollFeedbackEnabled = false;
private:
    void maybeUpdateCurrentConfig(int deviceId, int source, int axis);
    void updateTickIntervals(int deviceId, int source, int axis);
public:
    HapticScrollFeedbackProvider(View* view);
    HapticScrollFeedbackProvider(View* view, ViewConfiguration* viewConfig, bool disabledIfViewPlaysScrollHaptics);

    void onScrollProgress(int inputDeviceId, int source, int axis, int deltaInPixels)override;
    void onScrollLimit(int inputDeviceId, int source, int axis, bool isStart)override;
    void onSnapToItem(int inputDeviceId, int source, int axis)override;
};
}/*endof namespace*/
#endif/*__HAPTICK_SCROLL_FEEDBACK_PROVIDER_H__*/
