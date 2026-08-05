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
#include <transition/visibility.h>

#include <animation/animator.h>
#include <core/any.h>
#include <porting/cdlog.h>
#include <stdexcept>
#include <view/view.h>
#include <view/viewgroup.h>
#include <view/viewgroupoverlay.h>
#include <widget/R.h>

#include <transition/transitionutils.h>

namespace cdroid {

// android: anonymous TransitionListener inside onDisappear that removes the overlay view on
// pause/end and re-adds it on resume. Now wired inline in createAnimator as an EventSet
// TransitionListener value.

// Helpers for the disappear listener: shared by the animator-side and transition-side
// EventSet lambdas (both hold a shared_ptr<Visibility::DisappearState>). Replace the old
// Visibility::DisappearListener methods.
static void disappearSuppressLayout(Visibility::DisappearState& s, bool suppress) {
    if (s.mSuppressLayout && s.mLayoutSuppressed != suppress && s.mParent != nullptr) {
        s.mLayoutSuppressed = suppress;
        s.mParent->suppressLayout(suppress);
    }
}
static void disappearHideWhenNotCanceled(Visibility::DisappearState& s) {
    if (!s.mCanceled) {
        // Recreate the parent's display list in case it includes mView.
        s.mView->setTransitionVisibility(s.mFinalVisibility);
        if (s.mParent != nullptr) {
            s.mParent->invalidate();
        }
    }
    // Layout is allowed now that the View is in its final state.
    disappearSuppressLayout(s, false);
}

const std::vector<std::string> Visibility::sTransitionProperties = {PROPNAME_VISIBILITY, PROPNAME_PARENT};

void Visibility::setSuppressLayout(bool suppress) {
    mSuppressLayout = suppress;
}

void Visibility::setMode(int mode) {
    if ((mode & ~(MODE_IN | MODE_OUT)) != 0) {
        throw std::invalid_argument("Only MODE_IN and MODE_OUT flags are allowed");
    }
    mMode = mode;
}

std::vector<std::string> Visibility::getTransitionProperties() {
    return sTransitionProperties;
}

void Visibility::captureValues(TransitionValues& transitionValues) {
    int visibility = transitionValues.view->getVisibility();
    transitionValues.values[PROPNAME_VISIBILITY] = visibility;
    transitionValues.values[PROPNAME_PARENT] = transitionValues.view->getParent(); // ViewGroup*
    int loc[2] = {0, 0};
    transitionValues.view->getLocationOnScreen(loc);
    transitionValues.values[PROPNAME_SCREEN_LOCATION] = std::vector<int> {loc[0], loc[1]};
}

void Visibility::captureStartValues(TransitionValues& transitionValues) {
    captureValues(transitionValues);
}

void Visibility::captureEndValues(TransitionValues& transitionValues) {
    captureValues(transitionValues);
}

bool Visibility::isVisible(TransitionValues* values) {
    if (values == nullptr) {
        return false;
    }
    int visibility = nonstd::any_cast<int>(values->values.at(PROPNAME_VISIBILITY));
    ViewGroup* parent = nonstd::any_cast<ViewGroup*>(values->values.at(PROPNAME_PARENT));
    return visibility == View::VISIBLE && parent != nullptr;
}

Visibility::VisibilityInfo Visibility::getVisibilityChangeInfo(TransitionValues* startValues,
        TransitionValues* endValues) {
    VisibilityInfo visInfo;
    if (startValues != nullptr && startValues->values.count(PROPNAME_VISIBILITY)) {
        visInfo.startVisibility = nonstd::any_cast<int>(startValues->values.at(PROPNAME_VISIBILITY));
        visInfo.startParent = nonstd::any_cast<ViewGroup*>(startValues->values.at(PROPNAME_PARENT));
    } else {
        visInfo.startVisibility = -1;
        visInfo.startParent = nullptr;
    }
    if (endValues != nullptr && endValues->values.count(PROPNAME_VISIBILITY)) {
        visInfo.endVisibility = nonstd::any_cast<int>(endValues->values.at(PROPNAME_VISIBILITY));
        visInfo.endParent = nonstd::any_cast<ViewGroup*>(endValues->values.at(PROPNAME_PARENT));
    } else {
        visInfo.endVisibility = -1;
        visInfo.endParent = nullptr;
    }
    if (startValues != nullptr && endValues != nullptr) {
        if (visInfo.startVisibility == visInfo.endVisibility &&
                visInfo.startParent == visInfo.endParent) {
            return visInfo;
        } else {
            if (visInfo.startVisibility != visInfo.endVisibility) {
                if (visInfo.startVisibility == View::VISIBLE) {
                    visInfo.fadeIn = false;
                    visInfo.visibilityChange = true;
                } else if (visInfo.endVisibility == View::VISIBLE) {
                    visInfo.fadeIn = true;
                    visInfo.visibilityChange = true;
                }
                // no visibilityChange if going between INVISIBLE and GONE
            } else if (visInfo.startParent != visInfo.endParent) {
                if (visInfo.endParent == nullptr) {
                    visInfo.fadeIn = false;
                    visInfo.visibilityChange = true;
                } else if (visInfo.startParent == nullptr) {
                    visInfo.fadeIn = true;
                    visInfo.visibilityChange = true;
                }
            }
        }
    } else if (startValues == nullptr && visInfo.endVisibility == View::VISIBLE) {
        visInfo.fadeIn = true;
        visInfo.visibilityChange = true;
    } else if (endValues == nullptr && visInfo.startVisibility == View::VISIBLE) {
        visInfo.fadeIn = false;
        visInfo.visibilityChange = true;
    }
    return visInfo;
}

Animator* Visibility::createAnimator(ViewGroup* sceneRoot,
                                     TransitionValues* startValues, TransitionValues* endValues) {
    VisibilityInfo visInfo = getVisibilityChangeInfo(startValues, endValues);
    if (visInfo.visibilityChange && (visInfo.startParent != nullptr || visInfo.endParent != nullptr)) {
        if (visInfo.fadeIn) {
            return onAppear(sceneRoot, startValues, visInfo.startVisibility, endValues, visInfo.endVisibility);
        } else {
            return onDisappear(sceneRoot, startValues, visInfo.startVisibility, endValues, visInfo.endVisibility);
        }
    }
    return nullptr;
}

Animator* Visibility::onAppear(ViewGroup* sceneRoot,
                               TransitionValues* startValues, int /*startVisibility*/,
                               TransitionValues* endValues, int /*endVisibility*/) {
    if ((mMode & MODE_IN) != MODE_IN || endValues == nullptr) {
        return nullptr;
    }
    if (startValues == nullptr) {
        View* endParent = endValues->view->getParent(); // ViewGroup*
        TransitionValues* startParentValues = getMatchedTransitionValues(endParent, false);
        TransitionValues* endParentValues = getTransitionValues(endParent, false);
        VisibilityInfo parentVisibilityInfo = getVisibilityChangeInfo(startParentValues, endParentValues);
        if (parentVisibilityInfo.visibilityChange) {
            return nullptr;
        }
    }
    return onAppear(sceneRoot, endValues->view, startValues, endValues);
}

Animator* Visibility::onAppear(ViewGroup* /*sceneRoot*/, View* /*view*/,
                               TransitionValues* /*startValues*/, TransitionValues* /*endValues*/) {
    return nullptr;
}

Animator* Visibility::onDisappear(ViewGroup* sceneRoot,
                                  TransitionValues* startValues, int /*startVisibility*/,
                                  TransitionValues* endValues, int endVisibility) {
    if ((mMode & MODE_OUT) != MODE_OUT) {
        return nullptr;
    }
    if (startValues == nullptr) {
        // startValues(and startView) will never be null for disappear transition.
        return nullptr;
    }

    View* startView = startValues->view;
    View* endView = (endValues != nullptr) ? endValues->view : nullptr;
    View* overlayView = nullptr;
    View* viewToKeep = nullptr;
    bool reusingOverlayView = false;

    View* savedOverlayView = static_cast<View*>(startView->getTag(R::id::transition_overlay_view_tag));
    if (savedOverlayView != nullptr) {
        // We've already created overlay for the start view — applying two visibility
        // transitions for the same view.
        overlayView = savedOverlayView;
        reusingOverlayView = true;
    } else {
        bool needOverlayForStartView = false;
        if (endView == nullptr || endView->getParent() == nullptr) {
            if (endView != nullptr) {
                // endView was removed from its parent - add it to the overlay
                overlayView = endView;
            } else {
                needOverlayForStartView = true;
            }
        } else {
            // visibility change
            if (endVisibility == View::INVISIBLE) {
                viewToKeep = endView;
            } else {
                // Becoming GONE
                if (startView == endView) {
                    viewToKeep = endView;
                } else {
                    needOverlayForStartView = true;
                }
            }
        }

        if (needOverlayForStartView) {
            if (startView->getParent() == nullptr) {
                overlayView = startView; // no parent - safe to use
            } else if (startView->getParent() != nullptr) {
                View* startParent = startView->getParent(); // ViewGroup* (a View)
                TransitionValues* startParentValues = getTransitionValues(startParent, true);
                TransitionValues* endParentValues = getMatchedTransitionValues(startParent, true);
                VisibilityInfo parentVisibilityInfo = getVisibilityChangeInfo(startParentValues, endParentValues);
                if (!parentVisibilityInfo.visibilityChange) {
                    overlayView = TransitionUtils::copyViewImage(sceneRoot, startView, static_cast<ViewGroup*>(startParent));
                } else {
                    int id = startParent->getId();
                    if (startParent->getParent() == nullptr && id != View::NO_ID
                            && sceneRoot->findViewById(id) != nullptr && mCanRemoveViews) {
                        overlayView = startView;
                    } else {
                        // TODO: Handle this case as well
                    }
                }
            }
        }
    }

    if (overlayView != nullptr) {
        ViewGroupOverlay* overlay = nullptr;
        if (!reusingOverlayView) {
            overlay = static_cast<ViewGroupOverlay*>(sceneRoot->getOverlay());
            std::vector<int>* screenLoc = nonstd::any_cast<std::vector<int>>(&startValues->values.at(PROPNAME_SCREEN_LOCATION));
            int screenX = (screenLoc && screenLoc->size() > 0) ? (*screenLoc)[0] : 0;
            int screenY = (screenLoc && screenLoc->size() > 1) ? (*screenLoc)[1] : 0;
            int loc[2] = {0, 0};
            sceneRoot->getLocationOnScreen(loc);
            overlayView->offsetLeftAndRight((screenX - loc[0]) - overlayView->getLeft());
            overlayView->offsetTopAndBottom((screenY - loc[1]) - overlayView->getTop());
            overlay->add(overlayView);
        }
        Animator* animator = onDisappear(sceneRoot, overlayView, startValues, endValues);
        if (!reusingOverlayView) {
            if (animator == nullptr) {
                overlay->remove(overlayView);
            } else {
                startView->setTag(R::id::transition_overlay_view_tag, overlayView);
                Transition::TransitionListener l;
                l.onTransitionPause = [overlay, overlayView](Transition&) {
                    overlay->remove(overlayView);
                };
                l.onTransitionResume = [overlay, overlayView](Transition& transition) {
                    if (overlayView->getParent() == nullptr) {
                        overlay->add(overlayView);
                    } else {
                        transition.cancel();
                    }
                };
                l.onTransitionEnd = [startView, overlay, overlayView](Transition&) {
                    startView->setTag(R::id::transition_overlay_view_tag, nullptr);
                    overlay->remove(overlayView);
                };
                addListener(l);
            }
        }
        return animator;
    }

    if (viewToKeep != nullptr) {
        int originalVisibility = viewToKeep->getVisibility();
        viewToKeep->setTransitionVisibility(View::VISIBLE);
        Animator* animator = onDisappear(sceneRoot, viewToKeep, startValues, endValues);
        if (animator != nullptr) {
            // android: new DisappearListener(...) shared by the animator side and the
            // transition side. EventSet: one shared_ptr<DisappearState> captured by all
            // lambdas (mCanceled/mLayoutSuppressed are mutated by both sides).
            auto st = std::make_shared<DisappearState>();
            st->mView = viewToKeep;
            st->mFinalVisibility = endVisibility;
            st->mParent = viewToKeep ? viewToKeep->getParent() : nullptr;
            st->mSuppressLayout = mSuppressLayout;
            st->mLayoutSuppressed = false;
            st->mCanceled = false;
            disappearSuppressLayout(*st, true); // android ctor: suppressLayout(true)
            Animator::AnimatorListener al;
            al.onAnimationCancel = [st](Animator&) { st->mCanceled = true; };
            al.onAnimationEnd    = [st](Animator&, bool) { disappearHideWhenNotCanceled(*st); };
            Animator::AnimatorPauseListener apl;
            apl.onAnimationPause  = [st](Animator&) {
                if (!st->mCanceled) st->mView->setTransitionVisibility(st->mFinalVisibility);
            };
            apl.onAnimationResume = [st](Animator&) {
                if (!st->mCanceled) st->mView->setTransitionVisibility(View::VISIBLE);
            };
            animator->addListener(al);
            animator->addPauseListener(apl);
            Transition::TransitionListener disappearListener;
            disappearListener.onTransitionEnd    = [st](Transition&) { disappearHideWhenNotCanceled(*st); };
            disappearListener.onTransitionPause  = [st](Transition&) { disappearSuppressLayout(*st, false); };
            disappearListener.onTransitionResume = [st](Transition&) { disappearSuppressLayout(*st, true); };
            addListener(disappearListener); // transition side
        } else {
            viewToKeep->setTransitionVisibility(originalVisibility);
        }
        return animator;
    }
    return nullptr;
}

Animator* Visibility::onDisappear(ViewGroup* /*sceneRoot*/, View* /*view*/,
                                  TransitionValues* /*startValues*/, TransitionValues* /*endValues*/) {
    return nullptr;
}

bool Visibility::isTransitionRequired(TransitionValues* startValues, TransitionValues* newValues) {
    if (startValues == nullptr && newValues == nullptr) {
        return false;
    }
    if (startValues != nullptr && newValues != nullptr &&
            (newValues->values.count(PROPNAME_VISIBILITY) != 0) !=
            (startValues->values.count(PROPNAME_VISIBILITY) != 0)) {
        // The transition wasn't targeted in either the start or end, so it couldn't have changed.
        return false;
    }
    VisibilityInfo changeInfo = getVisibilityChangeInfo(startValues, newValues);
    return changeInfo.visibilityChange && (changeInfo.startVisibility == View::VISIBLE ||
                                           changeInfo.endVisibility == View::VISIBLE);
}

// ---- DisappearListener ----
// (DisappearListener methods removed — logic moved to the disappearHideWhenNotCanceled /
// disappearSuppressLayout helpers above + the EventSet lambdas wired in onDisappear.)

} // namespace cdroid
