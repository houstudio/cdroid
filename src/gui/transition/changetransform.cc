/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.ChangeTransform.
 *********************************************************************************/
#include <transition/changetransform.h>

#include <animation/animator.h>
#include <animation/valueanimator.h>
#include <cairomm/matrix.h>
#include <core/any.h>
#include <porting/cdlog.h>
#include <view/ghostview.h>
#include <view/view.h>
#include <view/viewgroup.h>

#include <transition/transitionlisteneradapter.h>

namespace cdroid {

namespace {

bool matrixEquals(const Cairo::Matrix& a, const Cairo::Matrix& b) {
    return a.xx == b.xx && a.yx == b.yx && a.xy == b.xy && a.yy == b.yy && a.x0 == b.x0 && a.y0 == b.y0;
}

Cairo::Matrix lerpMatrix(const Cairo::Matrix& s, const Cairo::Matrix& e, float f) {
    Cairo::Matrix m;
    m.xx = s.xx + (e.xx - s.xx) * f;
    m.yx = s.yx + (e.yx - s.yx) * f;
    m.xy = s.xy + (e.xy - s.xy) * f;
    m.yy = s.yy + (e.yy - s.yy) * f;
    m.x0 = s.x0 + (e.x0 - s.x0) * f;
    m.y0 = s.y0 + (e.y0 - s.y0) * f;
    return m;
}

// android: GhostListener — removes the ghost on transition end, toggles visibility on
// pause/resume. Now wired inline in createAnimator as an EventSet TransitionListener value.

} // anonymous namespace

const std::vector<std::string> ChangeTransform::sTransitionProperties = {
    PROPNAME_MATRIX, PROPNAME_TRANSFORMS, PROPNAME_PARENT_MATRIX
};

// ---- Transforms ----
ChangeTransform::Transforms::Transforms(View* view) {
    translationX = view->getTranslationX();
    translationY = view->getTranslationY();
    translationZ = view->getTranslationZ();
    scaleX = view->getScaleX();
    scaleY = view->getScaleY();
    rotationX = view->getRotationX();
    rotationY = view->getRotationY();
    rotationZ = view->getRotation();
}

void ChangeTransform::Transforms::restore(View* view) const {
    setTransforms(view, translationX, translationY, translationZ, scaleX, scaleY, rotationX, rotationY, rotationZ);
}

void ChangeTransform::setTransforms(View* view, float tx, float ty, float tz,
                                    float sx, float sy, float rx, float ry, float rz) {
    view->setTranslationX(tx);
    view->setTranslationY(ty);
    view->setTranslationZ(tz);
    view->setScaleX(sx);
    view->setScaleY(sy);
    view->setRotationX(rx);
    view->setRotationY(ry);
    view->setRotation(rz);
}

void ChangeTransform::setIdentityTransforms(View* view) {
    setTransforms(view, 0, 0, 0, 1, 1, 0, 0, 0);
}

std::vector<std::string> ChangeTransform::getTransitionProperties() {
    return sTransitionProperties;
}

void ChangeTransform::captureValues(TransitionValues& transitionValues) {
    View* view = transitionValues.view;
    if (view->getVisibility() == View::GONE) {
        return;
    }
    transitionValues.values[PROPNAME_PARENT] = view->getParent(); // ViewGroup*
    transitionValues.values[PROPNAME_TRANSFORMS] = Transforms(view);
    transitionValues.values[PROPNAME_MATRIX] = view->getMatrix(); // Cairo::Matrix copy
    if (mReparent) {
        ViewGroup* parent = view->getParent();
        Cairo::Matrix parentMatrix;
        parent->transformMatrixToGlobal(parentMatrix);
        // android preTranslate(-scrollX,-scrollY); cairo translate is post-multiply — approximate.
        parentMatrix.translate(-parent->getScrollX(), -parent->getScrollY());
        transitionValues.values[PROPNAME_PARENT_MATRIX] = parentMatrix;
    }
}

void ChangeTransform::captureStartValues(TransitionValues& transitionValues) {
    captureValues(transitionValues);
}

void ChangeTransform::captureEndValues(TransitionValues& transitionValues) {
    captureValues(transitionValues);
}

bool ChangeTransform_parentsMatch(Transition* self, ViewGroup* startParent, ViewGroup* endParent) {
    if (!self->isValidTarget(startParent) || !self->isValidTarget(endParent)) {
        return startParent == endParent;
    }
    TransitionValues* endValues = self->getMatchedTransitionValues(startParent, true);
    if (endValues != nullptr) {
        return endParent == endValues->view;
    }
    return false;
}

Animator* ChangeTransform::createAnimator(ViewGroup* sceneRoot,
        TransitionValues* startValues, TransitionValues* endValues) {
    if (startValues == nullptr || endValues == nullptr
            || startValues->values.count(PROPNAME_PARENT) == 0
            || endValues->values.count(PROPNAME_PARENT) == 0) {
        return nullptr;
    }
    ViewGroup* startParent = nonstd::any_cast<ViewGroup*>(startValues->values.at(PROPNAME_PARENT));
    ViewGroup* endParent   = nonstd::any_cast<ViewGroup*>(endValues->values.at(PROPNAME_PARENT));
    bool handleParentChange = mReparent && !ChangeTransform_parentsMatch(this, startParent, endParent);

    // Simplified: skip the parent-matrix rebase + intermediate tags (setMatricesForParent)
    // and the GhostView overlay for now — animate the local matrix transform directly.
    // (The GhostView path can be enabled by calling createGhost below when handleParentChange
    // && mUseOverlay; left as a documented extension point.)

    Cairo::Matrix startMatrix = nonstd::any_cast<Cairo::Matrix>(startValues->values.at(PROPNAME_MATRIX));
    Cairo::Matrix endMatrix   = nonstd::any_cast<Cairo::Matrix>(endValues->values.at(PROPNAME_MATRIX));
    if (matrixEquals(startMatrix, endMatrix)) {
        return nullptr;
    }

    Transforms transforms = nonstd::any_cast<Transforms>(endValues->values.at(PROPNAME_TRANSFORMS));
    View* view = endValues->view;
    setIdentityTransforms(view);

    ValueAnimator* anim = ValueAnimator::ofFloat({0.0f, 1.0f});
    anim->addUpdateListener([view, startMatrix, endMatrix](ValueAnimator& a) {
        float f = a.getAnimatedFraction();
        Cairo::Matrix m = lerpMatrix(startMatrix, endMatrix, f);
        view->setAnimationMatrix(&m); // no-op on cairo (property transforms are the render path)
    });
    Animator::AnimatorListener listener;
    bool useOverlay = mUseOverlay;
    listener.onAnimationEnd = [view, transforms, useOverlay, sceneRoot, startValues, endValues, handleParentChange, this](Animator&, bool) {
        view->setAnimationMatrix(nullptr);
        transforms.restore(view);
        if (handleParentChange && useOverlay) {
            // GhostView overlay for parent change: add ghost, listen for end to remove.
            Cairo::Matrix endParentMatrix = nonstd::any_cast<Cairo::Matrix>(endValues->values.at(PROPNAME_PARENT_MATRIX));
            Cairo::Matrix localEnd = endParentMatrix;
            sceneRoot->transformMatrixToLocal(localEnd);
            GhostView* ghost = GhostView::addGhost(view, sceneRoot, &localEnd);
            if (ghost != nullptr) {
                View* startView = startValues->view;
                Transition::TransitionListener gl;
                gl.onTransitionEnd = [view, startView](Transition&) {
                    GhostView::removeGhost(view);
                    startView->setTransitionAlpha(1);
                };
                gl.onTransitionPause = [ghost](Transition&) {
                    ghost->setVisibility(View::INVISIBLE);
                };
                gl.onTransitionResume = [ghost](Transition&) {
                    ghost->setVisibility(View::VISIBLE);
                };
                this->addListener(gl);
                view->setTransitionAlpha(1);
            }
        }
    };
    anim->addListener(listener);
    return anim;
}

} // namespace cdroid
