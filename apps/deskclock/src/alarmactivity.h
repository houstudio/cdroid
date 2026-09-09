#ifndef __DESKCLOCK_ALARMACTIVITY_H__
#define __DESKCLOCK_ALARMACTIVITY_H__
/*********************************************************************************
 * Port of com.android.deskclock.alarms.AlarmActivity — the fullscreen firing
 * UX: pulsing alarm button, drag toward SNOOZE/DISMISS, bounce hints, and the
 * circular reveal that ends in the "Snoozed"/"Alarm off" panel.
 *
 * Port notes (documented deviations, everything else 1:1):
 *  - No AlarmService on cdroid: the FIRE point in AlarmStateManager starts
 *    AlarmKlaxon and this activity; AOSP's unbindAlarmService() (which stops
 *    the ringing via service teardown) maps to AlarmKlaxon::stop().
 *  - No lockscreen/navigation-bar/window flags on cdroid (no-op).
 *  - PathInterpolator(PULSE/REVEAL) approximated by built-in interpolators;
 *    the pulse's ARGB fill fade animates view alpha instead (no ArgbEvaluator).
 *********************************************************************************/
#include <core/handler.h>
#include <widget/cdwindow.h>
#include <animation/valueanimator.h>

namespace cdroid {
class ImageView;
class TextView;
class ViewGroup;

namespace deskclock {
namespace data { class Alarminstance; }

namespace alarms {

class AlarmActivity : public Window {
private:
    Handler mHandler;

    data::Alarminstance* mAlarmInstance = nullptr;
    bool mAlarmHandled = false;
    int mVolumeBehavior = 0;   // DataModel.AlarmVolumeButtonBehavior ordinal
    int mCurrentHourColor = 0;

    ViewGroup* mAlertView = nullptr;
    TextView* mAlertTitleView = nullptr;
    TextView* mAlertInfoView = nullptr;

    ViewGroup* mContentView = nullptr;
    ImageView* mAlarmButton = nullptr;
    ImageView* mSnoozeButton = nullptr;
    ImageView* mDismissButton = nullptr;
    TextView* mHintView = nullptr;

    ValueAnimator* mAlarmAnimator = nullptr;
    ValueAnimator* mSnoozeAnimator = nullptr;
    ValueAnimator* mDismissAnimator = nullptr;
    ValueAnimator* mPulseAnimator = nullptr;

    int mInitialPointerIndex = -1;   // MotionEvent.INVALID_POINTER_ID

public:
    AlarmActivity();
    ~AlarmActivity() override;

    void onCreate(Bundle* savedInstanceState) override;
    void onResume() override;
    void onPause() override;
    bool dispatchKeyEvent(KeyEvent& event) override;
    void onBackPressed() override;

private:
    void onClick(View& view);
    bool onTouch(View& view, MotionEvent& event);

    void hintSnooze();
    void hintDismiss();
    void resetAnimations();
    void snooze();
    void dismiss();
    void setAnimatedFractions(float snoozeFraction, float dismissFraction);
    static float getFraction(float x0, float x1, float x);
    ValueAnimator* getButtonAnimator(ImageView* button, int tintColor);
    ValueAnimator* getAlarmBounceAnimator(float translationX, int hintResId);
    Animator* getAlertAnimator(View& source, int titleResId,
            const std::string& infoText, const std::string& accessibilityText,
            int revealColor, int backgroundColor);
};

} // namespace alarms
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_ALARMACTIVITY_H__
