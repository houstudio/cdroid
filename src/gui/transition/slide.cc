/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.Slide.
 *********************************************************************************/
#include <transition/slide.h>

#include <cstdlib>
#include <stdexcept>

#include <animation/interpolators.h>
#include <core/any.h>
#include <core/attributeset.h>
#include <core/context.h>
#include <view/view.h>
#include <view/viewgroup.h>

#include <transition/sidepropagation.h>
#include <transition/translationanimationcreator.h>

namespace cdroid{

namespace {
DecelerateInterpolator sDecelerate;
AccelerateInterpolator sAccelerate;
} // anonymous namespace

// Struct field order is { getGoneX, getGoneY }.
// Horizontal calculators: getGoneY unchanged; Vertical calculators: getGoneX unchanged.

const Slide::CalculateSlide Slide::sCalculateLeft = {
    [](ViewGroup* sr, View* v, float f){ return v->getTranslationX() - sr->getWidth() * f; },
    [](ViewGroup*, View* v, float){ return v->getTranslationY(); }
};

const Slide::CalculateSlide Slide::sCalculateStart = {
    [](ViewGroup* sr, View* v, float f){
        const bool isRtl = sr->getLayoutDirection() == View::LAYOUT_DIRECTION_RTL;
        return isRtl ? (v->getTranslationX() + sr->getWidth() * f)
                     : (v->getTranslationX() - sr->getWidth() * f);
    },
    [](ViewGroup*, View* v, float){ return v->getTranslationY(); }
};

const Slide::CalculateSlide Slide::sCalculateTop = {
    [](ViewGroup*, View* v, float){ return v->getTranslationX(); },
    [](ViewGroup* sr, View* v, float f){ return v->getTranslationY() - sr->getHeight() * f; }
};

const Slide::CalculateSlide Slide::sCalculateRight = {
    [](ViewGroup* sr, View* v, float f){ return v->getTranslationX() + sr->getWidth() * f; },
    [](ViewGroup*, View* v, float){ return v->getTranslationY(); }
};

const Slide::CalculateSlide Slide::sCalculateEnd = {
    [](ViewGroup* sr, View* v, float f){
        const bool isRtl = sr->getLayoutDirection() == View::LAYOUT_DIRECTION_RTL;
        return isRtl ? (v->getTranslationX() - sr->getWidth() * f)
                     : (v->getTranslationX() + sr->getWidth() * f);
    },
    [](ViewGroup*, View* v, float){ return v->getTranslationY(); }
};

const Slide::CalculateSlide Slide::sCalculateBottom = {
    [](ViewGroup*, View* v, float){ return v->getTranslationX(); },
    [](ViewGroup* sr, View* v, float f){ return v->getTranslationY() + sr->getHeight() * f; }
};

Slide::Slide(){
    setSlideEdge(Gravity::BOTTOM);
}

Slide::Slide(int slideEdge){
    setSlideEdge(slideEdge);
}

Slide::Slide(Context* context, AttributeSet* attrs)
    : Visibility(context, attrs){
    int edge = Gravity::BOTTOM;
    if (attrs != nullptr){
        std::string e = attrs->getAttributeValue("slideEdge");
        if (!e.empty()) edge = atoi(e.c_str());
    }
    setSlideEdge(edge);
}

void Slide::setSlideEdge(int slideEdge){
    switch (slideEdge){
        case Gravity::LEFT:   mSlideCalculator = &sCalculateLeft; break;
        case Gravity::TOP:    mSlideCalculator = &sCalculateTop; break;
        case Gravity::RIGHT:  mSlideCalculator = &sCalculateRight; break;
        case Gravity::BOTTOM: mSlideCalculator = &sCalculateBottom; break;
        case Gravity::START:  mSlideCalculator = &sCalculateStart; break;
        case Gravity::END:    mSlideCalculator = &sCalculateEnd; break;
        default:
            throw std::invalid_argument("Invalid slide direction");
    }
    mSlideEdge = slideEdge;
    SidePropagation* propagation = new SidePropagation();
    propagation->setSide(slideEdge);
    setPropagation(propagation);
}

void Slide::captureValues(TransitionValues& transitionValues){
    View* view = transitionValues.view;
    int position[2] = {0, 0};
    view->getLocationOnScreen(position);
    transitionValues.values[PROPNAME_SCREEN_POSITION] = std::vector<int>{position[0], position[1]};
}

void Slide::captureStartValues(TransitionValues& transitionValues){
    Visibility::captureStartValues(transitionValues);
    captureValues(transitionValues);
}

void Slide::captureEndValues(TransitionValues& transitionValues){
    Visibility::captureEndValues(transitionValues);
    captureValues(transitionValues);
}

Animator* Slide::onAppear(ViewGroup* sceneRoot, View* view,
        TransitionValues* /*startValues*/, TransitionValues* endValues){
    if (endValues == nullptr){
        return nullptr;
    }
    const std::vector<int>* position = nonstd::any_cast<std::vector<int>>(&endValues->values.at(PROPNAME_SCREEN_POSITION));
    float endX = view->getTranslationX();
    float endY = view->getTranslationY();
    float startX = mSlideCalculator->getGoneX(sceneRoot, view, mSlideFraction);
    float startY = mSlideCalculator->getGoneY(sceneRoot, view, mSlideFraction);
    return TranslationAnimationCreator::createAnimation(view, endValues, (*position)[0], (*position)[1],
            startX, startY, endX, endY, &sDecelerate, this);
}

Animator* Slide::onDisappear(ViewGroup* sceneRoot, View* view,
        TransitionValues* startValues, TransitionValues* /*endValues*/){
    if (startValues == nullptr){
        return nullptr;
    }
    const std::vector<int>* position = nonstd::any_cast<std::vector<int>>(&startValues->values.at(PROPNAME_SCREEN_POSITION));
    float startX = view->getTranslationX();
    float startY = view->getTranslationY();
    float endX = mSlideCalculator->getGoneX(sceneRoot, view, mSlideFraction);
    float endY = mSlideCalculator->getGoneY(sceneRoot, view, mSlideFraction);
    return TranslationAnimationCreator::createAnimation(view, startValues, (*position)[0], (*position)[1],
            startX, startY, endX, endY, &sAccelerate, this);
}

} // namespace cdroid
