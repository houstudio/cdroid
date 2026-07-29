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
 *********************************************************************************/

/*
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
#include <unordered_set>
#include <vector>

#include <widgetEx/constraintlayout/sharedvalues.h>
#include <widgetEx/constraintlayout/motion/viewtransition.h>

namespace cdroid {

class MotionEvent;
class MotionLayout;
class ValueAnimator;
class View;

class ViewTransitionController : public SharedValues::SharedValuesListener {
  public:
    explicit ViewTransitionController(MotionLayout* layout);
    ~ViewTransitionController();

    // SharedValues::SharedValuesListener — fires sharedValueSet/Unset ViewTransitions whose target
    // value was reached/left.
    void onNewValue(int key, int newValue, int oldValue) override;

    // Register a parsed <ViewTransition>. Borrowed — the MotionScene owns the ViewTransition.
    void add(ViewTransition* vt);
    void remove(int id);

    void enableViewTransition(int id, bool enable);
    bool isViewTransitionEnabled(int id) const;

    // Programmatic fire (Android viewTransition(int, View...)): animate `views` per the ViewTransition
    // registered under `id`.
    void viewTransition(int id, const std::vector<View*>& views);
    // Merge the keyframes of the ViewTransition registered under `id` into `mc` (Android
    // applyViewTransition). Returns true if the ViewTransition was found.
    bool applyViewTransition(int id, class Motion* mc);

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
    std::unordered_set<int> mListenedKeys; // shared-value keys this controller already listens on
                                           // (dedup: the controller is a single SharedValuesListener,
                                           // so two VTs sharing a key must register `this` only once)
    // For a sharedValueSet/Unset ViewTransition, register this controller as a SharedValues listener
    // for the VT's shared-value id.
    void listenForSharedVariable(ViewTransition* vt);
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_VIEW_TRANSITION_CONTROLLER_H
