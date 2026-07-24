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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *********************************************************************************/
#include <transition/changescroll.h>

#include <animation/objectanimator.h>
#include <core/attributeset.h>
#include <core/context.h>
#include <view/view.h>
#include <view/viewgroup.h>

#include <transition/transitionutils.h>

namespace cdroid{

const std::vector<std::string> ChangeScroll::PROPERTIES = {PROPNAME_SCROLL_X, PROPNAME_SCROLL_Y};

ChangeScroll::ChangeScroll(Context* context, AttributeSet* attrs)
    : Transition(context, attrs){
}

void ChangeScroll::captureStartValues(TransitionValues& transitionValues){
    captureValues(transitionValues);
}

void ChangeScroll::captureEndValues(TransitionValues& transitionValues){
    captureValues(transitionValues);
}

std::vector<std::string> ChangeScroll::getTransitionProperties(){
    return PROPERTIES;
}

void ChangeScroll::captureValues(TransitionValues& transitionValues){
    transitionValues.values[PROPNAME_SCROLL_X] = transitionValues.view->getScrollX();
    transitionValues.values[PROPNAME_SCROLL_Y] = transitionValues.view->getScrollY();
}

Animator* ChangeScroll::createAnimator(ViewGroup* /*sceneRoot*/,
        TransitionValues* startValues, TransitionValues* endValues){
    if (startValues == nullptr || endValues == nullptr){
        return nullptr;
    }
    View* view = endValues->view;
    int startX = nonstd::any_cast<int>(startValues->values.at(PROPNAME_SCROLL_X));
    int endX   = nonstd::any_cast<int>(endValues->values.at(PROPNAME_SCROLL_X));
    int startY = nonstd::any_cast<int>(startValues->values.at(PROPNAME_SCROLL_Y));
    int endY   = nonstd::any_cast<int>(endValues->values.at(PROPNAME_SCROLL_Y));
    Animator* scrollXAnimator = nullptr;
    Animator* scrollYAnimator = nullptr;
    if (startX != endX){
        view->setScrollX(startX);
        scrollXAnimator = ObjectAnimator::ofInt(view, "scrollX", {startX, endX});
    }
    if (startY != endY){
        view->setScrollY(startY);
        scrollYAnimator = ObjectAnimator::ofInt(view, "scrollY", {startY, endY});
    }
    return TransitionUtils::mergeAnimators(scrollXAnimator, scrollYAnimator);
}

} // namespace cdroid
