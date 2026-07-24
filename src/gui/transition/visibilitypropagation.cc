/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.VisibilityPropagation.
 *********************************************************************************/
#include <transition/visibilitypropagation.h>

#include <cmath>

#include <core/any.h>
#include <view/view.h>
#include <transition/transitionvalues.h>

namespace cdroid{

void VisibilityPropagation::captureValues(TransitionValues* values){
    View* view = values->view;
    int visibility = View::GONE;
    // Visibility transition stores "android:visibility:visibility"; fall back to current.
    auto it = values->values.find("android:visibility:visibility");
    if (it != values->values.end() && it->second.has_value()){
        visibility = nonstd::any_cast<int>(it->second);
    } else {
        visibility = view->getVisibility();
    }
    values->values[PROPNAME_VISIBILITY] = visibility;

    int loc[2] = {0, 0};
    view->getLocationOnScreen(loc);
    loc[0] += (int)lround(view->getTranslationX());
    loc[0] += view->getWidth() / 2;
    loc[1] += (int)lround(view->getTranslationY());
    loc[1] += view->getHeight() / 2;
    values->values[PROPNAME_VIEW_CENTER] = std::vector<int>{loc[0], loc[1]};
}

std::vector<std::string> VisibilityPropagation::getPropagationProperties(){
    return {PROPNAME_VISIBILITY, PROPNAME_VIEW_CENTER};
}

int VisibilityPropagation::getViewVisibility(TransitionValues* values){
    if (values == nullptr){
        return View::GONE;
    }
    auto it = values->values.find(PROPNAME_VISIBILITY);
    if (it == values->values.end() || !it->second.has_value()){
        return View::GONE;
    }
    return nonstd::any_cast<int>(it->second);
}

int VisibilityPropagation::getViewCoordinate(TransitionValues* values, int coordinateIndex){
    if (values == nullptr){
        return -1;
    }
    auto it = values->values.find(PROPNAME_VIEW_CENTER);
    if (it == values->values.end() || !it->second.has_value()){
        return -1;
    }
    const std::vector<int>* coordinates = nonstd::any_cast<std::vector<int>>(&it->second);
    if (coordinates == nullptr || coordinates->size() <= (size_t)coordinateIndex){
        return -1;
    }
    return (*coordinates)[coordinateIndex];
}

} // namespace cdroid
