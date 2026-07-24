/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.CircularPropagation.
 *********************************************************************************/
#include <transition/circularpropagation.h>

#include <cmath>
#include <stdexcept>

#include <core/rect.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <transition/transition.h>

namespace cdroid{

void CircularPropagation::setPropagationSpeed(float propagationSpeed){
    if (propagationSpeed == 0){
        throw std::invalid_argument("propagationSpeed may not be 0");
    }
    mPropagationSpeed = propagationSpeed;
}

long CircularPropagation::getStartDelay(ViewGroup* sceneRoot, Transition* transition,
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

    int epicenterX;
    int epicenterY;
    // android: epicenter = transition.getEpicenter() (nullable Rect). CDROID Rect has no
    // null, so gate on the callback instead.
    if (transition->getEpicenterCallback() != nullptr){
        Rect epicenter = transition->getEpicenter();
        epicenterX = epicenter.centerX();
        epicenterY = epicenter.centerY();
    } else {
        int loc[2] = {0, 0};
        sceneRoot->getLocationOnScreen(loc);
        epicenterX = (int)lround(loc[0] + (sceneRoot->getWidth() / 2) + sceneRoot->getTranslationX());
        epicenterY = (int)lround(loc[1] + (sceneRoot->getHeight() / 2) + sceneRoot->getTranslationY());
    }
    double dist = distance(viewCenterX, viewCenterY, epicenterX, epicenterY);
    double maxDistance = distance(0, 0, sceneRoot->getWidth(), sceneRoot->getHeight());
    double distanceFraction = (maxDistance != 0) ? dist / maxDistance : 0;

    int64_t duration = transition->getDuration();
    if (duration < 0){
        duration = 300;
    }
    return (long)lround(duration * directionMultiplier / mPropagationSpeed * distanceFraction);
}

double CircularPropagation::distance(float x1, float y1, float x2, float y2){
    double x = x2 - x1;
    double y = y2 - y1;
    return std::hypot(x, y);
}

} // namespace cdroid
