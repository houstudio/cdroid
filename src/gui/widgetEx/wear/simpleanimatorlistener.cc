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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <widgetEx/wear/simpleanimatorlistener.h>
namespace cdroid{

SimpleAnimatorListener::SimpleAnimatorListener() : mWasCanceled(false) {
    // Wire the value-semantics EventSet hooks to the virtual extension points.
    // The derived method names hide the identically named base data members,
    // so the assignments below must qualify the base hooks.
    Animator::AnimatorListener::onAnimationCancel =
            [this](Animator& animator) { onAnimationCancel(animator); };
    Animator::AnimatorListener::onAnimationEnd =
            [this](Animator& animator, bool /*isReverse*/) { onAnimationEnd(animator); };
    Animator::AnimatorListener::onAnimationRepeat =
            [this](Animator& animator) { onAnimationRepeat(animator); };
    Animator::AnimatorListener::onAnimationStart =
            [this](Animator& animator, bool /*isReverse*/) { onAnimationStart(animator); };
}

void SimpleAnimatorListener::onAnimationCancel(Animator& animator) {
    mWasCanceled = true;
}

void SimpleAnimatorListener::onAnimationEnd(Animator& animator) {
    if (!mWasCanceled) {
        onAnimationComplete(animator);
    }
}

void SimpleAnimatorListener::onAnimationRepeat(Animator& animator) {
}

void SimpleAnimatorListener::onAnimationStart(Animator& animator) {
    mWasCanceled = false;
}

void SimpleAnimatorListener::onAnimationComplete(Animator& animator) {
}

bool SimpleAnimatorListener::wasCanceled() const {
    return mWasCanceled;
}

}/*endof namespace*/
