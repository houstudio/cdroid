#ifndef __EDGE_EFFECT_H__
#define __EDGE_EFFECT_H__
#include <canvas.h>
#include <context.h>
#include <interpolators.h>

namespace cdroid{

class EdgeEffect{
private:
    // Time it will take the effect to fully recede in ms
    static constexpr int RECEDE_TIME = 600;
    // Time it will take before a pulled glow begins receding in ms
    static constexpr int PULL_TIME   = 167;

    // Time it will take in ms for a pulled glow to decay to partial strength before release
    static constexpr int PULL_DECAY_TIME = 2000;

    static constexpr float MAX_ALPHA =.15f ;
    static constexpr float GLOW_ALPHA_START = .09f;
    static constexpr float MAX_GLOW_SCALE   = 2.f;
    static constexpr float PULL_GLOW_BEGIN  =0.f;

    // Minimum velocity that will be absorbed
    static constexpr int MIN_VELOCITY = 100;
    // Maximum velocity, clamps at this value
    static constexpr int MAX_VELOCITY = 10000;

    static constexpr float EPSILON = 0.001f;

    static constexpr double ANGLE = M_PI / 6.f;
    static constexpr float SIN = (float) std::sin(ANGLE);
    static constexpr float COS = (float) std::cos(ANGLE);
    static constexpr float RADIUS_FACTOR = 0.6f;
    float mGlowAlpha;
    float mGlowScaleY;

    float mGlowAlphaStart;
    float mGlowAlphaFinish;
    float mGlowScaleYStart;
    float mGlowScaleYFinish;

    long mStartTime;
    float mDuration;

    Interpolator* mInterpolator;

    static constexpr int STATE_IDLE = 0;
    static constexpr int STATE_PULL = 1;
    static constexpr int STATE_ABSORB = 2;
    static constexpr int STATE_RECEDE = 3;
    static constexpr int STATE_PULL_DECAY = 4;
    static constexpr float PULL_DISTANCE_ALPHA_GLOW_FACTOR = 0.8f;

    static constexpr int VELOCITY_GLOW_FACTOR = 6;

    int mState = STATE_IDLE;

    float mPullDistance;

    RECT mBounds;
    int mColor;
    float mRadius;
    float mBaseGlowScale;
    float mDisplacement = 0.5f;
    float mTargetDisplacement = 0.5f;
private:
    void update();
public:
    EdgeEffect(Context* context);
    ~EdgeEffect();
    void setSize(int width, int height);
    bool isFinished()const;
    void finish();
    void onPull(float deltaDistance);
    void onPull(float deltaDistance, float displacement);
    void onRelease();
    void onAbsorb(int velocity);
    void setColor(int color);
    int getColor()const;
    bool draw(Canvas& canvas);
    int getMaxHeight()const;
};

}//endof namespace
#endif//__EDGE_EFFECT_H__
