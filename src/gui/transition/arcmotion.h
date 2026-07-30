/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.ArcMotion.
 *********************************************************************************/
#ifndef __CDROID_TRANSITION_ARCMOTION_H__
#define __CDROID_TRANSITION_ARCMOTION_H__

#include <transition/pathmotion.h>

namespace cdroid {

class Context;
class AttributeSet;

/**
 * A PathMotion that generates a curved path along an arc on an imaginary circle containing
 * the two points. Ported from android-36 android.transition.ArcMotion.
 */
class ArcMotion: public PathMotion {
  public:
    ArcMotion();
    ArcMotion(Context* context, AttributeSet* attrs);

    void setMinimumHorizontalAngle(float angleInDegrees);
    float getMinimumHorizontalAngle() const {
        return mMinimumHorizontalAngle;
    }
    void setMinimumVerticalAngle(float angleInDegrees);
    float getMinimumVerticalAngle() const {
        return mMinimumVerticalAngle;
    }
    void setMaximumAngle(float angleInDegrees);
    float getMaximumAngle() const {
        return mMaximumAngle;
    }

    Path getPath(float startX, float startY, float endX, float endY) override;

  private:
    static float toTangent(float arcInDegrees);

    static constexpr float DEFAULT_MIN_ANGLE_DEGREES = 0;
    static constexpr float DEFAULT_MAX_ANGLE_DEGREES = 70;

    float mMinimumHorizontalAngle = 0;
    float mMinimumVerticalAngle = 0;
    float mMaximumAngle = DEFAULT_MAX_ANGLE_DEGREES;
    float mMinimumHorizontalTangent = 0;
    float mMinimumVerticalTangent = 0;
    float mMaximumTangent; // = tan(toRadians(DEFAULT_MAX/2))
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_ARCMOTION_H__
