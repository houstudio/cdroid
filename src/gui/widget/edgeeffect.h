/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __EDGE_EFFECT_H__
#define __EDGE_EFFECT_H__
#include <core/canvas.h>
#include <core/context.h>
#include <animation/interpolators.h>

namespace cdroid{

class EdgeEffect{
private:
    /*Completely disable edge effect*/
    static constexpr int TYPE_NONE = -1;

    /*Use a color edge glow for the edge effect.*/
    static constexpr int TYPE_GLOW = 0;

    /*Use a stretch for the edge effect. */
    static constexpr int TYPE_STRETCH = 1;
    /**
     * The velocity threshold before the spring animation is considered settled.
     * The idea here is that velocity should be less than 0.1 pixel per second.
     */
    static constexpr double VELOCITY_THRESHOLD = 0.01;

    /**
     * The speed at which we should start linearly interpolating to the destination.
     * When using a spring, as it gets closer to the destination, the speed drops off exponentially.
     * Instead of landing very slowly, a better experience is achieved if the constexpr
     * destination is arrived at quicker.
     */
    static constexpr float LINEAR_VELOCITY_TAKE_OVER = 200.f;

    /**
     * The value threshold before the spring animation is considered close enough to
     * the destination to be settled. This should be around 0.01 pixel.
     */
    static constexpr double VALUE_THRESHOLD = 0.001;

    /**
     * The maximum distance at which we should start linearly interpolating to the destination.
     * When using a spring, as it gets closer to the destination, the speed drops off exponentially.
     * Instead of landing very slowly, a better experience is achieved if the constexpr
     * destination is arrived at quicker.
     */
    static constexpr double LINEAR_DISTANCE_TAKE_OVER = 8.0;

    /**
     * The natural frequency of the stretch spring.
     */
    static constexpr double NATURAL_FREQUENCY = 24.657;

    /**
     * The damping ratio of the stretch spring.
     */
    static constexpr double DAMPING_RATIO = 0.98;

    /**
     * The variation of the velocity for the stretch effect when it meets the bound.
     * if value is > 1, it will accentuate the absorption of the movement.
     */
    static constexpr float ON_ABSORB_VELOCITY_ADJUSTMENT = 13.f;

    static constexpr float LINEAR_STRETCH_INTENSITY = 0.016f;

    static constexpr float EXP_STRETCH_INTENSITY = 0.016f;

    static constexpr float SCROLL_DIST_AFFECTED_BY_EXP_STRETCH = 0.33f;
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
    static constexpr float SIN = 0.5f/*(float) std::sin(ANGLE)*/;
    static constexpr float COS = 0.86602540378f/*(float) std::cos(ANGLE)*/;
    static constexpr float RADIUS_FACTOR = 0.6f;
    float mGlowAlpha;
    float mGlowScaleY;
    float mDistance;
    float mVelocity; // only for stretch animations

    float mGlowAlphaStart;
    float mGlowAlphaFinish;
    float mGlowScaleYStart;
    float mGlowScaleYFinish;

    int64_t mStartTime;
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

    Rect mBounds;
    float mWidth;
    float mHeight;
    int mColor;
    float mRadius;
    float mBaseGlowScale;
    float mDisplacement = 0.5f;
    float mTargetDisplacement = 0.5f;
    int mEdgeEffectType = TYPE_GLOW;
private:
    void update();
    void updateSpring();
    int  getCurrentEdgeEffectBehavior();
    float calculateDistanceFromGlowValues(float scale, float alpha);
    bool isAtEquilibrium()const;
    float dampStretchVector(float normalizedVec)const;
public:
    EdgeEffect(Context* context);
    EdgeEffect(Context* context,const AttributeSet& attrs);
    virtual ~EdgeEffect();
    void setSize(int width, int height);
    bool isFinished()const;
    void finish();
    void onPull(float deltaDistance);
    virtual void onPull(float deltaDistance, float displacement);
    virtual float onPullDistance(float deltaDistance, float displacement);
    float getDistance()const;
    virtual void onRelease();
    virtual void onAbsorb(int velocity);
    void setColor(int color);
    int getColor()const;
    virtual bool draw(Canvas& canvas);
    int getMaxHeight()const;
};

}//endof namespace
#endif//__EDGE_EFFECT_H__
