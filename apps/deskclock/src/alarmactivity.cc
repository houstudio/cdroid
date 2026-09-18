#include <alarmactivity.h>
#include <R.h>
#include <porting/cdlog.h>
#include <core/activityfactory.h>
#include <core/intent.h>
#include <core/systemclock.h>
#include <view/keyevent.h>
#include <view/layoutinflater.h>
#include <view/motionevent.h>
#include <widget/framelayout.h>
#include <widget/imageview.h>
#include <widget/linearlayout.h>
#include <widget/textview.h>
#include <animation/objectanimator.h>
#include <animation/animatorset.h>


#include <alarmklaxon.h>
#include <alarmstatemanager.h>
#include <alarminstance.h>
#include <animatorutils.h>
#include <circleview.h>
#include <datamodel.h>
#include <utils.h>

using namespace ::deskclock;
namespace cdroid {
namespace deskclock {
namespace alarms {

using data::Alarminstance;
using data::DataModel;

namespace {

const char* EXTRA_INSTANCE_ID = "alarmInstanceId";   // AOSP: intent data uri id

// PathInterpolator(0.4f, 0.0f, 0.2f, 1.0f) / (0.0f, 0.0f, 0.2f, 1.0f) upstream;
// cdroid has no path interpolator — these are the closest built-ins.
const Interpolator* pulseInterpolator() { static DecelerateInterpolator i(1.5f); return &i; }
const Interpolator* revealInterpolator() { static AccelerateDecelerateInterpolator i; return &i; }

constexpr long PULSE_DURATION_MILLIS = 1000;
constexpr long ALARM_BOUNCE_DURATION_MILLIS = 500;
constexpr long ALERT_REVEAL_DURATION_MILLIS = 500;
constexpr long ALERT_FADE_DURATION_MILLIS = 500;
constexpr long ALERT_DISMISS_DELAY_MILLIS = 2000;

constexpr float BUTTON_SCALE_DEFAULT = 0.7f;
constexpr int BUTTON_DRAWABLE_ALPHA_DEFAULT = 165;

} // namespace

AlarmActivity::AlarmActivity()
    : Window(0, 0, -1, -1)
    , mHandler(Looper::getMainLooper()) {
}

AlarmActivity::~AlarmActivity() {
    delete mAlarmInstance;
    delete mAlarmAnimator;
    delete mSnoozeAnimator;
    delete mDismissAnimator;
    delete mPulseAnimator;
}

void AlarmActivity::onCreate(Bundle* /*savedInstanceState*/) {
    Window::onCreate(nullptr);

    const int64_t instanceId = getIntent().getLongExtra(EXTRA_INSTANCE_ID, -1);
    mAlarmInstance = new Alarminstance();
    if (!Alarminstance::getInstance(*getContext()->getSharedPreferences("DeskClock", Context::MODE_PRIVATE), instanceId,
                *mAlarmInstance)) {
        // The alarm was deleted before the activity got created, so just finish().
        LOGE("Error displaying alarm for instance %lld", (long long) instanceId);
        close();
        return;
    } else if (mAlarmInstance->mAlarmState != Alarminstance::FIRED_STATE) {
        LOGI("Skip displaying alarm for instance: %lld", (long long) instanceId);
        close();
        return;
    }
    LOGI("Displaying alarm for instance: %lld", (long long) instanceId);

    // Get the volume/camera button behavior setting (0=NOTHING 1=SNOOZE 2=DISMISS).
    mVolumeBehavior = (int) DataModel::getDataModel().getAlarmVolumeButtonBehavior();

    LayoutInflater::from(getContext())->inflate(R::layout::alarm_activity, this, true);

    mAlertView = (ViewGroup*) findViewById(R::id::alert);
    mAlertTitleView = (TextView*) findViewById(R::id::alert_title);
    mAlertInfoView = (TextView*) findViewById(R::id::alert_info);

    mContentView = (ViewGroup*) findViewById(R::id::content);
    mAlarmButton = (ImageView*) findViewById(R::id::alarm);
    mSnoozeButton = (ImageView*) findViewById(R::id::snooze);
    mDismissButton = (ImageView*) findViewById(R::id::dismiss);
    mHintView = (TextView*) findViewById(R::id::hint);

    auto* titleView = (TextView*) findViewById(R::id::title);
    auto* digitalClock = (TextClock*) findViewById(R::id::digital_clock);
    auto* pulseView = (CircleView*) findViewById(R::id::pulse);

    titleView->setText(mAlarmInstance->mLabel.empty()
            ? getContext()->getString(R::string::alarm) : mAlarmInstance->mLabel);
    Utils::setTimeFormat(digitalClock, false);

    // AOSP: ThemeUtils.resolveColor(this, android.R.attr.windowBackground) —
    // the framework attr id resolves through the theme (utils.cc idiom).
    TypedValue hourValue;
    mCurrentHourColor = (getContext()->getTheme().resolveAttribute(
            0x01010054 /* android:windowBackground */, &hourValue, true)
            && hourValue.type >= TypedValue::TYPE_FIRST_COLOR_INT)
            ? (int) hourValue.data : 0xff303030;
    setBackground(new ColorDrawable(mCurrentHourColor));   // owned (View dtor frees)

    mAlarmButton->setOnTouchListener([this](View& v, MotionEvent& e) { return onTouch(v, e); });
    mSnoozeButton->setOnClickListener([this](View& v) { onClick(v); });
    mDismissButton->setOnClickListener([this](View& v) { onClick(v); });

    mAlarmAnimator = AnimatorUtils::getScaleAnimator(mAlarmButton, {1.0f, 0.0f});
    mSnoozeAnimator = getButtonAnimator(mSnoozeButton, Color::WHITE);
    mDismissAnimator = getButtonAnimator(mDismissButton, mCurrentHourColor);

    // AOSP animates CircleView.RADIUS (the Property object) + view alpha.
    // The string-name form crashes: Property::fromName("radius") finds no
    // global registration and setupSetterAndGetter dereferences the null.
    const float pulseRadius = pulseView->getRadius();
    const std::vector<PropertyValuesHolder*> pulseProps = {
        PropertyValuesHolder::ofFloat(CircleView::RADIUS(), {0.0f, pulseRadius}),
        PropertyValuesHolder::ofFloat(View::ALPHA, {1.0f, 0.0f}),
    };
    mPulseAnimator = ObjectAnimator::ofPropertyValuesHolder(pulseView, pulseProps);
    mPulseAnimator->setDuration(PULSE_DURATION_MILLIS);
    mPulseAnimator->setInterpolator(pulseInterpolator());
    mPulseAnimator->setRepeatCount(ValueAnimator::INFINITE);
    mPulseAnimator->start();
}

void AlarmActivity::onResume() {
    Window::onResume();

    // Re-query for AlarmInstance in case the state has changed externally.
    const int64_t instanceId = getIntent().getLongExtra(EXTRA_INSTANCE_ID, -1);
    Alarminstance fresh;
    if (!Alarminstance::getInstance(*getContext()->getSharedPreferences("DeskClock", Context::MODE_PRIVATE), instanceId, fresh)) {
        LOGI("No alarm instance for instanceId: %lld", (long long) instanceId);
        close();
        return;
    }
    *mAlarmInstance = fresh;

    // Verify that the alarm is still firing before showing the activity.
    if (mAlarmInstance->mAlarmState != Alarminstance::FIRED_STATE) {
        LOGI("Skip displaying alarm for instance: %lld", (long long) instanceId);
        close();
        return;
    }
    resetAnimations();
}

void AlarmActivity::onPause() {
    Window::onPause();
}

bool AlarmActivity::dispatchKeyEvent(KeyEvent& keyEvent) {
    const int keyCode = keyEvent.getKeyCode();
    switch (keyCode) {
        case KeyEvent::KEYCODE_VOLUME_UP:
        case KeyEvent::KEYCODE_VOLUME_DOWN:
        case KeyEvent::KEYCODE_VOLUME_MUTE:
        case KeyEvent::KEYCODE_HEADSETHOOK:
        case KeyEvent::KEYCODE_CAMERA:
        case KeyEvent::KEYCODE_FOCUS:
            if (!mAlarmHandled) {
                switch (mVolumeBehavior) {
                    case 1:   // SNOOZE
                        if (keyEvent.getAction() == KeyEvent::ACTION_UP) snooze();
                        return true;
                    case 2:   // DISMISS
                        if (keyEvent.getAction() == KeyEvent::ACTION_UP) dismiss();
                        return true;
                    default:  // NOTHING
                        break;
                }
            }
            break;
        default:
            break;
    }
    return Window::dispatchKeyEvent(keyEvent);
}

void AlarmActivity::onBackPressed() {
    // (empty on purpose)
}

void AlarmActivity::onClick(View& view) {
    if (mAlarmHandled) return;

    // If in accessibility mode, allow snooze/dismiss by tapping the icons.
    if (AccessibilityManager::getInstance(getContext()).isEnabled()) {
        if (&view == mSnoozeButton) {
            snooze();
        } else if (&view == mDismissButton) {
            dismiss();
        }
        return;
    }

    if (&view == mSnoozeButton) {
        hintSnooze();
    } else if (&view == mDismissButton) {
        hintDismiss();
    }
}

bool AlarmActivity::onTouch(View& /*view*/, MotionEvent& event) {
    if (mAlarmHandled) return false;

    const int action = event.getActionMasked();
    if (action == MotionEvent::ACTION_DOWN) {
        // Track the pointer that initiated the touch sequence.
        mInitialPointerIndex = event.getPointerId(event.getActionIndex());
        // Stop the pulse, allowing the last pulse to finish.
        mPulseAnimator->setRepeatCount(0);
    } else if (action == MotionEvent::ACTION_CANCEL) {
        mInitialPointerIndex = -1;
        resetAnimations();
    }

    const int actionIndex = event.getActionIndex();
    if (mInitialPointerIndex == -1
            || mInitialPointerIndex != event.getPointerId(actionIndex)) {
        return true;   // ignore any pointers other than the initial one
    }

    const int contentLocation[2] = {0, 0};
    mContentView->getLocationOnScreen(const_cast<int*>(contentLocation));
    const float x = event.getRawX() - contentLocation[0];
    const float y = event.getRawY() - contentLocation[1];

    const int alarmLeft = mAlarmButton->getLeft() + mAlarmButton->getPaddingLeft();
    const int alarmRight = mAlarmButton->getRight() - mAlarmButton->getPaddingRight();

    float snoozeFraction, dismissFraction;
    if (mContentView->getLayoutDirection() == View::LAYOUT_DIRECTION_RTL) {
        snoozeFraction = getFraction((float) alarmRight, (float) mSnoozeButton->getLeft(), x);
        dismissFraction = getFraction((float) alarmLeft, (float) mDismissButton->getRight(), x);
    } else {
        snoozeFraction = getFraction((float) alarmLeft, (float) mSnoozeButton->getRight(), x);
        dismissFraction = getFraction((float) alarmRight, (float) mDismissButton->getLeft(), x);
    }
    setAnimatedFractions(snoozeFraction, dismissFraction);

    if (action == MotionEvent::ACTION_UP || action == MotionEvent::ACTION_POINTER_UP) {
        mInitialPointerIndex = -1;
        if (snoozeFraction == 1.0f) {
            snooze();
        } else if (dismissFraction == 1.0f) {
            dismiss();
        } else {
            if (snoozeFraction > 0.0f || dismissFraction > 0.0f) {
                // Animate back to the initial state.
                AnimatorUtils::reverse({mAlarmAnimator, mSnoozeAnimator, mDismissAnimator});
            } else if (mAlarmButton->getTop() <= y && y <= mAlarmButton->getBottom()) {
                // User touched the alarm button, hint the dismiss action.
                hintDismiss();
            }
            // Restart the pulse.
            mPulseAnimator->setRepeatCount(ValueAnimator::INFINITE);
            if (!mPulseAnimator->isStarted()) mPulseAnimator->start();
        }
    }
    return true;
}

void AlarmActivity::hintSnooze() {
    const int alarmLeft = mAlarmButton->getLeft() + mAlarmButton->getPaddingLeft();
    const int alarmRight = mAlarmButton->getRight() - mAlarmButton->getPaddingRight();
    const float translationX = (float) (std::max(mSnoozeButton->getLeft() - alarmRight, 0)
            + std::min(mSnoozeButton->getRight() - alarmLeft, 0));
    getAlarmBounceAnimator(translationX, translationX < 0.0f
            ? (int) R::string::description_direction_left
            : (int) R::string::description_direction_right)->start();
}

void AlarmActivity::hintDismiss() {
    const int alarmLeft = mAlarmButton->getLeft() + mAlarmButton->getPaddingLeft();
    const int alarmRight = mAlarmButton->getRight() - mAlarmButton->getPaddingRight();
    const float translationX = (float) (std::max(mDismissButton->getLeft() - alarmRight, 0)
            + std::min(mDismissButton->getRight() - alarmLeft, 0));
    getAlarmBounceAnimator(translationX, translationX < 0.0f
            ? (int) R::string::description_direction_left
            : (int) R::string::description_direction_right)->start();
}

void AlarmActivity::resetAnimations() {
    setAnimatedFractions(0.0f, 0.0f);
    mPulseAnimator->setRepeatCount(ValueAnimator::INFINITE);
    if (!mPulseAnimator->isStarted()) mPulseAnimator->start();
}

void AlarmActivity::snooze() {
    mAlarmHandled = true;
    LOGV("Snoozed: %lld", (long long) mAlarmInstance->mId);

    TypedValue accent;
    const int colorAccent = (getContext()->getTheme().resolveAttribute(
            0x01010435 /* android:colorAccent */, &accent, true)
            && accent.type >= TypedValue::TYPE_FIRST_COLOR_INT) ? (int) accent.data : 0xff2979ff;
    setAnimatedFractions(1.0f, 0.0f);

    const int snoozeMinutes = DataModel::getDataModel().getSnoozeLength();
    const std::string infoText = getContext()->getResources().getQuantityString(
            R::plurals::alarm_alert_snooze_duration, snoozeMinutes,
            {std::to_string(snoozeMinutes)});
    const std::string accessibilityText = getContext()->getResources().getQuantityString(
            R::plurals::alarm_alert_snooze_set, snoozeMinutes,
            {std::to_string(snoozeMinutes)});

    getAlertAnimator(*mSnoozeButton, R::string::alarm_alert_snoozed_text, infoText,
            accessibilityText, colorAccent, colorAccent)->start();

    AlarmStateManager::setSnoozeState(*getContext(), *mAlarmInstance);

    // Unbind here, otherwise alarm will keep ringing until activity finishes.
    AlarmKlaxon::stop(*getContext());
}

void AlarmActivity::dismiss() {
    mAlarmHandled = true;
    LOGV("Dismissed: %lld", (long long) mAlarmInstance->mId);

    setAnimatedFractions(0.0f, 1.0f);

    const std::string offText = getContext()->getString(R::string::alarm_alert_off_text);
    getAlertAnimator(*mDismissButton, R::string::alarm_alert_off_text, std::string(), offText,
            Color::WHITE, mCurrentHourColor)->start();

    AlarmStateManager::deleteInstanceAndUpdateParent(*getContext(), *mAlarmInstance);

    AlarmKlaxon::stop(*getContext());
}

void AlarmActivity::setAnimatedFractions(float snoozeFraction, float dismissFraction) {
    const float alarmFraction = std::max(snoozeFraction, dismissFraction);
    AnimatorUtils::setAnimatedFraction(*mAlarmAnimator, alarmFraction);
    AnimatorUtils::setAnimatedFraction(*mSnoozeAnimator, snoozeFraction);
    AnimatorUtils::setAnimatedFraction(*mDismissAnimator, dismissFraction);
}

float AlarmActivity::getFraction(float x0, float x1, float x) {
    return std::max(std::min((x - x0) / (x1 - x0), 1.0f), 0.0f);
}

ValueAnimator* AlarmActivity::getButtonAnimator(ImageView* button, int tintColor) {
    const std::vector<PropertyValuesHolder*> props = {
        PropertyValuesHolder::ofFloat(View::SCALE_X, {BUTTON_SCALE_DEFAULT, 1.0f}),
        PropertyValuesHolder::ofFloat(View::SCALE_Y, {BUTTON_SCALE_DEFAULT, 1.0f}),
        PropertyValuesHolder::ofInt(AnimatorUtils::BACKGROUND_ALPHA(), {0, 255}),
        PropertyValuesHolder::ofInt(AnimatorUtils::DRAWABLE_ALPHA(),
                {BUTTON_DRAWABLE_ALPHA_DEFAULT, 255}),
    };
    return ObjectAnimator::ofPropertyValuesHolder(button, props);
}

ValueAnimator* AlarmActivity::getAlarmBounceAnimator(float translationX, int hintResId) {
    auto* bounceAnimator = ObjectAnimator::ofFloat(mAlarmButton, View::TRANSLATION_X,
            {mAlarmButton->getTranslationX(), translationX, 0.0f});
    bounceAnimator->setInterpolator(AnimatorUtils::DECELERATE_ACCELERATE_INTERPOLATOR());
    bounceAnimator->setDuration(ALARM_BOUNCE_DURATION_MILLIS);
    Animator::AnimatorListener bounceListener;
    bounceListener.onAnimationStart = [this, hintResId](Animator& a, bool isReverse) {
        mHintView->setText(hintResId);
        if (mHintView->getVisibility() != View::VISIBLE) {
            mHintView->setVisibility(View::VISIBLE);
            ObjectAnimator::ofFloat(mHintView, View::ALPHA, {0.0f, 1.0f})->start();
        }
    };
    bounceAnimator->addListener(bounceListener);
    return bounceAnimator;
}

Animator* AlarmActivity::getAlertAnimator(View& source, int titleResId,
        const std::string& infoText, const std::string& accessibilityText,
        int revealColor, int backgroundColor) {
    auto* containerView = (ViewGroup*) findViewById(R::id::content)->getParent();

    Rect sourceBounds;   // cdroid Rect is l/t/w/h; AOSP passes (0,0,h,w) as l/t/r/b
    sourceBounds.set(0, 0, source.getHeight(), source.getWidth());
    containerView->offsetDescendantRectToMyCoords(&source, sourceBounds);

    const int centerX = sourceBounds.centerX();
    const int centerY = sourceBounds.centerY();
    const int xMax = std::max(centerX, containerView->getWidth() - centerX);
    const int yMax = std::max(centerY, containerView->getHeight() - centerY);
    const float startRadius = std::max(sourceBounds.width, sourceBounds.height) / 2.0f;
    const float endRadius = std::sqrt((float) (xMax * xMax + yMax * yMax));

    auto* revealView = new CircleView(getContext(), nullptr);
    revealView->setCenterX((float) centerX);
    revealView->setCenterY((float) centerY);
    revealView->setFillColor(revealColor);
    containerView->addView(revealView);

    const bool hasInfo = !infoText.empty();
    // CircleView.RADIUS Property object (AOSP); the string name has no global
    // registration (see the pulse animator note in onCreate).
    auto* revealAnimator = ObjectAnimator::ofFloat(revealView, CircleView::RADIUS(),
            {startRadius, endRadius});
    revealAnimator->setDuration(ALERT_REVEAL_DURATION_MILLIS);
    revealAnimator->setInterpolator(revealInterpolator());
    Animator::AnimatorListener revealListener;
    revealListener.onAnimationEnd = [this, titleResId, hasInfo, infoText, backgroundColor]
            (Animator& a, bool isReverse) {
        mAlertView->setVisibility(View::VISIBLE);
        mAlertTitleView->setText(titleResId);
        if (hasInfo) {
            mAlertInfoView->setText(infoText);
            mAlertInfoView->setVisibility(View::VISIBLE);
        }
        mContentView->setVisibility(View::GONE);
        setBackground(new ColorDrawable(backgroundColor));
    };
    revealAnimator->addListener(revealListener);

    auto* fadeAnimator = ObjectAnimator::ofFloat(revealView, View::ALPHA, {0.0f});
    fadeAnimator->setDuration(ALERT_FADE_DURATION_MILLIS);
    Animator::AnimatorListener fadeListener;
    fadeListener.onAnimationEnd = [containerView, revealView](Animator& a, bool isReverse) {
        containerView->removeView(revealView);
    };
    fadeAnimator->addListener(fadeListener);

    auto* alertAnimator = new AnimatorSet();
    alertAnimator->play(revealAnimator)->before(fadeAnimator);
    Animator::AnimatorListener endListener;
    endListener.onAnimationEnd = [this, accessibilityText](Animator& a, bool isReverse) {
        mAlertView->announceForAccessibility(accessibilityText);
        mHandler.postDelayed([this]() { close(); }, ALERT_DISMISS_DELAY_MILLIS);
    };
    alertAnimator->addListener(endListener);
    return alertAnimator;
}

// Registered under the same class name the FIRE wiring in AlarmStateManager
// starts (hand-written key: REGISTER_ACTIVITY stringizes a qualified name).
static const bool sActivityRegistered = []() {
    cdroid::ActivityFactory::registerActivity(
            "AlarmActivity", []() -> cdroid::Window* { return new AlarmActivity(); });
    return true;
}();

} // namespace alarms
} // namespace deskclock
} // namespace cdroid
