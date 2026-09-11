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
// boundaries: theme window-animation/background dressing is PhoneWindow.generateLayout,
// the window-level FADE/SLIDE driver is the AppTransition/WindowState surface-animation role,
// and the shared-element stamp routes into ActivityTransitionCoordinator (route B).
// Definitions moved verbatim — no signature or behavior change (the loadThemeWindowBackground
// decode and the slide-delta read below were simplified in place: Theme obtainStyledAttributes
// and the now-public TranslateAnimation::resolve*).
#include <widget/cdwindow.h>
#include <widget/activitytransitioncoordinator.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
#include <content/typedvalue.h>
#include <core/windowmanager.h>
#include <animation/animator.h>
#include <animation/objectanimator.h>
#include <animation/valueanimator.h>
#include <animation/animationutils.h>
#include <animation/animationset.h>
#include <animation/alphaanimation.h>
#include <animation/translateanimation.h>
#include <view/gravity.h>

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
// a FADE. Returns nullptr when the animation expresses nothing mappable.
static ActivityTransition* transitionFromAnimation(Animation* anim, bool enter) {
    if (anim == nullptr) return nullptr;
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
    bool fades = false;
    for (Animation* a : parts) {
        if (a == nullptr) continue;
        duration = std::max<int64_t>(duration, a->getDuration());
        if (slide == nullptr) slide = dynamic_cast<TranslateAnimation*>(a);
        if (dynamic_cast<AlphaAnimation*>(a) != nullptr) fades = true;
    }
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
        return ActivityTransition::slide(edge, duration > 0 ? duration : 300);
    }
    if (fades) {
        // A bare whole-surface fade is nearly imperceptible at the resource's own
        // 150-220ms — the scale component it normally pairs with (the visible part of
        // grow_fade_in) is not expressible window-level, so hold the fade long enough
        // to read (window scaling is unsupported).
        constexpr int64_t MIN_FADE_DURATION_MS = 350;
        return ActivityTransition::fade(std::max<int64_t>(duration, MIN_FADE_DURATION_MS));
    }
    return nullptr;
}

void Window::setWindowAnimations(int resId, bool enableExit) {
    mWindowAnimationStyle = resId;
    mWindowExitAnimationsEnabled = enableExit;
    if (resId != 0) applyWindowAnimationStyle(resId); // resolve now (AOSP: params.windowAnimations)
}

void Window::applyWindowAnimationStyle(int styleRes) {
    if (styleRes == 0 || mContext == nullptr) return;
    // AOSP R.styleable.WindowAnimation: the plain window names, falling back to the Activity
    // open/close names Animation.Activity carries (the windowAnimationStyle target). The
    // generated styleable array carries its terminating 0 sentinel (gen_styleable.py appends
    // one), so the hand-maintained attr list and sentinel are gone.
    auto ta = mContext->getTheme().obtainStyledAttributes(styleRes, R::styleable::WindowAnimation);
    if (!ta) return;
    const int enterRes = ta->getResourceId(R::styleable::WindowAnimation_windowEnterAnimation,
                          ta->getResourceId(R::styleable::WindowAnimation_activityOpenEnterAnimation, 0));
    const int exitRes  = ta->getResourceId(R::styleable::WindowAnimation_windowExitAnimation,
                          ta->getResourceId(R::styleable::WindowAnimation_activityCloseExitAnimation, 0));

    // Install like setEnterTransition would — the snap is visual-only, so re-installing on a
    // window whose snap already ran just re-snaps the offset (getLeft()/getTop() never corrupted).
    auto enterT = enterRes != 0
        ? transitionFromAnimation(AnimationUtils::loadAnimation(mContext, enterRes), true) : nullptr;
    if (enterT != nullptr) {
        delete mEnterTransition;
        mEnterTransition = enterT;
        mPendingEnterAnim = true;
        snapEnterStart(enterT);
    }
    auto exitT = (exitRes != 0 && mWindowExitAnimationsEnabled)
        ? transitionFromAnimation(AnimationUtils::loadAnimation(mContext, exitRes), false) : nullptr;
    if (exitT != nullptr) {
        delete mExitTransition;
        mExitTransition = exitT;
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

// AOSP PhoneWindow.generateLayout, getContainer()==null branch:
//     if (mBackgroundDrawable == null && a.hasValue(R.styleable.Window_windowBackground))
//         mBackgroundDrawable = a.getDrawable(R.styleable.Window_windowBackground);
//     if (a.hasValue(R.styleable.Window_windowBackgroundFallback))
//         mBackgroundFallbackDrawable = a.getDrawable(R.styleable.Window_windowBackgroundFallback);
//     ...
//     mDecor.setWindowBackground(mBackgroundDrawable);          // -> DecorView.setBackground
//     if (mDecor.getBackground() == null && mBackgroundFallbackDrawable != null)
//         mDecor.setBackgroundFallback(mBackgroundFallbackDrawable);
// The Window IS the fused decor, so the resolved background goes straight to
// setBackground (View ownership; PhoneWindow's mBackgroundDrawable alias is dropped —
// a later app setBackground would free what it points at). The hand-built attr array
// stands in for the generated Window styleable (the trailing 0 is the sentinel
// obtainStyledAttributes scans to); TypedArray::getDrawable is the full AOSP decode
// (references, inline colors, file paths).
void Window::loadThemeWindowBackground() {
    if (mContext == nullptr) return;
    static const uint32_t attrs[] = {R::attr::windowBackground, R::attr::windowBackgroundFallback, 0};
    auto ta = mContext->getTheme().obtainStyledAttributes(attrs);
    if (!ta) return;
    if (Drawable* background = ta->getDrawable(0)) {  // null = unset or unresolvable
        setBackground(background);  // DecorView.setWindowBackground -> setBackground
        return;  // the fallback only applies when no window background is set
    }
    setBackgroundFallback(ta->getDrawable(1));
}

// AOSP DecorView.setBackgroundFallback (its BackgroundFallback member folded into
// the fused Window; the drawable is owned here — Java's GC becomes a delete).
void Window::setBackgroundFallback(Drawable* fallbackDrawable) {
    if (mBackgroundFallbackDrawable != fallbackDrawable) {
        delete mBackgroundFallbackDrawable;
        mBackgroundFallbackDrawable = fallbackDrawable;
    }
    setWillNotDraw(getBackground() == nullptr && mBackgroundFallbackDrawable == nullptr);
}

// AOSP DecorView.onDraw: super, then the background fallback.
void Window::onDraw(Canvas& canvas) {
    FrameLayout::onDraw(canvas);
    drawBackgroundFallback(canvas);
}

// AOSP com.android.internal.widget.BackgroundFallback.draw(boundsView, root, c, content,
// coveringView1, coveringView2) with boundsView/root == this Window and null covering
// views: track the union of the opaque visible children and fill the uncovered strips
// with the fallback drawable.
void Window::drawBackgroundFallback(Canvas& canvas) {
    if (mBackgroundFallbackDrawable == nullptr) return;  // !hasFallback()

    // Draw the fallback in the padding.
    const int width = getWidth();
    const int height = getHeight();

    int left = width;
    int top = height;
    int right = 0;
    int bottom = 0;

    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        Drawable* childBg = child->getBackground();
        // Potentially translucent or invisible children don't count, and we assume the
        // content view will cover the whole area if we're in a background fallback
        // situation.
        if (child->getVisibility() != View::VISIBLE
                || childBg == nullptr || childBg->getOpacity() != PixelFormat::OPAQUE) {
            continue;
        }
        left = std::min(left, child->getLeft());
        top = std::min(top, child->getTop());
        right = std::max(right, child->getRight());
        bottom = std::max(bottom, child->getBottom());
    }

    if (left >= right || top >= bottom) {
        // No valid area to draw in.
        return;
    }

    // CDROID Drawable::setBounds takes (x, y, w, h) — AOSP's (l, t, r, b) strips below.
    if (top > 0) {
        mBackgroundFallbackDrawable->setBounds(0, 0, width, top);
        mBackgroundFallbackDrawable->draw(canvas);
    }
    if (left > 0) {
        mBackgroundFallbackDrawable->setBounds(0, top, left, height - top);
        mBackgroundFallbackDrawable->draw(canvas);
    }
    if (right < width) {
        mBackgroundFallbackDrawable->setBounds(right, top, width - right, height - top);
        mBackgroundFallbackDrawable->draw(canvas);
    }
    if (bottom < height) {
        mBackgroundFallbackDrawable->setBounds(left, bottom, right - left, height - bottom);
        mBackgroundFallbackDrawable->draw(canvas);
    }
}

void Window::startEnterAnimation() {
    runActivityTransition(mEnterTransition, true, std::function<void()>());
}

void Window::startExitAnimation(const std::function<void()>& onEnd) {
    ActivityTransition* t = mReturnTransition ? mReturnTransition : mExitTransition;
    runActivityTransition(t, false, onEnd);
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

void Window::snapEnterStart(ActivityTransition* t) {
    if (!t || !isAttachedToWindow()) return;
    if (t->getType() == ActivityTransition::Type::FADE) {
        setAlpha(0.f);
    } else if (t->getType() == ActivityTransition::Type::SLIDE) {
        int x, y;
        computeSlidePos(t->getSlideEdge(), getLeft(), getTop(), getWidth(), getHeight(), true, x, y);
        setSurfaceTranslation(x - getLeft(), y - getTop());
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
        // ObjectAnimator "alpha" dispatches to View::setAlpha, which Window overrides to
        // GFXSurfaceSetOpacity (whole-surface opacity). Each frame must re-compose, so schedule
        // a traversal (setAlpha itself does not invalidate).
        ObjectAnimator* anim = ObjectAnimator::ofFloat(this, "alpha",
            std::vector<float>{enter ? 0.f : 1.f, enter ? 1.f : 0.f});
        anim->setDuration(duration);
        anim->addUpdateListener([this](ValueAnimator&) { scheduleTraversals(); });
        anim->addListener(endListener);
        mCurrentTransitionAnimator = anim;
        anim->start();
    } else { // SLIDE — animate the compose-time visual translation only. The real frame stays
             // at the resting position (getLeft()/getTop() are ALWAYS the rest — no capture),
             // so a11y bounds, input routing and WMS placement are stable mid-animation.
        int offX, offY;
        computeSlidePos(t->getSlideEdge(), getLeft(), getTop(), getWidth(), getHeight(), true, offX, offY);
        const int startX = enter ? offX - getLeft() : 0;
        const int startY = enter ? offY - getTop() : 0;
        const int endX   = enter ? 0 : offX - getLeft();
        const int endY   = enter ? 0 : offY - getTop();
        ValueAnimator* anim = ValueAnimator::ofFloat(std::vector<float>{0.f, 1.f});
        anim->setDuration(duration);
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
