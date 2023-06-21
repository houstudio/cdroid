#ifndef __SCROLLER_H__
#define __SCROLLER_H__
#include <animation/interpolators.h>
#include <core/context.h>
#include <math.h>

namespace cdroid{

class Scroller  {
private:
    Interpolator* mInterpolator;
    int mMode;
    int mStartX;
    int mStartY;
    int mFinalX;
    int mFinalY;
    int mMinX;
    int mMaxX;
    int mMinY;
    int mMaxY;

    int mCurrX;
    int mCurrY;
    long mStartTime;
    int mDuration;
    float mDurationReciprocal;
    float mDeltaX;
    float mDeltaY;
    bool mFinished;
    bool mFlywheel;

    float mVelocity;
    float mCurrVelocity;
    int mDistance;

    float mFlingFriction;

    static constexpr int DEFAULT_DURATION = 250;
    static constexpr int SCROLL_MODE = 0;
    static constexpr int FLING_MODE = 1;

    static constexpr float DECELERATION_RATE = (float) (log(0.78) / log(0.9));
    static constexpr float INFLEXION = 0.35f; // Tension lines cross at (INFLEXION, 1)
    static constexpr float START_TENSION = 0.5f;
    static constexpr float END_TENSION = 1.0f;
    static constexpr float P1 = START_TENSION * INFLEXION;
    static constexpr float P2 = 1.0f - END_TENSION * (1.0f - INFLEXION);

    static constexpr int NB_SAMPLES = 100;
    static float SPLINE_POSITION [NB_SAMPLES + 1];
    static float SPLINE_TIME [NB_SAMPLES + 1];

    float mDeceleration;
    float mPpi;

    // A context-specific coefficient adjusted to physical values.
    float mPhysicalCoeff;
private:
    static void sInit();
    double getSplineDeceleration(float velocity);
    int getSplineFlingDuration(float velocity);
    double getSplineFlingDistance(float velocity);
public:
    Scroller(Context* context);
    Scroller(Context* context, Interpolator* interpolator,bool flywheel=true);
    ~Scroller();
    void setFriction(float friction);
    float computeDeceleration(float friction);
    bool isFinished()const;
    void forceFinished(bool finished);
    int getDuration()const;
    int getCurrX()const;
    int getCurrY()const;
    float getCurrVelocity()const;
    int getStartX()const ;
    int getStartY()const ;
    int getFinalX()const ;
    int getFinalY()const ;
    bool computeScrollOffset();
    void startScroll(int startX, int startY, int dx, int dy);
    void startScroll(int startX, int startY, int dx, int dy, int duration);
    void fling(int startX, int startY, int velocityX, int velocityY,
            int minX, int maxX, int minY, int maxY);
    void abortAnimation();
    void extendDuration(int extend);
    int timePassed()const;
    void setFinalX(int newX);
    void setFinalY(int newY);
    bool isScrollingInDirection(float xvel, float yvel);

    class ViscousFluidInterpolator:public Interpolator{
    private:
        static constexpr float VISCOUS_FLUID_SCALE = 8.0f;
        static float VISCOUS_FLUID_NORMALIZE;
        static float VISCOUS_FLUID_OFFSET;
        static float viscousFluid(float x);
    public:
        float getInterpolation(float input)override;
    };
};//Scroller
}//end namespace
#endif
