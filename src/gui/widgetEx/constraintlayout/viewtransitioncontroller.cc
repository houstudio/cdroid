/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.motion.widget.ViewTransitionController.
 */
#include <widgetEx/constraintlayout/viewtransitioncontroller.h>

#include <algorithm>

#include <widgetEx/constraintlayout/motionlayout.h>
#include <widgetEx/constraintlayout/viewtransition.h>
#include <widgetEx/constraintlayout/constraintset.h>

#include <animation/valueanimator.h>
#include <core/rect.h>
#include <view/motionevent.h>
#include <view/view.h>

namespace cdroid {

ViewTransitionController::ViewTransitionController(MotionLayout* layout)
    : mMotionLayout(layout) {}

ViewTransitionController::~ViewTransitionController() {
    if (mAnimator != nullptr) {
        mAnimator->cancel();
        delete mAnimator;
    }
    ConstraintLayout::getSharedValues().removeListener(this);
}

void ViewTransitionController::add(ViewTransition* vt) {
    mViewTransitions.push_back(vt);
    mRelatedDirty = true;
    if (vt->getStateTransition() == ViewTransition::ONSTATE_SHARED_VALUE_SET ||
        vt->getStateTransition() == ViewTransition::ONSTATE_SHARED_VALUE_UNSET) {
        listenForSharedVariable(vt);
    }
}

void ViewTransitionController::remove(int id) {
    auto it = std::find_if(mViewTransitions.begin(), mViewTransitions.end(),
                           [id](ViewTransition* vt) { return vt->getId() == id; });
    if (it != mViewTransitions.end()) {
        // Note: we do NOT removeListener here. The controller is a single SharedValuesListener shared
        // across every VT watching a given key; removing one VT must not silence the others on that
        // key. The listener is left registered (harmless — onNewValue filters by mViewTransitions,
        // which no longer contains `vt`), and ~ViewTransitionController removes `this` from all keys.
        mRelatedDirty = true;
        mViewTransitions.erase(it);
    }
}

void ViewTransitionController::listenForSharedVariable(ViewTransition* vt) {
    // Dedup: register `this` only once per key (the controller is a single listener; two VTs sharing
    // a key would otherwise register it twice and onNewValue would fire each matching VT twice).
    const int key = vt->getSharedValueID();
    if (mListenedKeys.insert(key).second) {
        ConstraintLayout::getSharedValues().addListener(key, this);
    }
}

void ViewTransitionController::onNewValue(int key, int newValue, int /*oldValue*/) {
    // A shared value changed: fire each sharedValueSet/Unset ViewTransition watching this key whose
    // target value was reached (set) or left (unset). Mirrors Android listenForSharedVariable.
    const int currentId = mMotionLayout->getCurrentState();
    if (currentId == -1) return; // no firing while a transition is mid-flight
    ConstraintSet* current = mMotionLayout->getConstraintSet(currentId);
    const int count = mMotionLayout->getChildCount();
    for (ViewTransition* vt : mViewTransitions) {
        if (vt->getSharedValueID() != key) continue;
        const int mode = vt->getStateTransition();
        if (mode != ViewTransition::ONSTATE_SHARED_VALUE_SET &&
            mode != ViewTransition::ONSTATE_SHARED_VALUE_UNSET) continue;
        vt->setSharedValueCurrent(newValue);
        const bool reached = (newValue == vt->getSharedValue());
        const bool fire = (mode == ViewTransition::ONSTATE_SHARED_VALUE_SET) ? reached : !reached;
        if (!fire) continue;
        for (int i = 0; i < count; i++) {
            View* view = mMotionLayout->getChildAt(i);
            if (vt->matchesView(view)) {
                std::vector<View*> views = { view };
                vt->applyTransition(this, mMotionLayout, currentId, current, views);
            }
        }
    }
}

void ViewTransitionController::enableViewTransition(int id, bool enable) {
    for (ViewTransition* vt : mViewTransitions) {
        if (vt->getId() == id) {
            vt->setEnabled(enable);
            return;
        }
    }
}

bool ViewTransitionController::isViewTransitionEnabled(int id) const {
    for (ViewTransition* vt : mViewTransitions) {
        if (vt->getId() == id) return vt->isEnabled();
    }
    return false;
}

void ViewTransitionController::viewTransition(int id, const std::vector<View*>& views) {
    for (ViewTransition* vt : mViewTransitions) {
        if (vt->getId() != id) continue;
        const int currentId = mMotionLayout->getCurrentState();
        ConstraintSet* current = nullptr;
        // noState is independent of the main transition; the delta modes need the current set.
        if (vt->getViewTransitionMode() != ViewTransition::VIEWTRANSITIONMODE_NOSTATE) {
            if (currentId == -1) return;
            current = mMotionLayout->getConstraintSet(currentId);
        }
        vt->applyTransition(this, mMotionLayout, currentId, current, views);
        return;
    }
}

bool ViewTransitionController::applyViewTransition(int id, Motion* mc) {
    for (ViewTransition* vt : mViewTransitions) {
        if (vt->getId() == id) return vt->addAllFrames(mc);
    }
    return false;
}

void ViewTransitionController::touchEvent(const MotionEvent& evt) {
    const int currentId = mMotionLayout->getCurrentState();
    if (currentId == -1) return; // faithful: no ViewTransition support while a transition is running.

    // Lazily collect the set of children any ViewTransition targets (cache until the set changes).
    if (mRelatedDirty) {
        mRelatedViews.clear();
        const int count = mMotionLayout->getChildCount();
        for (ViewTransition* vt : mViewTransitions) {
            for (int i = 0; i < count; i++) {
                View* v = mMotionLayout->getChildAt(i);
                if (vt->matchesView(v)) mRelatedViews.push_back(v);
            }
        }
        mRelatedDirty = false;
    }

    const float x = evt.getX();
    const float y = evt.getY();
    const int action = evt.getActionMasked();

    // Let active Animates react first (reverse on release / when the finger leaves the target).
    for (auto& a : mAnimations) a->reactTo(action, x, y);

    if (action == MotionEvent::ACTION_DOWN || action == MotionEvent::ACTION_UP) {
        ConstraintSet* current = mMotionLayout->getConstraintSet(currentId); // null → delta modes no-op
        for (ViewTransition* vt : mViewTransitions) {
            if (!vt->supports(action)) continue;
            for (View* v : mRelatedViews) {
                if (!vt->matchesView(v)) continue;
                Rect rec;
                v->getHitRect(rec);
                if (rec.contains((int) x, (int) y)) {
                    std::vector<View*> views = { v };
                    vt->applyTransition(this, mMotionLayout, currentId, current, views);
                }
            }
        }
    }
}

void ViewTransitionController::addAnimation(std::unique_ptr<ViewTransition::Animate> a) {
    ViewTransition::Animate* raw = a.get();
    mAnimations.push_back(std::move(a));
    raw->mutate(); // first frame immediately (the Animate constructor in Android calls mutate()).
    if (mAnimator == nullptr) {
        // Start a repeating animator as the frame source (the per-draw animate() analog). Each tick
        // advances every active Animate by the elapsed wall-clock; cancelled once all finish.
        mAnimator = ValueAnimator::ofFloat({0.0f, 1.0f});
        mAnimator->setDuration(1000);
        mAnimator->setRepeatCount(ValueAnimator::INFINITE);
        mAnimator->addUpdateListener([this](ValueAnimator&) { animate(); });
        mAnimator->start();
    }
}

void ViewTransitionController::removeAnimation(ViewTransition::Animate* a) {
    a->mRemove = true; // deferred: animate()/stepAnimations() reaps flagged entries after the loop.
}

// Reap finished Animates and stop the frame driver when none remain.
static void reapAndMaybeStop(std::vector<std::unique_ptr<ViewTransition::Animate>>& animations,
                             ValueAnimator* animator) {
    animations.erase(
        std::remove_if(animations.begin(), animations.end(),
                       [](const std::unique_ptr<ViewTransition::Animate>& a) { return a->mRemove; }),
        animations.end());
    if (animations.empty() && animator != nullptr) animator->cancel();
}

void ViewTransitionController::animate() {
    for (auto& a : mAnimations) a->mutate();
    reapAndMaybeStop(mAnimations, mAnimator);
}

void ViewTransitionController::stepAnimations(long elapsedMs) {
    for (auto& a : mAnimations) a->stepMutate(elapsedMs);
    reapAndMaybeStop(mAnimations, mAnimator);
}

void ViewTransitionController::invalidate() {
    if (mMotionLayout) mMotionLayout->invalidate();
}

} // namespace cdroid
