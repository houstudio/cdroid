#ifndef __OVER_SCROLLER_H__
#define __OVER_SCROLLER_H__
#include <core/context.h>
#include <animation/interpolators.h>

namespace cdroid{

class OverScroller{
private:
class SplineOverScroller{
private://constexprs  
    // Constant gravity value, used in the deceleration phase.
    static constexpr float GRAVITY = 2000.0f;
    static constexpr float DECELERATION_RATE = (float) (std::log(0.78) / std::log(0.9));
    static constexpr float INFLEXION = 0.35f; // Tension lines cross at (INFLEXION, 1)
    static constexpr float START_TENSION = 0.5f;
    static constexpr float END_TENSION = 1.0f;
    static constexpr float P1 = START_TENSION * INFLEXION;
    static constexpr float P2 = 1.0f - END_TENSION * (1.0f - INFLEXION);

    static constexpr int NB_SAMPLES = 100;
    static float SPLINE_POSITION[NB_SAMPLES + 1];
    static float SPLINE_TIME[NB_SAMPLES+1];
public:
    static constexpr int SPLINE = 0;
    static constexpr int CUBIC = 1;
    static constexpr int BALLISTIC = 2;
public:
    int mStart;
    int mCurrentPosition;
    int mFinal;// Final position
    // Initial velocity
    int mVelocity;
    float mCurrVelocity;
    // Constant current deceleration
    float mDeceleration;
    // Animation starting time, in system milliseconds
    long mStartTime;
    // Animation duration, in milliseconds
    int mDuration;
    // Duration to complete spline component of animation
    int mSplineDuration;
    // Distance to travel along spline animation
    int mSplineDistance;
    // Whether the animation is currently in progress
     bool mFinished;
    // The allowed overshot distance before boundary is reached.
    int mOver;
    // Fling friction
    float mFlingFriction;
    // Current state of the animation.
    int mState = SPLINE;
    // A context-specific coefficient adjusted to physical values.
    float mPhysicalCoeff;
private:
    static void sInit();
    float getDeceleration(int velocity);
    void adjustDuration(int start, int oldFinal, int newFinal);
    void startSpringback(int start, int end, int velocity);
    double getSplineDeceleration(int velocity);
    double getSplineFlingDistance(int velocity);
    int getSplineFlingDuration(int velocity);
    void fitOnBounceCurve(int start, int end, int velocity);
    void startBounceAfterEdge(int start, int end, int velocity);
    void startAfterEdge(int start, int min, int max, int velocity);
    void onEdgeReached();
public:
    SplineOverScroller(Context* context);
    void setFriction(float friction);
    void updateScroll(float q);
    void startScroll(int start, int distance, int duration);
    void finish();
    void setFinalPosition(int position);
    void extendDuration(int extend);
    bool springback(int start, int min, int max);
    void fling(int start, int velocity, int min, int max, int over);
    void notifyEdgeReached(int start, int end, int over);
    bool continueWhenFinished();
    bool update();
};
private:
    static constexpr int DEFAULT_DURATION = 250;
    static constexpr int SCROLL_MODE = 0;
    static constexpr int FLING_MODE = 1;
    int mMode;
    SplineOverScroller * mScrollerX;
    SplineOverScroller * mScrollerY;

    Interpolator* mInterpolator;
    bool mFlywheel;
public:
    OverScroller(Context* context);
    OverScroller(Context* context, Interpolator* interpolator, bool flywheel);
    ~OverScroller();
    void setInterpolator(Interpolator* interpolator);
    void setFriction(float friction);
    bool isFinished()const;
    void forceFinished(bool finished);
    int getCurrX()const;
    int getCurrY()const;
    float getCurrVelocity()const;
    int getStartX()const;
    int getStartY()const;
    int getFinalX()const;
    int getFinalY()const;
    bool computeScrollOffset();
    void startScroll(int startX, int startY, int dx, int dy);
    void startScroll(int startX, int startY, int dx, int dy, int duration);
    bool springBack(int startX, int startY, int minX, int maxX, int minY, int maxY);
    void fling(int startX, int startY, int velocityX, int velocityY,
            int minX, int maxX, int minY, int maxY);
    void fling(int startX, int startY, int velocityX, int velocityY,
            int minX, int maxX, int minY, int maxY, int overX, int overY);
    void notifyHorizontalEdgeReached(int startX, int finalX, int overX);
    void notifyVerticalEdgeReached(int startY, int finalY, int overY);
    bool isOverScrolled()const;
    void abortAnimation();
    int timePassed()const;
    bool isScrollingInDirection(float xvel, float yvel)const;
};

}//endnamespace
#endif
