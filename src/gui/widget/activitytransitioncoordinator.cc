/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — port of android.app.ActivityTransitionCoordinator (android-36), scoped to
 * the cross-Window shared-element flight. See the header for the design.
 *********************************************************************************/
#include <widget/activitytransitioncoordinator.h>

#include <animation/animator.h>
#include <animation/valueanimator.h>
#include <core/canvas.h>
#include <porting/cdlog.h>
#include <transition/transitionutils.h>
#include <view/viewgroup.h>
#include <view/viewgroupoverlay.h>
#include <widget/cdwindow.h>

namespace cdroid {

namespace {

// AOSP's default shared-element duration (a Transition with no explicit duration).
constexpr int64_t FLIGHT_DURATION_MS = 300;

// The ghost: draws the captured snapshot stretched to this view's frame. AOSP renders the
// pixels from a Bitmap in an overlay slot; stretching (not aspect-preserving) matches the
// ChangeBounds+ChangeTransform snapshot morph.
class SnapshotView : public View {
public:
    SnapshotView(Context* ctx, const Cairo::RefPtr<Cairo::ImageSurface>& snapshot)
        : View(ctx), mSnapshot(snapshot) {}
protected:
    void onDraw(Canvas& canvas) override {
        if (!mSnapshot || getWidth() <= 0 || getHeight() <= 0) return;
        const double sw = mSnapshot->get_width(), sh = mSnapshot->get_height();
        canvas.save();
        canvas.scale(getWidth() / sw, getHeight() / sh);
        canvas.set_source(mSnapshot, 0, 0);
        canvas.paint();
        canvas.restore();
    }
private:
    Cairo::RefPtr<Cairo::ImageSurface> mSnapshot;
};

// AOSP ActivityTransitionCoordinator.createSnapshots: capture a view's pixels at its
// current size. createViewBitmap's MAX_IMAGE_SIZE downscale applies (a giant shared
// view yields a clamped surface the ghost stretches anyway); a null RefPtr comes back
// for an unmeasured view (bounds with no area) or an unattached one with no sceneRoot.
Cairo::RefPtr<Cairo::ImageSurface> snapshotView(View* v) {
    Cairo::Matrix matrix = Cairo::identity_matrix();  // capture in view-local coordinates
    Cairo::Rectangle bounds = {0.0, 0.0, (double)v->getWidth(), (double)v->getHeight()};
    return TransitionUtils::createViewBitmap(v, matrix, bounds, nullptr);
}

// View-tree search by transitionName (androidx FragmentManager has the same helper; the
// widget layer keeps its own copy to avoid the fragment dependency).
View* findViewByTransitionName(View* root, const std::string& name) {
    if (!root || name.empty()) return nullptr;
    if (root->getTransitionName() == name) return root;
    if (ViewGroup* vg = dynamic_cast<ViewGroup*>(root)) {
        for (int i = 0; i < vg->getChildCount(); i++) {
            if (View* found = findViewByTransitionName(vg->getChildAt(i), name)) return found;
        }
    }
    return nullptr;
}

Rect lerpRect(const Rect& a, const Rect& b, float f) {
    return Rect::MakeLTRB((int)(a.left     + (b.left     - a.left)     * f),
                          (int)(a.top      + (b.top      - a.top)      * f),
                          (int)(a.right()  + (b.right()  - a.right())  * f),
                          (int)(a.bottom() + (b.bottom() - a.bottom()) * f));
}

// Reveal a view hidden with setTransitionVisibility(INVISIBLE) at flight landing.
void revealView(View* v) {
    if (v && v->isAttachedToWindow()) {
        v->setTransitionVisibility(View::VISIBLE);
        v->invalidate();
    }
}

} // namespace

ActivityTransitionCoordinator::ActivityTransitionCoordinator(Window* host, Window* caller,
        const std::vector<std::pair<View*, std::string>>& sharedElements)
    : mHost(host), mCaller(caller),
      mCallerLiveness(caller ? caller->getSceneLiveness() : std::shared_ptr<bool>()) {
    // Capture the exit scene NOW (the caller's pixels/bounds at startActivity — AOSP captures
    // before the new window covers it; no caller redraw can interleave on this same stack).
    // The host tree is built (its ctor ran) though not yet laid out, so each pair is resolved
    // by name FIRST — findViewByTransitionName walks the tree, no bounds needed — and only
    // matching pairs are captured: a launch whose target has no matching transitionName
    // rasterizes nothing on the open path (AOSP times out and falls back the same way).
    for (const auto& p : sharedElements) {
        if (!p.first || p.second.empty()) continue;
        if (findViewByTransitionName(host, p.second) == nullptr) continue;
        SharedElementSource src;
        src.view = p.first;
        src.name = p.second;
        int loc[2] = {0, 0};
        p.first->getLocationOnScreen(loc);
        src.l = loc[0]; src.t = loc[1];
        src.w = p.first->getWidth(); src.h = p.first->getHeight();
        src.snapshot = snapshotView(p.first);
        if (src.snapshot) mSources.push_back(src);
    }
}

ActivityTransitionCoordinator::~ActivityTransitionCoordinator() {
    mTornDown = true;
    cancelAnimator();
    // Enter-flight ghosts live in the HOST's overlay and die with the host's tree (this dtor
    // runs from ~Window before the overlay teardown). Return-flight ghosts live in the
    // CALLER's overlay — free them and restore the caller's hidden targets while the caller
    // is still alive; if it is gone its tree already freed the ghosts (no dereference).
    if (mFlightsInCaller && mCaller && !mCallerLiveness.expired()) {
        ViewGroupOverlay* overlay = mCaller->getOverlay();
        for (SceneFlight& fl : mSceneFlights) {
            overlay->remove(fl.ghost);
            delete fl.ghost;
        }
        for (SharedElementSource& src : mSources) revealView(src.view);
    }
    mSceneFlights.clear();
}

bool ActivityTransitionCoordinator::prepareEnter() {
    // Already prepared: the enter flight may since have finished (mSceneFlights cleared),
    // but the coordinator stays alive holding the sources for the return flight — returning
    // false here would make Window's doTraversal hook drop it.
    if (mEnterPrepared) return true;
    mEnterPrepared = true;
    // Resolve this window's side of each pair by transitionName. Unresolved names simply
    // drop out of the flight (AOSP times out and falls back for the whole set when nothing
    // arrives; a partial set still animates here).
    for (const SharedElementSource& src : mSources) {
        View* target = findViewByTransitionName(mHost, src.name);
        if (!target || target->getWidth() <= 0 || target->getHeight() <= 0) continue;
        // Bounds in the HOST's local space (top-level windows rest at their frame).
        const int hl = mHost->getLeft(), ht = mHost->getTop();
        Rect from = Rect::MakeLTRB(src.l - hl, src.t - ht, src.l - hl + src.w, src.t - ht + src.h);
        int loc[2] = {0, 0};
        target->getLocationOnScreen(loc);
        Rect to = Rect::MakeLTRB(loc[0] - hl, loc[1] - ht,
                                 loc[0] - hl + target->getWidth(), loc[1] - ht + target->getHeight());
        // Hide the real view until landing (AOSP hides shared elements until the ghost
        // settles). Pre-draw, so the very first frame never shows it un-ghosted.
        target->setTransitionVisibility(View::INVISIBLE);
        SnapshotView* ghost = new SnapshotView(mHost->getContext(), src.snapshot);
        // View::layout(l, t, w, h) — pass extents, not right/bottom.
        ghost->layout(from.left, from.top, from.right() - from.left, from.bottom() - from.top);
        mHost->getOverlay()->add(ghost);
        mSceneFlights.push_back({ghost, from, to});
        mHiddenTargets.push_back(target);
    }
    // Each ghost holds its own RefPtr to its surface and the return flight re-captures
    // fresh pixels, so the source-side snapshots are never read again — release them now
    // instead of pinning full-size surfaces for the host window's lifetime.
    for (SharedElementSource& src : mSources) src.snapshot.reset();
    if (mSceneFlights.empty()) return false;
    // Suppress the window-level enter (AOSP: a scene transition replaces the app transition).
    // This runs pre-first-draw, so it also undoes the themed pre-snap (alpha 0 / offscreen)
    // before anything is composed; frame 1 shows background + ghosts at the start bounds.
    mHost->clearEnterTransition();
    // Content fade-in (the windowContentTransitions=fade look): the direct content children
    // fade 0 -> 1 at VIEW level — a window-surface fade would dim the ghosts, which live in
    // this same surface's overlay. The window's own background is not faded (AOSP fades it
    // via the window background transition; deviation noted).
    for (int i = 0; i < mHost->getChildCount(); i++) {
        View* child = mHost->getChildAt(i);
        mChildAlphas.push_back(child->getAlpha());
        child->setAlpha(0.f);
    }
    startFlightAnimator(true, std::function<void()>());
    return true;
}

bool ActivityTransitionCoordinator::startReturn(const std::function<void()>& onEnd) {
    if (mSources.empty()) return false;
    if (!mCaller || mCallerLiveness.expired()) return false;
    // Cancel a still-running enter flight first: its end callback frees the host-overlay
    // ghosts and reveals this window's targets, so the snapshots below capture full state.
    cancelAnimator();
    // Ghosts fly in the CALLER's coordinate space: from this window's shared-view bounds to
    // the caller's target bounds (live — the caller may have re-laid-out while covered).
    const int cl = mCaller->getLeft(), ct = mCaller->getTop();
    bool any = false;
    for (SharedElementSource& src : mSources) {
        if (!src.view || !src.view->isAttachedToWindow()) continue;
        View* mine = findViewByTransitionName(mHost, src.name);
        if (!mine || mine->getWidth() <= 0 || mine->getHeight() <= 0) continue;
        Cairo::RefPtr<Cairo::ImageSurface> snap = snapshotView(mine);
        if (!snap) continue;
        int myLoc[2] = {0, 0}, callerLoc[2] = {0, 0};
        mine->getLocationOnScreen(myLoc);
        src.view->getLocationOnScreen(callerLoc);
        Rect from = Rect::MakeLTRB(myLoc[0] - cl, myLoc[1] - ct,
                                   myLoc[0] - cl + mine->getWidth(), myLoc[1] - ct + mine->getHeight());
        Rect to = Rect::MakeLTRB(callerLoc[0] - cl, callerLoc[1] - ct,
                                 callerLoc[0] - cl + src.view->getWidth(),
                                 callerLoc[1] - ct + src.view->getHeight());
        SnapshotView* ghost = new SnapshotView(mCaller->getContext(), snap);
        ghost->layout(from.left, from.top, from.right() - from.left, from.bottom() - from.top);
        mCaller->getOverlay()->add(ghost);
        mSceneFlights.push_back({ghost, from, to});
        // Hide the caller's real target until landing (revealed in finishFlight).
        src.view->setTransitionVisibility(View::INVISIBLE);
        src.view->invalidate();
        any = true;
    }
    if (!any) return false;
    // AOSP stopSharedElementAnimation: once the shared elements transfer, the exiting decor
    // is hidden. The retirement runs before the animator starts (removeWindow purges this
    // window's handler queue — see finishClose) and the ghosts already hold captured pixels;
    // the window itself is freed by finishClose()'s posted delete at landing.
    mHost->retireFromCompositor();
    mFlightsInCaller = true;
    startFlightAnimator(false, onEnd);
    return true;
}

void ActivityTransitionCoordinator::cancelAnimator() {
    if (mAnimator == nullptr) return;
    Animator* prev = mAnimator;
    mAnimator = nullptr;   // clear before cancel(): the end callback must not see it
    prev->cancel();
    delete prev;
}

void ActivityTransitionCoordinator::startFlightAnimator(bool enter,
        const std::function<void()>& onEnd) {
    cancelAnimator();
    const std::weak_ptr<bool> callerGuard = mCallerLiveness;
    ValueAnimator* anim = ValueAnimator::ofFloat(std::vector<float>{0.f, 1.f});
    anim->setDuration(FLIGHT_DURATION_MS);
    anim->addUpdateListener([this, enter, callerGuard](ValueAnimator& a) {
        if (mTornDown) return;
        // Return flight: the ghosts live in the caller's tree. Check the liveness token
        // BEFORE touching them — weak_ptr expiry is observable without dereferencing, and
        // the single UI thread makes check-then-use atomic w.r.t. the caller's destruction.
        if (!enter && callerGuard.expired()) {
            a.cancel();
            return;
        }
        const float f = a.getAnimatedFraction();
        for (SceneFlight& fl : mSceneFlights) {
            const Rect r = lerpRect(fl.from, fl.to, f);
            // layout() -> setFrame() invalidates when the bounds move; an identical
            // frame (integer rounding) needs no repaint either. Extents, not right/bottom.
            fl.ghost->layout(r.left, r.top, r.right() - r.left, r.bottom() - r.top);
        }
        if (enter && !mChildAlphas.empty()) {
            const size_t n = std::min(mChildAlphas.size(), (size_t)mHost->getChildCount());
            for (size_t i = 0; i < n; i++) {
                const float base = mChildAlphas[i];
                mHost->getChildAt((int)i)->setAlpha(base + (1.f - base) * f);
            }
        }
    });
    Animator::AnimatorListener endListener;
    endListener.onAnimationEnd = [this, enter, onEnd](Animator&, bool) {
        if (mTornDown) return;  // the coordinator (or its host window) is being torn down
        // finishFlight guards its own cross-window touches; onEnd (the host's finishClose on
        // the return flight) must ALWAYS run — the host is already hidden (alpha 0) and may
        // not depend on the caller's survival to complete its teardown.
        finishFlight(enter);
        if (onEnd) onEnd();
        // Deliberately no animator delete here (delete-in-end-callback); it stays in mAnimator
        // and is freed by the dtor or the next startFlightAnimator.
    };
    anim->addListener(endListener);
    mAnimator = anim;
    anim->start();
}

void ActivityTransitionCoordinator::finishFlight(bool enter) {
    // Free the ghosts. SnapshotView is flight-owned: the overlay's removeView detaches but
    // does not delete (AOSP hands that to the GC).
    ViewGroupOverlay* overlay = nullptr;
    if (enter) {
        overlay = mHost->getOverlay();
    } else if (mCaller && !mCallerLiveness.expired()) {
        overlay = mCaller->getOverlay();
    }
    for (SceneFlight& fl : mSceneFlights) {
        if (overlay) {
            overlay->remove(fl.ghost);
            delete fl.ghost;
        } // else (return flight, caller gone): the caller's tree already freed the ghost
    }
    if (enter) {
        // Landing: reveal the real targets at their final bounds, land the fade at 1.
        for (View* t : mHiddenTargets) revealView(t);
        const size_t n = std::min(mChildAlphas.size(), (size_t)mHost->getChildCount());
        for (size_t i = 0; i < n; i++) mHost->getChildAt((int)i)->setAlpha(1.f);
    } else {
        for (SharedElementSource& src : mSources) revealView(src.view);
    }
    mSceneFlights.clear();
    mHiddenTargets.clear();
    mChildAlphas.clear();
    mFlightsInCaller = false;
}

} // namespace cdroid
