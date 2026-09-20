/*
 * Copyright (C) 2015 UI project
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
 */
// Window's activity/window-transition slice, split out of cdwindow.cc along the AOSP class
// boundaries: the window-level FADE/SLIDE driver is the AppTransition/WindowState
// surface-animation role, and the shared-element stamp routes into
// ActivityTransitionCoordinator (route B). The theme window-animation dressing
// (loadThemeWindowAnimations/applyWindowAnimationStyle) lives here with the driver it
// feeds; the DecorView background dressing (onDraw/fallback family) stayed in
// cdwindow.cc with the other window/decor concerns.
#include <widget/cdwindow.h>
#include <widget/activitytransitioncoordinator.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
#include <content/typedvalue.h>
#include <core/windowmanager.h>
#include <core/graphdevice.h>
#include <cairomm/surface.h>
#include <animation/animator.h>
#include <animation/objectanimator.h>
#include <animation/valueanimator.h>
#include <animation/animationutils.h>
#include <animation/interpolators.h>
#include <animation/animationset.h>
#include <animation/alphaanimation.h>
#include <animation/translateanimation.h>
#include <view/gravity.h>
#include <unordered_map>

namespace cdroid {
using namespace cdroid::internal;

// =====================================================================================
//  Activity transitions (Window-level: setAlpha for Fade, setPos for Slide)
//  CDROID's Window is the composition root, so only moving the Window itself (setPos) or setting
//  its surface opacity (setAlpha) produces a visible whole-window transition; a content view's
//  translationX cannot move the surface (composeSurfaces blits by getBound(), bypassing the View
//  transform). Mirrors android.app.Activity transition API names; the implementation is NOT
//  android.transition.Transition (content-level).
// =====================================================================================
void Window::setEnterTransition(ActivityTransition* t) {
    delete mEnterTransition;
    mEnterTransition = t;
    if (t && t->getType() != ActivityTransition::Type::NONE) {
        // The snap is a visual-only offset (snapEnterStart -> setSurfaceTranslation), so
        // getLeft()/getTop() stay the resting position — no capture dance needed.
        mPendingEnterAnim = true;
        snapEnterStart(t);  // pre-snap to the start state before the first frame (no full-show flash)
    } else {
        // setEnterTransition(nullptr) / NONE must fully UNDO the previous install
        // (AOSP: "no enter transition" shows the window at rest). Without this,
        // a themed snap (alpha 0 / offscreen translation) stays applied while
        // mPendingEnterAnim stays true — runActivityTransition's null/NONE
        // early-return restores neither, leaving the window invisible forever.
        mPendingEnterAnim = false;
        setSurfaceTranslation(0, 0);
        setAlpha(1.f);
    }
}

void Window::setExitTransition(ActivityTransition* t){
    delete mExitTransition;
    mExitTransition = t;
}

// Undo setEnterTransition/snapEnterStart's install — see cdwindow.h.
void Window::clearEnterTransition() {
    delete mEnterTransition;
    mEnterTransition = nullptr;
    mPendingEnterAnim = false;
    setSurfaceTranslation(0, 0);
    setAlpha(1.f);
}
void Window::setReturnTransition(ActivityTransition* t)  {
    delete mReturnTransition;
    mReturnTransition = t;
}
void Window::setReenterTransition(ActivityTransition* t) {
    delete mReenterTransition;
    mReenterTransition = t;
}

// =====================================================================================
//  Shared-element scene transitions (B route): the flight engine lives in
//  ActivityTransitionCoordinator (the android.app class that owns it in AOSP too — Activity
//  only holds an ActivityTransitionState). The Window hosts these thin hooks: this stamp
//  (App::startActivity, before the first traversal), the doTraversal prepare, close()'s
//  return consult and the dtor drop.
// =====================================================================================
void Window::setSharedElementEnter(Window* caller,
        const std::vector<std::pair<View*, std::string>>& sharedElements) {
    delete mSceneTransition;
    mSceneTransition = new ActivityTransitionCoordinator(this, caller, sharedElements);
    // Enter suppression happens in prepareEnter() (pre-first-draw): when a pair resolves it
    // undoes the themed pre-snap and drops mEnterTransition; when none does, the window-level
    // enter animation stays completely untouched (and the doTraversal hook drops the empty
    // coordinator — the single "will anything fly" decision lives in the coordinator's data).
}

// The shared-element return flight's hide-at-once — see cdwindow.h. A Window method (not a
// coordinator call into WindowManager) so the compositor-lifecycle reasoning stays with the
// window that owns its teardown.
void Window::retireFromCompositor() {
    WindowManager::getInstance().removeWindow(this);
}

// Map an AOSP window-animation resource onto the whole-Window ActivityTransition model.
// AOSP window animations are usually <set>s of alpha/translate/extend children; the
// whole-surface model can express one effect, so a translate child (the dominant motion)
// drives a SLIDE — edge from the offset delta's axis/sign — and an alpha-only set drives
// a FADE. Returns an empty spec when the animation expresses nothing mappable.
struct AnimSpec {
    ActivityTransition::Type type = ActivityTransition::Type::NONE;
    int slideEdge = 0;      // Gravity::LEFT/RIGHT/TOP/BOTTOM (SLIDE only)
    // The translate child's own deltas+units: AOSP plays the resource's real
    // motion (popup_enter_material rises 20dp), never a full-offscreen edge
    // fly-in. Empty (authored=false) when the resource names no translate.
    ActivityTransition::SlideDelta fromX, fromY, toX, toY;
    int64_t duration = 0;
    // The authored curve/timing. The interpolator instance is OWNED BY THE
    // PROCESS-WIDE STYLE CACHE (AnimSpec lives in sAnimStyleCache, which
    // outlives every window and every ActivityTransition borrowing it).
    TimeInterpolator* interpolator = nullptr;
    int64_t startOffset = 0;
};

// Deep-copy a loaded interpolator: the extracted Animation tree is freed right
// after extraction (AnimGuard), so a borrowed pointer would dangle. Every
// interpolator is a stateless value object with a usable copy constructor —
// the framework window-animation resources all resolve to this closed set.
static TimeInterpolator* cloneInterpolator(const TimeInterpolator* src) {
    if (src == nullptr) return nullptr;
    #define CLONE(T) if (auto* p = dynamic_cast<const T*>(src)) return new T(*p)
    CLONE(LinearInterpolator);
    CLONE(AccelerateInterpolator);
    CLONE(DecelerateInterpolator);
    CLONE(AnticipateInterpolator);
    CLONE(CycleInterpolator);
    CLONE(OvershootInterpolator);
    CLONE(AnticipateOvershootInterpolator);
    CLONE(BounceInterpolator);
    CLONE(AccelerateDecelerateInterpolator);
    CLONE(PathInterpolator);
    CLONE(FastOutSlowInInterpolator);
    CLONE(LinearOutSlowInInterpolator);
    CLONE(FastOutLinearInInterpolator);
    CLONE(BezierSCurveInterpolator);
    #undef CLONE
    return nullptr;  // unknown exotic subclass — animator default curve
}
static AnimSpec extractAnimSpec(Animation* anim, bool enter) {
    if (anim == nullptr) return AnimSpec();
    // OWNS anim: the caller loads a fresh Animation just for this parameter
    // extraction (AnimationUtils::loadAnimation), and ~AnimationSet frees the
    // child parts - free the whole tree on every exit or every window-style
    // apply (every popup show!) leaks it (valgrind: 2KB per record).
    struct AnimGuard {
        Animation* a;
        ~AnimGuard() { delete a; }
    } guard{anim};
    std::vector<Animation*> parts;
    if (auto* set = dynamic_cast<AnimationSet*>(anim)) {
        parts = set->getAnimations();
    } else {
        parts.push_back(anim);
    }
    int64_t duration = 0;
    TranslateAnimation* slide = nullptr;
    AlphaAnimation* fade = nullptr;
    for (Animation* a : parts) {
        if (a == nullptr) continue;
        duration = std::max<int64_t>(duration, a->getDuration());
        if (slide == nullptr) slide = dynamic_cast<TranslateAnimation*>(a);
        if (fade == nullptr && dynamic_cast<AlphaAnimation*>(a) != nullptr)
            fade = static_cast<AlphaAnimation*>(a);
    }
    // The DOMINANT child (the translate that drives a SLIDE, the alpha that
    // drives a FADE) also lends its interpolator and startOffset — a
    // shareInterpolator=false set (dialog_enter: scale@decelerate_quint +
    // alpha@decelerate_cubic) authors the curve per child, and the whole-window
    // model plays the dominant effect's curve. Prefer the child's own values,
    // falling back to the top-level set's (set on the <set> itself).
    const Animation* dominant = slide ? static_cast<const Animation*>(slide)
                                      : static_cast<const Animation*>(fade);
    AnimSpec spec;
    if (dominant != nullptr) {
        spec.interpolator = cloneInterpolator(dominant->getInterpolator());
        spec.startOffset = dominant->getStartOffset();
    }
    if (spec.interpolator == nullptr)
        spec.interpolator = cloneInterpolator(anim->getInterpolator());
    if (spec.startOffset == 0)
        spec.startOffset = anim->getStartOffset();
    if (slide != nullptr) {
        // The nonzero delta gives the motion axis+direction: an enter animation's from-delta is
        // the side the window comes FROM; an exit animation's to-delta is the side it leaves TO.
        // resolve* against a unit size: initialize() never ran on this extraction path, and
        // ABSOLUTE keeps its value while RELATIVE_* scales by 1 — the sign (the only thing the
        // edge computation uses) survives.
        const float dx = enter ? slide->resolveFromX(1, 1) : slide->resolveToX(1, 1);
        const float dy = enter ? slide->resolveFromY(1, 1) : slide->resolveToY(1, 1);
        int edge = Gravity::RIGHT; // computeSlidePos's default for a degenerate zero delta
        if      (dx < 0) edge = Gravity::LEFT;
        else if (dx == 0 && dy < 0) edge = Gravity::TOP;
        else if (dx == 0 && dy > 0) edge = Gravity::BOTTOM;
        spec.type = ActivityTransition::Type::SLIDE;
        spec.slideEdge = edge;
        spec.fromX = {slide->fromXType(), slide->fromXValue(), true};
        spec.fromY = {slide->fromYType(), slide->fromYValue(), true};
        spec.toX = {slide->toXType(), slide->toXValue(), true};
        spec.toY = {slide->toYType(), slide->toYValue(), true};
        spec.duration = duration > 0 ? duration : 300;
    } else if (fade != nullptr) {
        // A bare whole-surface fade is nearly imperceptible at the resource's own
        // 150-220ms — the scale component it normally pairs with (the visible part of
        // grow_fade_in) is not expressible window-level, so hold the fade long enough
        // to read (window scaling is unsupported).
        constexpr int64_t MIN_FADE_DURATION_MS = 350;
        spec.type = ActivityTransition::Type::FADE;
        spec.duration = std::max<int64_t>(duration, MIN_FADE_DURATION_MS);
    }
    return spec;
}

// Rebuild the owned value object from a spec (Window::setEnterTransition etc. take
// ownership; specs are cheap to re-apply, the style query + loadAnimation
// extraction they came from is not).
static ActivityTransition* transitionFromSpec(const AnimSpec& spec) {
    if (spec.type == ActivityTransition::Type::FADE)
        return ActivityTransition::fade(spec.duration, spec.interpolator, spec.startOffset);
    if (spec.type == ActivityTransition::Type::SLIDE)
        return ActivityTransition::slide(spec.slideEdge, spec.duration, spec.interpolator,
                spec.startOffset, spec.fromX, spec.fromY, spec.toX, spec.toY);
    return nullptr;
}

// Per-styleId resolution cache: applyWindowAnimationStyle runs on EVERY popup show
// (invokePopup -> setWindowAnimations), and a style's extracted content is a pure
// function of the resId — the theme only picks WHICH style id (that lookup stays
// per-window in loadThemeWindowAnimations). Cache the style query + the two
// loadAnimation extractions; rebuild the tiny value objects per window.
// Main-thread only (window show/relayout never runs on the input thread).
struct ResolvedAnimStyle {
    bool resolved = false;           // first-touch marker (a style may legitimately map to nothing)
    int enterRes = 0, exitRes = 0;   // 0 = the style names no animation for that leg
    AnimSpec enter, exit;
};
// Key: theme identity << 32 | styleRes — the style query resolves through THIS
// theme's resources, and in a multi-pak process two AssetManagers can mint the
// same numeric 0x7f style id with different content; keying by styleRes alone
// would cross-contaminate them. A dead theme's entries simply never hit again
// (pointer compare only — no dereference, a few hundred stale bytes at most).
static std::unordered_map<uint64_t, ResolvedAnimStyle> sAnimStyleCache;

void Window::setWindowAnimations(int resId, bool enableExit) {
    mWindowAnimationStyle = resId;
    mWindowExitAnimationsEnabled = enableExit;
    if (resId != 0) applyWindowAnimationStyle(resId); // resolve now (AOSP: params.windowAnimations)
}

void Window::applyWindowAnimationStyle(int styleRes) {
    if (styleRes == 0 || mContext == nullptr) return;
    // Resolve through the per-(theme,styleId) cache — every popup show passes
    // here with the same dropdown style, and only the FIRST visit pays the style
    // query + the two AnimationUtils::loadAnimation extraction trees. The theme
    // identity is its engine handle (the themed-cache key convention, resources.h).
    const uint64_t cacheKey = ((uint64_t)(uintptr_t)mContext->getTheme()._engineHandle() << 32)
                            ^ (uint32_t)styleRes;
    ResolvedAnimStyle& rs = sAnimStyleCache[cacheKey];
    if (!rs.resolved) {
        // AOSP R.styleable.WindowAnimation: the plain window names, falling back to the Activity
        // open/close names Animation.Activity carries (the windowAnimationStyle target). The
        // generated styleable array carries its terminating 0 sentinel (gen_styleable.py appends
        // one), so the hand-maintained attr list and sentinel are gone.
        auto ta = mContext->getTheme().obtainStyledAttributes(styleRes, R::styleable::WindowAnimation);
        if (!ta) return;
        rs.enterRes = ta->getResourceId(R::styleable::WindowAnimation_windowEnterAnimation,
                        ta->getResourceId(R::styleable::WindowAnimation_activityOpenEnterAnimation, 0));
        rs.exitRes  = ta->getResourceId(R::styleable::WindowAnimation_windowExitAnimation,
                        ta->getResourceId(R::styleable::WindowAnimation_activityCloseExitAnimation, 0));
        rs.enter = rs.enterRes != 0
            ? extractAnimSpec(AnimationUtils::loadAnimation(mContext, rs.enterRes), true) : AnimSpec();
        // The exit spec is cached unconditionally (style content): the
        // enableExit=false flavor of setWindowAnimations just skips APPLYING it.
        rs.exit = rs.exitRes != 0
            ? extractAnimSpec(AnimationUtils::loadAnimation(mContext, rs.exitRes), false) : AnimSpec();
        rs.resolved = true;
    }

    // Install like setEnterTransition would — but only ARM + pre-snap the enter
    // while the window has never drawn its first frame (mCanvas is created by the
    // first getCanvas). Re-resolution on an on-screen window (setTheme at runtime,
    // a later style pass) must not re-snap and replay the whole enter animation:
    // AOSP reads window dressing at decor INSTALL time only — a visible window
    // keeps its pixels; recreate() replays via a fresh window.
    auto enterT = transitionFromSpec(rs.enter);
    if (enterT != nullptr) {
        delete mEnterTransition;
        mEnterTransition = enterT;
        if (mAttachInfo == nullptr || mAttachInfo->mCanvas == nullptr) {
            mPendingEnterAnim = true;
            snapEnterStart(enterT);  // pre-snap to the start state before the first frame
        }
    } else {
        // The style names no enter animation: clear whatever a previous style
        // installed (incl. an armed snap) — setEnterTransition's null branch does
        // the full undo (armed flag + translation/alpha revert).
        setEnterTransition(nullptr);
    }
    // Symmetrically for the exit leg: a style with no windowExitAnimation (or a
    // swap onto one) must not keep the OLD style's exit armed (enableExit=false
    // deliberately keeps whatever is installed — the legacy sync-dismiss flavor).
    auto exitT = mWindowExitAnimationsEnabled ? transitionFromSpec(rs.exit) : nullptr;
    if (exitT != nullptr) {
        delete mExitTransition;
        mExitTransition = exitT;
    } else if (mWindowExitAnimationsEnabled) {
        setExitTransition(nullptr);
    }
}

void Window::loadThemeWindowAnimations() {
    if (mContext == nullptr) return;
    if (mWindowAnimationStyle != 0) { // explicit override (AOSP LayoutParams.windowAnimations)
        applyWindowAnimationStyle(mWindowAnimationStyle);
        return;
    }
    // AOSP PhoneWindow.generateLayout: the theme's windowAnimationStyle carries the window
    // animation style; a compiled @style item resolves as a reference whose data is the style id.
    TypedValue styleValue;
    if (!mContext->getTheme().resolveAttribute(R::attr::windowAnimationStyle, &styleValue, true)) return;
    const int styleRes = (styleValue.type == TypedValue::TYPE_REFERENCE)
            ? (int)styleValue.data : (int)styleValue.resourceId;
    if (styleRes != 0) applyWindowAnimationStyle(styleRes);
}

void Window::startEnterAnimation() {
    runActivityTransition(mEnterTransition, true, std::function<void()>());
}

GraphDevice::GhostLayer* Window::captureGhost() {
    // Snapshot the window's own surface — the same ImageSurface composeSurfaces
    // blits from. The compose-time translation is NOT baked into the pixels
    // (it is compose-side), so the ghost carries dx/dy separately, starting
    // from any in-flight enter offset to continue from what is on screen now.
    if (mAttachInfo == nullptr || mAttachInfo->mCanvas == nullptr) return nullptr;
    Cairo::RefPtr<Cairo::ImageSurface> snap = Cairo::ImageSurface::create(
            Cairo::Surface::Format::ARGB32, getWidth(), getHeight());
    Cairo::RefPtr<Cairo::Context> ctx = Cairo::Context::create(snap);
    ctx->set_source(mAttachInfo->mCanvas->get_target(), 0, 0);
    ctx->set_operator(Cairo::Context::Operator::SOURCE);
    ctx->paint();
    GraphDevice::GhostLayer* g = GraphDevice::getInstance().addGhost(snap, getBound());
    g->dx = mSurfaceDx;
    g->dy = mSurfaceDy;
    // Carry the in-flight enter fade too: dismiss during a FADE enter leaves the
    // window's alpha mid-flight (the canceled animator's end listener only resets
    // the surface translation), so the ghost must CONTINUE from the on-screen
    // opacity — its snapshot holds full-content pixels, a fresh alpha=1 ghost
    // would pop the frame bright before fading out.
    g->alpha = getAlpha();
    if (mSurfaceDx != 0 || mSurfaceDy != 0) {
        g->lastRect = getBound();
        g->lastRect.offset(mSurfaceDx, mSurfaceDy);
    }
    return g;
}

void Window::startGhostExit(ActivityTransition* t) {
    // AOSP WMS: the exit animation plays on the REMOVED window's surface
    // (WindowStateAnimator) while the view tree is already gone — the ghost IS
    // that surface here. The tree below tears down synchronously; this animator
    // touches compositor state only (no View, no this-capture, so the Window's
    // death mid-flight is a non-event).
    GraphDevice::GhostLayer* ghost = captureGhost();
    if (ghost == nullptr) return;
    const int64_t duration = t->getDuration();
    const TimeInterpolator* interpolator = t->getInterpolator();  // borrowed (style cache)
    const int64_t startDelay = t->getStartOffset();
    Animator::AnimatorListener endListener;
    endListener.onAnimationEnd = [ghost](Animator&, bool) {
        GraphDevice::getInstance().removeGhost(ghost);  // frees the animator too
    };
    if (t->getType() == ActivityTransition::Type::FADE) {
        // Start from the ghost's inherited alpha (mid-fade dismiss continuity).
        ValueAnimator* anim = ValueAnimator::ofFloat(
                std::vector<float>{ghost->alpha, 0.f});
        anim->setDuration(duration);
        anim->setStartDelay(startDelay);
        anim->setInterpolator(interpolator);
        anim->addUpdateListener([ghost](ValueAnimator& a) {
            ghost->alpha = 1.f - a.getAnimatedFraction();
            GraphDevice::getInstance().composeGhosts();
        });
        anim->addListener(endListener);
        ghost->animator = anim;
        anim->start();
    } else {  // SLIDE — translate the snapshot out toward the exit edge.
        int sX, sY, endX, endY;
        slideOffsets(t, false, ghost->bounds.left, ghost->bounds.top,
                ghost->bounds.width, ghost->bounds.height, sX, sY, endX, endY);
        const int startX = ghost->dx, startY = ghost->dy;
        ValueAnimator* anim = ValueAnimator::ofFloat(std::vector<float>{0.f, 1.f});
        anim->setDuration(duration);
        anim->setStartDelay(startDelay);
        anim->setInterpolator(interpolator);
        anim->addUpdateListener([ghost, startX, startY, endX, endY](ValueAnimator& a) {
            const float f = a.getAnimatedFraction();
            ghost->dx = (int)(startX + (endX - startX) * f);
            ghost->dy = (int)(startY + (endY - startY) * f);
            GraphDevice::getInstance().composeGhosts();
        });
        anim->addListener(endListener);
        ghost->animator = anim;
        anim->start();
    }
}

void Window::computeSlidePos(int edge, int ox, int oy, int w, int h, bool offscreen, int& x, int& y) {
    if (edge != Gravity::LEFT && edge != Gravity::RIGHT
        && edge != Gravity::TOP && edge != Gravity::BOTTOM) edge = Gravity::RIGHT;
    x = ox; y = oy;
    if (!offscreen) return;
    if (edge == Gravity::LEFT)        x = ox - w;
    else if (edge == Gravity::RIGHT)  x = ox + w;
    else if (edge == Gravity::TOP)    y = oy - h;
    else                              y = oy + h;  // BOTTOM
}

void Window::slideOffsets(const ActivityTransition* t, bool enter, int left, int top, int w, int h,
        int& startX, int& startY, int& endX, int& endY) {
    if (t->hasAuthoredDeltas()) {
        // AOSP plays the resource's own motion: the from-deltas are the enter
        // start, the to-deltas the exit end, the resting side 0. RELATIVE_TO_
        // SELF resolves against the live window size, PARENT against the
        // display (a window's parent).
        startX = t->fromX().resolve(w, GraphDevice::getInstance().getScreenWidth());
        startY = t->fromY().resolve(h, GraphDevice::getInstance().getScreenHeight());
        endX = t->toX().resolve(w, GraphDevice::getInstance().getScreenWidth());
        endY = t->toY().resolve(h, GraphDevice::getInstance().getScreenHeight());
        return;
    }
    int x, y;
    computeSlidePos(t->getSlideEdge(), left, top, w, h, true, x, y);
    startX = enter ? x - left : 0;
    startY = enter ? y - top : 0;
    endX = enter ? 0 : x - left;
    endY = enter ? 0 : y - top;
}

void Window::snapEnterStart(ActivityTransition* t) {
    if (!t || !isAttachedToWindow()) return;
    if (t->getType() == ActivityTransition::Type::FADE) {
        setAlpha(0.f);
    } else if (t->getType() == ActivityTransition::Type::SLIDE) {
        int startX, startY, endX, endY;
        slideOffsets(t, true, getLeft(), getTop(), getWidth(), getHeight(),
                startX, startY, endX, endY);
        setSurfaceTranslation(startX, startY);
    }
}

void Window::runActivityTransition(ActivityTransition* t, bool enter, const std::function<void()>& onEnd) {
    if (!t || t->getType() == ActivityTransition::Type::NONE || !isAttachedToWindow()) {
        mInTransition = false;
        if (onEnd) onEnd();
        return;
    }
    // Replace any in-flight transition animator. cancel() fires onAnimationEnd (animator.cc) — the
    // replaced animator is always an enter (onEnd empty), and ~Window's path is guarded by mDestroyed.
    if (mCurrentTransitionAnimator) {
        Animator* prev = mCurrentTransitionAnimator;
        mCurrentTransitionAnimator = nullptr;
        prev->cancel();
        delete prev;
    }
    mInTransition = true;
    const int64_t duration = t->getDuration();
    const TimeInterpolator* interpolator = t->getInterpolator();  // borrowed (style cache)
    const int64_t startDelay = t->getStartOffset();
    Animator::AnimatorListener endListener;
    endListener.onAnimationEnd = [this, onEnd, enter](Animator&, bool) {
        if (mDestroyed) return;  // ~Window is tearing us down — don't run finishClose / replace
        mInTransition = false;
        // Identity landing (AOSP onAnimationFinished commits the final surface transaction):
        // the surface must end exactly on the frame. Exit animations skip this — the window
        // is removed/hidden right after, and an interrupted exit's re-enter lands here anyway.
        if (enter) setSurfaceTranslation(0, 0);
        if (onEnd) onEnd();
        // The animator is NOT deleted here (delete-in-end-callback). It stays in
        // mCurrentTransitionAnimator and is freed by ~Window or the next runActivityTransition.
    };

    if (t->getType() == ActivityTransition::Type::FADE) {
        // ObjectAnimator "alpha" dispatches to Window::setAlpha — compositional
        // only (no-invalidation + self-flip), so each frame re-COMPOSES without
        // touching the view tree; no traversal is scheduled here.
        ObjectAnimator* anim = ObjectAnimator::ofFloat(this, "alpha",
            std::vector<float>{enter ? 0.f : 1.f, enter ? 1.f : 0.f});
        anim->setDuration(duration);
        anim->setStartDelay(startDelay);
        anim->setInterpolator(interpolator);
        anim->addListener(endListener);
        mCurrentTransitionAnimator = anim;
        anim->start();
    } else { // SLIDE — animate the compose-time visual translation only. The real frame stays
             // at the resting position (getLeft()/getTop() are ALWAYS the rest — no capture),
             // so a11y bounds, input routing and WMS placement are stable mid-animation.
        int startX, startY, endX, endY;
        slideOffsets(t, enter, getLeft(), getTop(), getWidth(), getHeight(),
                startX, startY, endX, endY);
        ValueAnimator* anim = ValueAnimator::ofFloat(std::vector<float>{0.f, 1.f});
        anim->setDuration(duration);
        anim->setStartDelay(startDelay);
        anim->setInterpolator(interpolator);
        anim->addUpdateListener([this, startX, startY, endX, endY](ValueAnimator& a) {
            const float f = a.getAnimatedFraction();
            setSurfaceTranslation((int)(startX + (endX - startX) * f),
                                  (int)(startY + (endY - startY) * f));
        });
        anim->addListener(endListener);
        mCurrentTransitionAnimator = anim;
        if (enter) setSurfaceTranslation(startX, startY);  // offscreen visual before the first frame
        anim->start();
    }
}

}  //endof namespace
