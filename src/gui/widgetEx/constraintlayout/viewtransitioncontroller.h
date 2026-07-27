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
 *
 * Owns the parsed <ViewTransition>s of a MotionScene and drives their independent per-view
 * animations. It is the touch/fire/animation hub for ViewTransitions:
 *   - touchEvent(evt)   — forwarded from MotionLayout::onInterceptTouchEvent; fires actionDown/Up
 *                         ViewTransitions whose target is under the touch and lets active Animates
 *                         react (reverse on ACTION_UP / when the finger leaves the hit rect);
 *   - viewTransition(id, views) — programmatic fire (MotionLayout.viewTransition);
 *   - animate()         — per-frame tick (Android dispatchDraw); advances each active Animate by
 *                         the elapsed wall-clock time and reaps finished ones.
 *
 * CDROID adaptation: the frame source is a single repeating ValueAnimator (the analog of Android's
 * per-draw animate() call), started when the first Animate registers and cancelled when the last one
 * finishes. Each Animate computes its own progress from the wall clock (SystemClock::uptimeMillis,
 * the analog of Android's System.nanoTime).
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_VIEW_TRANSITION_CONTROLLER_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_VIEW_TRANSITION_CONTROLLER_H

#include <memory>
#include <vector>

#include <widgetEx/constraintlayout/viewtransition.h>

namespace cdroid {

class MotionEvent;
class MotionLayout;
class ValueAnimator;
class View;

class ViewTransitionController {
  public:
    explicit ViewTransitionController(MotionLayout* layout);
    ~ViewTransitionController();

    // Register a parsed <ViewTransition>. Borrowed — the MotionScene owns the ViewTransition.
    void add(ViewTransition* vt);
    void remove(int id);

    void enableViewTransition(int id, bool enable);
    bool isViewTransitionEnabled(int id) const;

    // Programmatic fire (Android viewTransition(int, View...)): animate `views` per the ViewTransition
    // registered under `id`.
    void viewTransition(int id, const std::vector<View*>& views);

    // Touch dispatch from MotionLayout::onInterceptTouchEvent.
    void touchEvent(const MotionEvent& evt);

    // Per-frame tick (Android animate(), called from dispatchDraw). Mutates active Animates by the
    // elapsed wall-clock and reaps finished ones.
    void animate();
    // Advance every active Animate by a forced `elapsedMs` (test/programmatic; bypasses the clock).
    void stepAnimations(long elapsedMs);

    void invalidate();

    size_t animationCount() const {
        return mAnimations.size();
    }

    // For ViewTransition::Animate self-registration (transfers ownership; starts the frame driver and
    // runs the first frame). removeAnimation flags an Animate for deferred removal (it may be called
    // from inside the mutate() loop, so removal is deferred to the end of animate()/stepAnimations()).
    void addAnimation(std::unique_ptr<ViewTransition::Animate> a);
    void removeAnimation(ViewTransition::Animate* a);

  private:
    MotionLayout* mMotionLayout;
    std::vector<ViewTransition*> mViewTransitions;              // borrowed (MotionScene owns)
    std::vector<View*> mRelatedViews;                           // cached matchesView set
    bool mRelatedDirty = true;
    std::vector<std::unique_ptr<ViewTransition::Animate>> mAnimations;
    ValueAnimator* mAnimator = nullptr;                         // repeating frame-tick source
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_VIEW_TRANSITION_CONTROLLER_H
