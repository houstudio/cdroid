#ifndef __SWIPE_DISMISS_TRANSITION_HELPER_H__
#define __SWIPE_DISMISS_TRANSITION_HELPER_H__

#include <widgetEx/wear/dismissableframelayout.h>

namespace cdroid{
class SpringAnimation;
class SwipeDismissTransitionHelper {
public:
    static constexpr float SCRIM_BACKGROUND_MAX = 0.5f;
private:
    static constexpr float SCALE_MIN = 0.7f;
    static constexpr float SCALE_MAX = 1.0f;
    static constexpr float DIM_FOREGROUND_PROGRESS_FACTOR = 2.0f;
    static constexpr float DIM_FOREGROUND_MIN = 0.3f;
    static constexpr int VELOCITY_UNIT = 1000;
    // Spring properties
    static constexpr float SPRING_STIFFNESS = 600.f;
    static constexpr float SPRING_DAMPING_RATIO = SpringForce::DAMPING_RATIO_NO_BOUNCY;
    static constexpr float SPRING_MIN_VISIBLE_CHANGE = 0.5f;
    static constexpr int SPRING_ANIMATION_PROGRESS_FINISH_THRESHOLD_PX = 5;
private:
    DismissibleFrameLayout* mLayout;

    SparseArray<ColorFilter*> mDimmingColorFilterCache;
    Drawable* mScrimBackground;
    //Paint mCompositingPaint = new Paint();

    VelocityTracker* mVelocityTracker;
    bool mIsScreenRound;
    bool mStarted;
    int mScreenWidth;
    int mOriginalViewWidth;
    float mTranslationX;
    float mScale;
    float mProgress;
    float mDimming;
    SpringAnimation* mDismissalSpring;
    SpringAnimation* mRecoverySpring;
    // Variable to restore the parent's background which is added below mScrimBackground.
    Drawable* mPrevParentBackground = nullptr;
private:
    static void clipOutline(View* view, bool useRoundShape);

    ColorFilter* createDimmingColorFilter(float level);

    SpringAnimation* createSpringAnimation(float startValue, float finalValue, float startVelocity,
            const DynamicAnimation::OnAnimationUpdateListener& onUpdateListener,
            const DynamicAnimation::OnAnimationEndListener& onEndListener);
    void onDismissalRecoveryAnimationProgressChanged(float translationX);
    void updateView();
    void updateDim();
    void updateScrim();
    void initializeTransition();

    void resetTranslationAndAlpha();
    Drawable* generateScrimBackgroundDrawable(int width, int height);
    ViewGroup* getOriginalParentView();
public:
    SwipeDismissTransitionHelper(Context* context,DismissibleFrameLayout* layout);

    void onSwipeProgressChanged(float deltaX, MotionEvent& ev);

    bool isAnimating() const;
    /**
     * Triggers the recovery animation.
     */
    void animateRecovery(const DismissController::OnDismissListener& dismissListener);

    /**
     * Triggers the dismiss animation.
     */
    void animateDismissal(const DismissController::OnDismissListener& dismissListener);

    VelocityTracker* getVelocityTracker() const;

    void obtainVelocityTracker();

    void resetVelocityTracker();
};
}/*endof namespace*/
#endif/*__SWIPE_DISMISS_TRANSITION_HELPER_H__*/
