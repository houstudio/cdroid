/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.ArcMotion.
 *********************************************************************************/
#include <transition/arcmotion.h>

#include <cmath>
#include <stdexcept>

#include <core/attributeset.h>
#include <core/context.h>
#include <core/path.h>

namespace cdroid {

ArcMotion::ArcMotion() {
    mMaximumTangent = toTangent(DEFAULT_MAX_ANGLE_DEGREES);
}

ArcMotion::ArcMotion(Context* context, AttributeSet* attrs)
    : PathMotion(context, attrs) {
    // android reads ArcMotion_minimumVerticalAngle/minimumHorizontalAngle/maximumAngle.
    if (attrs != nullptr) {
        std::string v;
        v = attrs->getAttributeValue("minimumVerticalAngle");
        if (!v.empty()) setMinimumVerticalAngle((float)atof(v.c_str()));
        v = attrs->getAttributeValue("minimumHorizontalAngle");
        if (!v.empty()) setMinimumHorizontalAngle((float)atof(v.c_str()));
        v = attrs->getAttributeValue("maximumAngle");
        if (!v.empty()) setMaximumAngle((float)atof(v.c_str()));
    }
}

void ArcMotion::setMinimumHorizontalAngle(float angleInDegrees) {
    mMinimumHorizontalAngle = angleInDegrees;
    mMinimumHorizontalTangent = toTangent(angleInDegrees);
}

void ArcMotion::setMinimumVerticalAngle(float angleInDegrees) {
    mMinimumVerticalAngle = angleInDegrees;
    mMinimumVerticalTangent = toTangent(angleInDegrees);
}

void ArcMotion::setMaximumAngle(float angleInDegrees) {
    mMaximumAngle = angleInDegrees;
    mMaximumTangent = toTangent(angleInDegrees);
}

float ArcMotion::toTangent(float arcInDegrees) {
    if (arcInDegrees < 0 || arcInDegrees > 90) {
        throw std::invalid_argument("Arc must be between 0 and 90 degrees");
    }
    const float PI = 4.0f * std::atan(1.0f);
    return (float)std::tan((arcInDegrees / 2.0f) * PI / 180.0f); // tan(toRadians(arc/2))
}

Path ArcMotion::getPath(float startX, float startY, float endX, float endY) {
    Path path;
    path.moveTo(startX, startY);

    float ex, ey;
    float deltaX = endX - startX;
    float deltaY = endY - startY;
    float h2 = deltaX * deltaX + deltaY * deltaY;
    float dx = (startX + endX) / 2;
    float dy = (startY + endY) / 2;
    float midDist2 = h2 * 0.25f;
    float minimumArcDist2 = 0;
    bool isMovingUpwards = startY > endY;

    if (deltaY == 0) {
        ex = dx;
        ey = dy + (std::abs(deltaX) * 0.5f * mMinimumHorizontalTangent);
    } else if (deltaX == 0) {
        ex = dx + (std::abs(deltaY) * 0.5f * mMinimumVerticalTangent);
        ey = dy;
    } else if (std::abs(deltaX) < std::abs(deltaY)) {
        float eDistY = std::abs(h2 / (2 * deltaY));
        if (isMovingUpwards) {
            ey = endY + eDistY;
            ex = endX;
        } else {
            ey = startY + eDistY;
            ex = startX;
        }
        minimumArcDist2 = midDist2 * mMinimumVerticalTangent * mMinimumVerticalTangent;
    } else {
        float eDistX = h2 / (2 * deltaX);
        if (isMovingUpwards) {
            ex = startX + eDistX;
            ey = startY;
        } else {
            ex = endX - eDistX;
            ey = endY;
        }
        minimumArcDist2 = midDist2 * mMinimumHorizontalTangent * mMinimumHorizontalTangent;
    }
    float arcDistX = dx - ex;
    float arcDistY = dy - ey;
    float arcDist2 = arcDistX * arcDistX + arcDistY * arcDistY;
    float maximumArcDist2 = midDist2 * mMaximumTangent * mMaximumTangent;

    float newArcDistance2 = 0;
    if (arcDist2 != 0 && arcDist2 < minimumArcDist2) {
        newArcDistance2 = minimumArcDist2;
    } else if (arcDist2 > maximumArcDist2) {
        newArcDistance2 = maximumArcDist2;
    }
    if (newArcDistance2 != 0) {
        float ratio2 = newArcDistance2 / arcDist2;
        float ratio = (float)std::sqrt(ratio2);
        ex = dx + (ratio * (ex - dx));
        ey = dy + (ratio * (ey - dy));
    }
    float control1X = (startX + ex) / 2;
    float control1Y = (startY + ey) / 2;
    float control2X = (ex + endX) / 2;
    float control2Y = (ey + endY) / 2;
    path.cubicTo(control1X, control1Y, control2X, control2Y, endX, endY);
    return path;
}

} // namespace cdroid
