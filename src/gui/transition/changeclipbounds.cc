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
#include <transition/changeclipbounds.h>

#include <animation/animator.h>
#include <animation/objectanimator.h>
#include <animation/typeevaluators.h> // RectEvaluator
#include <core/any.h>
#include <core/rect.h>
#include <view/view.h>
#include <view/viewgroup.h>

namespace cdroid {

const std::vector<std::string> ChangeClipBounds::sTransitionProperties = {PROPNAME_CLIP};

std::vector<std::string> ChangeClipBounds::getTransitionProperties() {
    return sTransitionProperties;
}

void ChangeClipBounds::captureValues(TransitionValues& values) {
    View* view = values.view;
    if (view->getVisibility() == View::GONE) {
        return;
    }
    Rect clip;
    if (view->getClipBounds(clip)) {
        values.values[PROPNAME_CLIP] = clip;          // clip present
    } else {
        values.values[PROPNAME_CLIP] = nonstd::any(); // null clip (empty any)
        values.values[PROPNAME_BOUNDS] = Rect{0, 0, view->getWidth(), view->getHeight()};
    }
}

void ChangeClipBounds::captureStartValues(TransitionValues& transitionValues) {
    captureValues(transitionValues);
}

void ChangeClipBounds::captureEndValues(TransitionValues& transitionValues) {
    captureValues(transitionValues);
}

Animator* ChangeClipBounds::createAnimator(ViewGroup* /*sceneRoot*/,
        TransitionValues* startValues, TransitionValues* endValues) {
    if (startValues == nullptr || endValues == nullptr
            || startValues->values.count(PROPNAME_CLIP) == 0
            || endValues->values.count(PROPNAME_CLIP) == 0) {
        return nullptr;
    }

    // Resolve the (possibly null) clip Rect, falling back to PROPNAME_BOUNDS when null.
    auto resolve = [](TransitionValues* tv, bool& isNull) -> Rect{
        const nonstd::any& a = tv->values.at(PROPNAME_CLIP);
        if (!a.has_value()) {
            isNull = true;
            return nonstd::any_cast<Rect>(tv->values.at(PROPNAME_BOUNDS));
        }
        isNull = false;
        return nonstd::any_cast<Rect>(a);
    };
    bool startNull;
    bool endIsNull;
    Rect start = resolve(startValues, startNull);
    Rect end   = resolve(endValues, endIsNull);
    if (startNull && endIsNull) {
        return nullptr; // No animation required since there is no clip.
    }
    if (start == end) {
        return nullptr;
    }

    endValues->view->setClipBounds(&start);
    ObjectAnimator* animator = ObjectAnimator::ofObject(endValues->view, "clipBounds",
                               RectEvaluator, {start, end});
    if (endIsNull) {
        View* endView = endValues->view;
        Animator::AnimatorListener listener;
        listener.onAnimationEnd = [endView](Animator&, bool) {
            endView->setClipBounds(nullptr);
        };
        animator->addListener(listener);
    }
    return animator;
}

} // namespace cdroid
