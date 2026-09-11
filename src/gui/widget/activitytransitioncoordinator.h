/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — port of android.app.ActivityTransitionCoordinator (android-36), scoped to
 * the cross-Window shared-element flight (ActivityOptions.makeSceneTransitionAnimation).
 *********************************************************************************/
#ifndef __CDROID_ACTIVITYTRANSITIONCOORDINATOR_H__
#define __CDROID_ACTIVITYTRANSITIONCOORDINATOR_H__

#include <string>
#include <utility>
#include <vector>
#include <functional>
#include <memory>
#include <cairomm/surface.h>
#include <view/view.h>

namespace cdroid {

class Window;
class Animator;

// One instance per Window started with an ActivityOptions scene transition (Window::
// mSceneTransition, created by App::startActivity via Window::setSharedElementEnter).
// Owns the whole flight so the Window itself only hosts hook points:
//   - enter:  prepareEnter() from the first traversal (post-layout, pre-draw) resolves this
//     window's targets by transitionName, hides them, adds SNAPSHOT ghosts to the window's
//     own overlay and flies them caller-bounds -> final-bounds while the content children
//     fade in. The window-level enter animation (theme or programmatic) is suppressed —
//     AOSP: a scene transition replaces the app transition; prepareEnter() == false means
//     no pair resolved and the window-level enter stays untouched (the AOSP fallback).
//   - return: startReturn() from Window::close() flies ghosts (snapshots of this window's
//     shared views) in the CALLER's overlay back onto the caller's targets, hides this
//     window at once (AOSP stopSharedElementAnimation's decor-GONE) and runs onEnd
//     (finishClose) on landing. False = no valid pair, close() falls through.
//
// Ghosts render captured PIXELS (ImageSurface), the AOSP cross-window model — the live-view
// GhostView is for same-window transitions. After capture no pointer into the other window's
// tree is dereferenced per-frame; the residual cross-window touches (the caller targets'
// visibility on the return flight) are guarded by the caller's Window::mSceneLiveness weak
// token plus isAttachedToWindow(), check-then-use inside one message — atomic on the UI thread.
class ActivityTransitionCoordinator {
public:
    ActivityTransitionCoordinator(Window* host, Window* caller,
            const std::vector<std::pair<View*, std::string>>& sharedElements);
    ~ActivityTransitionCoordinator();

    // First-traversal hook (Window::doTraversal, after layout, before the first draw).
    bool prepareEnter();
    // Window::close() hook; onEnd runs when the return flight lands (or immediately when
    // nothing valid resolves and false is returned — close() then takes its normal path).
    bool startReturn(const std::function<void()>& onEnd);

private:
    // Caller-side shared element: borrowed view + the screen bounds and pixels captured at
    // startActivity (AOSP captures the exit scene before the new window covers it).
    struct SharedElementSource {
        View* view = nullptr;
        std::string name;
        int l = 0, t = 0, w = 0, h = 0;
        Cairo::RefPtr<Cairo::ImageSurface> snapshot;
    };
    // One ghost in flight, in the host's (enter) or the caller's (return) overlay.
    struct SceneFlight {
        View* ghost = nullptr;  // FIT_XY ImageView over the snapshot, owned by the flight
        Rect from;              // start bounds, host-of-the-ghost local coords
        Rect to;                // end bounds, same space
    };

    void startFlightAnimator(bool enter, const std::function<void()>& onEnd);
    void finishFlight(bool enter);
    // Cancel-and-free mAnimator outside its own end callback (delete-in-callback).
    void cancelAnimator();
    // Build + register one ghost flight (the shared tail of prepareEnter/startReturn).
    SceneFlight& addFlight(Window* host, const Cairo::RefPtr<Cairo::ImageSurface>& snapshot,
            const Rect& from, const Rect& to);

    Window* mHost = nullptr;    // the window being animated for (not owned)
    Window* mCaller = nullptr;  // the window that started mHost (not owned)
    std::weak_ptr<bool> mCallerLiveness;  // watches mCaller->mSceneLiveness
    std::vector<SharedElementSource> mSources;
    std::vector<SceneFlight> mSceneFlights;
    std::vector<View*> mHiddenTargets; // enter: targets hidden until landing (host tree)
    std::vector<float> mChildAlphas;   // enter: pre-fade alphas of the host's content children
    Animator* mAnimator = nullptr;     // owned flight animator (never deleted in its end callback)
    bool mFlightsInCaller = false;     // current mSceneFlights live in the caller's overlay
    bool mEnterPrepared = false;
    bool mTornDown = false;            // set in the dtor: animator end callbacks must not fire
};

} // namespace cdroid
#endif // __CDROID_ACTIVITYTRANSITIONCOORDINATOR_H__
