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

namespace cdroid{

namespace {

// android: anonymous TransitionListenerAdapter inside onDisappear that removes the
// overlay view on pause/end and re-adds it on resume. Named here (no anon classes).
class OverlayListener: public TransitionListenerAdapter{
public:
    ViewGroupOverlay* overlay;
    View* finalOverlayView;
    View* startView;

    void onTransitionPause(Transition& /*transition*/) override{
        overlay->remove(finalOverlayView);
    }
    void onTransitionResume(Transition& transition) override{
        if (finalOverlayView->getParent() == nullptr){
            overlay->add(finalOverlayView);
        } else {
            transition.cancel();
        }
    }
    void onTransitionEnd(Transition& transition) override{
        startView->setTag(R::id::transition_overlay_view_tag, nullptr);
        overlay->remove(finalOverlayView);
        transition.removeListener(this);
    }
};

} // anonymous namespace

const std::vector<std::string> Visibility::sTransitionProperties = {PROPNAME_VISIBILITY, PROPNAME_PARENT};

void Visibility::setSuppressLayout(bool suppress){
    mSuppressLayout = suppress;
}

void Visibility::setMode(int mode){
    if ((mode & ~(MODE_IN | MODE_OUT)) != 0){
        throw std::invalid_argument("Only MODE_IN and MODE_OUT flags are allowed");
    }
    mMode = mode;
}

std::vector<std::string> Visibility::getTransitionProperties(){
    return sTransitionProperties;
}

void Visibility::captureValues(TransitionValues& transitionValues){
    int visibility = transitionValues.view->getVisibility();
    transitionValues.values[PROPNAME_VISIBILITY] = visibility;
    transitionValues.values[PROPNAME_PARENT] = transitionValues.view->getParent(); // ViewGroup*
    int loc[2] = {0, 0};
    transitionValues.view->getLocationOnScreen(loc);
    transitionValues.values[PROPNAME_SCREEN_LOCATION] = std::vector<int>{loc[0], loc[1]};
}

void Visibility::captureStartValues(TransitionValues& transitionValues){
    captureValues(transitionValues);
}

void Visibility::captureEndValues(TransitionValues& transitionValues){
    captureValues(transitionValues);
}

bool Visibility::isVisible(TransitionValues* values){
    if (values == nullptr){
        return false;
    }
    int visibility = nonstd::any_cast<int>(values->values.at(PROPNAME_VISIBILITY));
    ViewGroup* parent = nonstd::any_cast<ViewGroup*>(values->values.at(PROPNAME_PARENT));
    return visibility == View::VISIBLE && parent != nullptr;
}

Visibility::VisibilityInfo Visibility::getVisibilityChangeInfo(TransitionValues* startValues,
        TransitionValues* endValues){
    VisibilityInfo visInfo;
    if (startValues != nullptr && startValues->values.count(PROPNAME_VISIBILITY)){
        visInfo.startVisibility = nonstd::any_cast<int>(startValues->values.at(PROPNAME_VISIBILITY));
        visInfo.startParent = nonstd::any_cast<ViewGroup*>(startValues->values.at(PROPNAME_PARENT));
    } else {
        visInfo.startVisibility = -1;
        visInfo.startParent = nullptr;
    }
    if (endValues != nullptr && endValues->values.count(PROPNAME_VISIBILITY)){
        visInfo.endVisibility = nonstd::any_cast<int>(endValues->values.at(PROPNAME_VISIBILITY));
        visInfo.endParent = nonstd::any_cast<ViewGroup*>(endValues->values.at(PROPNAME_PARENT));
    } else {
        visInfo.endVisibility = -1;
        visInfo.endParent = nullptr;
    }
    if (startValues != nullptr && endValues != nullptr){
        if (visInfo.startVisibility == visInfo.endVisibility &&
                visInfo.startParent == visInfo.endParent){
            return visInfo;
        } else {
            if (visInfo.startVisibility != visInfo.endVisibility){
                if (visInfo.startVisibility == View::VISIBLE){
                    visInfo.fadeIn = false;
                    visInfo.visibilityChange = true;
                } else if (visInfo.endVisibility == View::VISIBLE){
                    visInfo.fadeIn = true;
                    visInfo.visibilityChange = true;
                }
                // no visibilityChange if going between INVISIBLE and GONE
            } else if (visInfo.startParent != visInfo.endParent){
                if (visInfo.endParent == nullptr){
                    visInfo.fadeIn = false;
                    visInfo.visibilityChange = true;
                } else if (visInfo.startParent == nullptr){
                    visInfo.fadeIn = true;
                    visInfo.visibilityChange = true;
                }
            }
        }
    } else if (startValues == nullptr && visInfo.endVisibility == View::VISIBLE){
        visInfo.fadeIn = true;
        visInfo.visibilityChange = true;
    } else if (endValues == nullptr && visInfo.startVisibility == View::VISIBLE){
        visInfo.fadeIn = false;
        visInfo.visibilityChange = true;
    }
    return visInfo;
}

Animator* Visibility::createAnimator(ViewGroup* sceneRoot,
        TransitionValues* startValues, TransitionValues* endValues){
    VisibilityInfo visInfo = getVisibilityChangeInfo(startValues, endValues);
    if (visInfo.visibilityChange && (visInfo.startParent != nullptr || visInfo.endParent != nullptr)){
        if (visInfo.fadeIn){
            return onAppear(sceneRoot, startValues, visInfo.startVisibility, endValues, visInfo.endVisibility);
        } else {
            return onDisappear(sceneRoot, startValues, visInfo.startVisibility, endValues, visInfo.endVisibility);
        }
    }
    return nullptr;
}

Animator* Visibility::onAppear(ViewGroup* sceneRoot,
        TransitionValues* startValues, int /*startVisibility*/,
        TransitionValues* endValues, int /*endVisibility*/){
    if ((mMode & MODE_IN) != MODE_IN || endValues == nullptr){
        return nullptr;
    }
    if (startValues == nullptr){
        View* endParent = endValues->view->getParent(); // ViewGroup*
        TransitionValues* startParentValues = getMatchedTransitionValues(endParent, false);
        TransitionValues* endParentValues = getTransitionValues(endParent, false);
        VisibilityInfo parentVisibilityInfo = getVisibilityChangeInfo(startParentValues, endParentValues);
        if (parentVisibilityInfo.visibilityChange){
            return nullptr;
        }
    }
    return onAppear(sceneRoot, endValues->view, startValues, endValues);
}

Animator* Visibility::onAppear(ViewGroup* /*sceneRoot*/, View* /*view*/,
        TransitionValues* /*startValues*/, TransitionValues* /*endValues*/){
    return nullptr;
}

Animator* Visibility::onDisappear(ViewGroup* sceneRoot,
        TransitionValues* startValues, int /*startVisibility*/,
        TransitionValues* endValues, int endVisibility){
    if ((mMode & MODE_OUT) != MODE_OUT){
        return nullptr;
    }
    if (startValues == nullptr){
        // startValues(and startView) will never be null for disappear transition.
        return nullptr;
    }

    View* startView = startValues->view;
    View* endView = (endValues != nullptr) ? endValues->view : nullptr;
    View* overlayView = nullptr;
    View* viewToKeep = nullptr;
    bool reusingOverlayView = false;

    View* savedOverlayView = static_cast<View*>(startView->getTag(R::id::transition_overlay_view_tag));
    if (savedOverlayView != nullptr){
        // We've already created overlay for the start view — applying two visibility
        // transitions for the same view.
        overlayView = savedOverlayView;
        reusingOverlayView = true;
    } else {
        bool needOverlayForStartView = false;
        if (endView == nullptr || endView->getParent() == nullptr){
            if (endView != nullptr){
                // endView was removed from its parent - add it to the overlay
                overlayView = endView;
            } else {
                needOverlayForStartView = true;
            }
        } else {
            // visibility change
            if (endVisibility == View::INVISIBLE){
                viewToKeep = endView;
            } else {
                // Becoming GONE
                if (startView == endView){
                    viewToKeep = endView;
                } else {
                    needOverlayForStartView = true;
                }
            }
        }

        if (needOverlayForStartView){
            if (startView->getParent() == nullptr){
                overlayView = startView; // no parent - safe to use
            } else if (startView->getParent() != nullptr){
                View* startParent = startView->getParent(); // ViewGroup* (a View)
                TransitionValues* startParentValues = getTransitionValues(startParent, true);
                TransitionValues* endParentValues = getMatchedTransitionValues(startParent, true);
                VisibilityInfo parentVisibilityInfo = getVisibilityChangeInfo(startParentValues, endParentValues);
                if (!parentVisibilityInfo.visibilityChange){
                    overlayView = TransitionUtils::copyViewImage(sceneRoot, startView, static_cast<ViewGroup*>(startParent));
                } else {
                    int id = startParent->getId();
                    if (startParent->getParent() == nullptr && id != View::NO_ID
                            && sceneRoot->findViewById(id) != nullptr && mCanRemoveViews){
                        overlayView = startView;
                    } else {
                        // TODO: Handle this case as well
                    }
                }
            }
        }
    }

    if (overlayView != nullptr){
        ViewGroupOverlay* overlay = nullptr;
        if (!reusingOverlayView){
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
        if (!reusingOverlayView){
            if (animator == nullptr){
                overlay->remove(overlayView);
            } else {
                startView->setTag(R::id::transition_overlay_view_tag, overlayView);
                OverlayListener* l = new OverlayListener();
                l->overlay = overlay;
                l->finalOverlayView = overlayView;
                l->startView = startView;
                addListener(l); // Transition takes ownership
            }
        }
        return animator;
    }

    if (viewToKeep != nullptr){
        int originalVisibility = viewToKeep->getVisibility();
        viewToKeep->setTransitionVisibility(View::VISIBLE);
        Animator* animator = onDisappear(sceneRoot, viewToKeep, startValues, endValues);
        if (animator != nullptr){
            DisappearListener* disappearListener = new DisappearListener(viewToKeep, endVisibility, mSuppressLayout);
            // Wire the animator side via callback members (DisappearListener is one object
            // in android; CDROID's Animator listeners are EventSet values with callbacks).
            Animator::AnimatorListener al;
            al.onAnimationCancel = [disappearListener](Animator& a){ disappearListener->onAnimationCancel(a); };
            al.onAnimationEnd    = [disappearListener](Animator& a, bool){ disappearListener->onAnimationEnd(a); };
            Animator::AnimatorPauseListener apl;
            apl.onAnimationPause  = [disappearListener](Animator& a){ disappearListener->onAnimationPause(a); };
            apl.onAnimationResume = [disappearListener](Animator& a){ disappearListener->onAnimationResume(a); };
            animator->addListener(al);
            animator->addPauseListener(apl);
            addListener(disappearListener); // Transition takes ownership (transition side)
        } else {
            viewToKeep->setTransitionVisibility(originalVisibility);
        }
        return animator;
    }
    return nullptr;
}

Animator* Visibility::onDisappear(ViewGroup* /*sceneRoot*/, View* /*view*/,
        TransitionValues* /*startValues*/, TransitionValues* /*endValues*/){
    return nullptr;
}

bool Visibility::isTransitionRequired(TransitionValues* startValues, TransitionValues* newValues){
    if (startValues == nullptr && newValues == nullptr){
        return false;
    }
    if (startValues != nullptr && newValues != nullptr &&
            (newValues->values.count(PROPNAME_VISIBILITY) != 0) !=
                    (startValues->values.count(PROPNAME_VISIBILITY) != 0)){
        // The transition wasn't targeted in either the start or end, so it couldn't have changed.
        return false;
    }
    VisibilityInfo changeInfo = getVisibilityChangeInfo(startValues, newValues);
    return changeInfo.visibilityChange && (changeInfo.startVisibility == View::VISIBLE ||
            changeInfo.endVisibility == View::VISIBLE);
}

// ---- DisappearListener ----
Visibility::DisappearListener::DisappearListener(View* view, int finalVisibility, bool suppress)
    : mView(view), mFinalVisibility(finalVisibility),
      mParent(view ? view->getParent() : nullptr), // ViewGroup*
      mSuppressLayout(suppress){
    // Prevent a layout from including mView in its calculation.
    suppressLayout(true);
}

void Visibility::DisappearListener::onAnimationPause(Animator& /*animation*/){
    if (!mCanceled){
        mView->setTransitionVisibility(mFinalVisibility);
    }
}

void Visibility::DisappearListener::onAnimationResume(Animator& /*animation*/){
    if (!mCanceled){
        mView->setTransitionVisibility(View::VISIBLE);
    }
}

void Visibility::DisappearListener::onTransitionEnd(Transition& transition){
    hideViewWhenNotCanceled();
    transition.removeListener(this);
}

void Visibility::DisappearListener::hideViewWhenNotCanceled(){
    if (!mCanceled){
        // Recreate the parent's display list in case it includes mView.
        mView->setTransitionVisibility(mFinalVisibility);
        if (mParent != nullptr){
            mParent->invalidate();
        }
    }
    // Layout is allowed now that the View is in its final state
    suppressLayout(false);
}

void Visibility::DisappearListener::suppressLayout(bool suppress){
    if (mSuppressLayout && mLayoutSuppressed != suppress && mParent != nullptr){
        mLayoutSuppressed = suppress;
        mParent->suppressLayout(suppress);
    }
}

} // namespace cdroid
