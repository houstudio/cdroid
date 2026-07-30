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
#include <transition/rotate.h>

#include <animation/objectanimator.h>
#include <view/view.h>
#include <view/viewgroup.h>

namespace cdroid {

void Rotate::captureStartValues(TransitionValues& transitionValues) {
    transitionValues.values[PROPNAME_ROTATION] = transitionValues.view->getRotation();
}

void Rotate::captureEndValues(TransitionValues& transitionValues) {
    transitionValues.values[PROPNAME_ROTATION] = transitionValues.view->getRotation();
}

Animator* Rotate::createAnimator(ViewGroup* /*sceneRoot*/,
                                 TransitionValues* startValues, TransitionValues* endValues) {
    if (startValues == nullptr || endValues == nullptr) {
        return nullptr;
    }
    View* view = endValues->view;
    float startRotation = nonstd::any_cast<float>(startValues->values.at(PROPNAME_ROTATION));
    float endRotation   = nonstd::any_cast<float>(endValues->values.at(PROPNAME_ROTATION));
    if (startRotation != endRotation) {
        view->setRotation(startRotation);
        return ObjectAnimator::ofFloat(view, View::ROTATION, {startRotation, endRotation});
    }
    return nullptr;
}

} // namespace cdroid
