/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.SidePropagation.
 *********************************************************************************/
#include <transition/sidepropagation.h>

#include <cmath>
#include <stdexcept>

#include <core/rect.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <transition/transition.h>

namespace cdroid{

void SidePropagation::setPropagationSpeed(float propagationSpeed){
    if (propagationSpeed == 0){
        throw std::invalid_argument("propagationSpeed may not be 0");
    }
    mPropagationSpeed = propagationSpeed;
}

long SidePropagation::getStartDelay(ViewGroup* sceneRoot, Transition* transition,
        TransitionValues* startValues, TransitionValues* endValues){
    if (startValues == nullptr && endValues == nullptr){
        return 0;
    }
    int directionMultiplier = 1;
    TransitionValues* positionValues;
    if (endValues == nullptr || getViewVisibility(startValues) == View::VISIBLE){
        positionValues = startValues;
        directionMultiplier = -1;
    } else {
        positionValues = endValues;
    }

    int viewCenterX = getViewX(positionValues);
    int viewCenterY = getViewY(positionValues);

    int loc[2] = {0, 0};
    sceneRoot->getLocationOnScreen(loc);
    int left = loc[0] + (int)lround(sceneRoot->getTranslationX());
    int top = loc[1] + (int)lround(sceneRoot->getTranslationY());
    int right = left + sceneRoot->getWidth();
    int bottom = top + sceneRoot->getHeight();

    int epicenterX;
    int epicenterY;
    if (transition->getEpicenterCallback() != nullptr){
        Rect epicenter = transition->getEpicenter();
        epicenterX = epicenter.centerX();
        epicenterY = epicenter.centerY();
    } else {
        epicenterX = (left + right) / 2;
        epicenterY = (top + bottom) / 2;
    }

    float dist = distance(sceneRoot, viewCenterX, viewCenterY, epicenterX, epicenterY,
            left, top, right, bottom);
    float maxDistance = getMaxDistance(sceneRoot);
    float distanceFraction = (maxDistance != 0) ? dist / maxDistance : 0;

    int64_t duration = transition->getDuration();
    if (duration < 0){
        duration = 300;
    }
    return (long)lround(duration * directionMultiplier / mPropagationSpeed * distanceFraction);
}

int SidePropagation::distance(ViewGroup* sceneRoot, int viewX, int viewY, int epicenterX, int epicenterY,
        int left, int top, int right, int bottom){
    int side;
    if (mSide == Gravity::START){
        const bool isRtl = sceneRoot->getLayoutDirection() == View::LAYOUT_DIRECTION_RTL;
        side = isRtl ? Gravity::RIGHT : Gravity::LEFT;
    } else if (mSide == Gravity::END){
        const bool isRtl = sceneRoot->getLayoutDirection() == View::LAYOUT_DIRECTION_RTL;
        side = isRtl ? Gravity::LEFT : Gravity::RIGHT;
    } else {
        side = mSide;
    }
    int distance = 0;
    switch (side){
        case Gravity::LEFT:   distance = right - viewX + std::abs(epicenterY - viewY); break;
        case Gravity::TOP:    distance = bottom - viewY + std::abs(epicenterX - viewX); break;
        case Gravity::RIGHT:  distance = viewX - left + std::abs(epicenterY - viewY); break;
        case Gravity::BOTTOM: distance = viewY - top + std::abs(epicenterX - viewX); break;
        default: break;
    }
    return distance;
}

int SidePropagation::getMaxDistance(ViewGroup* sceneRoot){
    switch (mSide){
        case Gravity::LEFT:
        case Gravity::RIGHT:
        case Gravity::START:
        case Gravity::END:
            return sceneRoot->getWidth();
        default:
            return sceneRoot->getHeight();
    }
}

} // namespace cdroid
