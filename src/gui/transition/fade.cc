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
#include <transition/fade.h>

#include <cstdlib>

#include <animation/animator.h>
#include <animation/objectanimator.h>
#include <core/attributeset.h>
#include <core/context.h>
#include <core/any.h>
#include <porting/cdlog.h>
#include <view/view.h>
#include <view/viewgroup.h>

#include <transition/transitionlisteneradapter.h>

namespace cdroid {

namespace {

constexpr const char* LOG_TAG = "Fade";
constexpr bool DBG = false;

// android: anonymous TransitionListener in createAnimation that resets transitionAlpha
// to 1 when the transition ends. Now wired inline as an EventSet TransitionListener value
// (see createAnimation) — no subclass / no new.

} // anonymous namespace

Fade::Fade(Context* context, AttributeSet* attrs)
    : Visibility(context, attrs) {
    // android: obtainStyledAttributes(attrs, R.styleable.Fade) → fadingMode (default getMode()).
    // CDROID reads the attribute directly (TypedArray is rarely used).
    int fadingMode = getMode();
    if (attrs != nullptr) {
        std::string fm = attrs->getAttributeValue("fadingMode");
        if (!fm.empty()) {
            int parsed = atoi(fm.c_str());
            if (parsed == IN || parsed == OUT || parsed == (IN | OUT)) {
                fadingMode = parsed;
            }
        }
    }
    setMode(fadingMode);
}

void Fade::captureStartValues(TransitionValues& transitionValues) {
    Visibility::captureStartValues(transitionValues);
    transitionValues.values[PROPNAME_TRANSITION_ALPHA] = transitionValues.view->getTransitionAlpha();
}

Animator* Fade::createAnimation(View* view, float startAlpha, float endAlpha) {
    if (startAlpha == endAlpha) {
        return nullptr;
    }
    view->setTransitionAlpha(startAlpha);
    ObjectAnimator* anim = ObjectAnimator::ofFloat(view, "transitionAlpha", {endAlpha});
    if (DBG) {
        LOGD("%s: Created animator %p", LOG_TAG, (void*)anim);
    }

    FadeAnimatorListener* listenerState = new FadeAnimatorListener(view);
    Animator::AnimatorListener listener;
    listener.onAnimationStart = [listenerState](Animator& a, bool) {
        listenerState->onAnimationStart(a);
    };
    listener.onAnimationEnd   = [listenerState](Animator& a, bool) {
        listenerState->onAnimationEnd(a);
        delete listenerState;
    };
    anim->addListener(listener);

    Transition::TransitionListener endListener;
    endListener.onTransitionEnd = [view](Transition&) {
        view->setTransitionAlpha(1);
    };
    addListener(endListener);
    return anim;
}

Animator* Fade::onAppear(ViewGroup* /*sceneRoot*/, View* view,
                         TransitionValues* startValues, TransitionValues* /*endValues*/) {
    if (DBG) {
        LOGD("%s: Fade.onAppear: view=%p", LOG_TAG, (void*)view);
    }
    float startAlpha = getStartAlpha(startValues, 0);
    if (startAlpha == 1) {
        startAlpha = 0;
    }
    return createAnimation(view, startAlpha, 1);
}

Animator* Fade::onDisappear(ViewGroup* /*sceneRoot*/, View* view,
                            TransitionValues* startValues, TransitionValues* /*endValues*/) {
    float startAlpha = getStartAlpha(startValues, 1);
    return createAnimation(view, startAlpha, 0);
}

float Fade::getStartAlpha(TransitionValues* startValues, float fallbackValue) {
    float startAlpha = fallbackValue;
    if (startValues != nullptr) {
        auto it = startValues->values.find(PROPNAME_TRANSITION_ALPHA);
        if (it != startValues->values.end() && it->second.has_value()) {
            startAlpha = nonstd::any_cast<float>(it->second);
        }
    }
    return startAlpha;
}

void Fade::FadeAnimatorListener::onAnimationStart(Animator& /*animation*/) {
    if (mView->hasOverlappingRendering() && mView->getLayerType() == View::LAYER_TYPE_NONE) {
        mLayerTypeChanged = true;
        mView->setLayerType(View::LAYER_TYPE_HARDWARE); // CDROID setLayerType(int) — no Paint arg
    }
}

void Fade::FadeAnimatorListener::onAnimationEnd(Animator& /*animation*/) {
    mView->setTransitionAlpha(1);
    if (mLayerTypeChanged) {
        mView->setLayerType(View::LAYER_TYPE_NONE);
    }
}

} // namespace cdroid
