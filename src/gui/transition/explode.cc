/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.Explode.
 *********************************************************************************/
#include <transition/explode.h>

#include <cmath>
#include <cstdlib>

#include <animation/interpolators.h>
#include <core/any.h>
#include <core/rect.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <widget/R.h>

#include <transition/circularpropagation.h>
#include <transition/translationanimationcreator.h>

namespace cdroid{

namespace {
DecelerateInterpolator sDecelerate;
AccelerateInterpolator sAccelerate;
} // anonymous namespace

Explode::Explode(){
    setPropagation(new CircularPropagation());
}

Explode::Explode(Context* context, AttributeSet* attrs)
    : Visibility(context, attrs){
    setPropagation(new CircularPropagation());
}

void Explode::captureValues(TransitionValues& transitionValues){
    View* view = transitionValues.view;
    view->getLocationOnScreen(mTempLoc);
    int left = mTempLoc[0];
    int top = mTempLoc[1];
    int right = left + view->getWidth();
    int bottom = top + view->getHeight();
    transitionValues.values[PROPNAME_SCREEN_BOUNDS] = Rect::MakeLTRB(left, top, right, bottom);
}

void Explode::captureStartValues(TransitionValues& transitionValues){
    Visibility::captureStartValues(transitionValues);
    captureValues(transitionValues);
}

void Explode::captureEndValues(TransitionValues& transitionValues){
    Visibility::captureEndValues(transitionValues);
    captureValues(transitionValues);
}

Animator* Explode::onAppear(ViewGroup* sceneRoot, View* view,
        TransitionValues* /*startValues*/, TransitionValues* endValues){
    if (endValues == nullptr){
        return nullptr;
    }
    Rect bounds = nonstd::any_cast<Rect>(endValues->values.at(PROPNAME_SCREEN_BOUNDS));
    float endX = view->getTranslationX();
    float endY = view->getTranslationY();
    calculateOut(sceneRoot, bounds, mTempLoc);
    float startX = endX + mTempLoc[0];
    float startY = endY + mTempLoc[1];
    return TranslationAnimationCreator::createAnimation(view, endValues, bounds.left, bounds.top,
            startX, startY, endX, endY, &sDecelerate, this);
}

Animator* Explode::onDisappear(ViewGroup* sceneRoot, View* view,
        TransitionValues* startValues, TransitionValues* /*endValues*/){
    if (startValues == nullptr){
        return nullptr;
    }
    Rect bounds = nonstd::any_cast<Rect>(startValues->values.at(PROPNAME_SCREEN_BOUNDS));
    int viewPosX = bounds.left;
    int viewPosY = bounds.top;
    float startX = view->getTranslationX();
    float startY = view->getTranslationY();
    float endX = startX;
    float endY = startY;
    int* interruptedPosition = static_cast<int*>(startValues->view->getTag(R::id::transitionPosition));
    if (interruptedPosition != nullptr){
        // End position relative to the interrupted position, not the original start.
        endX += interruptedPosition[0] - bounds.left;
        endY += interruptedPosition[1] - bounds.top;
        bounds.offsetTo(interruptedPosition[0], interruptedPosition[1]);
    }
    calculateOut(sceneRoot, bounds, mTempLoc);
    endX += mTempLoc[0];
    endY += mTempLoc[1];
    return TranslationAnimationCreator::createAnimation(view, startValues, viewPosX, viewPosY,
            startX, startY, endX, endY, &sAccelerate, this);
}

void Explode::calculateOut(View* sceneRoot, Rect& bounds, int* outVector){
    sceneRoot->getLocationOnScreen(mTempLoc);
    int sceneRootX = mTempLoc[0];
    int sceneRootY = mTempLoc[1];
    int focalX;
    int focalY;
    if (getEpicenterCallback() == nullptr){
        focalX = sceneRootX + (sceneRoot->getWidth() / 2) + (int)lround(sceneRoot->getTranslationX());
        focalY = sceneRootY + (sceneRoot->getHeight() / 2) + (int)lround(sceneRoot->getTranslationY());
    } else {
        Rect epicenter = getEpicenter();
        focalX = epicenter.centerX();
        focalY = epicenter.centerY();
    }

    int centerX = bounds.centerX();
    int centerY = bounds.centerY();
    double xVector = centerX - focalX;
    double yVector = centerY - focalY;
    if (xVector == 0 && yVector == 0){
        // Random direction when View is centered on focal View.
        xVector = ((double)std::rand() / RAND_MAX) * 2 - 1;
        yVector = ((double)std::rand() / RAND_MAX) * 2 - 1;
    }
    double vectorSize = std::hypot(xVector, yVector);
    xVector /= vectorSize;
    yVector /= vectorSize;
    double maxDistance = calculateMaxDistance(sceneRoot, focalX - sceneRootX, focalY - sceneRootY);
    outVector[0] = (int)lround(maxDistance * xVector);
    outVector[1] = (int)lround(maxDistance * yVector);
}

double Explode::calculateMaxDistance(View* sceneRoot, int focalX, int focalY){
    int maxX = std::max(focalX, sceneRoot->getWidth() - focalX);
    int maxY = std::max(focalY, sceneRoot->getHeight() - focalY);
    return std::hypot(maxX, maxY);
}

} // namespace cdroid
