/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.TranslationAnimationCreator.
 *********************************************************************************/
#include <transition/translationanimationcreator.h>

#include <cmath>

#include <animation/objectanimator.h>
#include <core/path.h>
#include <view/view.h>
#include <widget/R.h>

#include <transition/transitionlisteneradapter.h>

namespace cdroid {

namespace {

// android: private static TransitionPositionListener extends AnimatorListenerAdapter implements
// TransitionListener. C++: a TransitionListenerAdapter subclass (transition side) whose
// animator-side methods (onAnimationCancel/Pause/Resume/End) are invoked from Animator
// listener lambdas wired in createAnimation.
class TransitionPositionListener: public TransitionListenerAdapter {
  public:
    View* mViewInHierarchy;  // holds the transitionPosition tag
    View* mMovingView;       // the view being translated (may be an overlay copy)
    int mStartX;
    int mStartY;
    int* mTransitionPosition = nullptr; // heap int[2] stored in the view's tag
    float mPausedX = 0;
    float mPausedY = 0;
    float mTerminalX;
    float mTerminalY;

    TransitionPositionListener(View* movingView, View* viewInHierarchy,
                               int startX, int startY, float terminalX, float terminalY) {
        mMovingView = movingView;
        mViewInHierarchy = viewInHierarchy;
        mStartX = startX - (int)lround(mMovingView->getTranslationX());
        mStartY = startY - (int)lround(mMovingView->getTranslationY());
        mTerminalX = terminalX;
        mTerminalY = terminalY;
        mTransitionPosition = static_cast<int*>(mViewInHierarchy->getTag(R::id::transitionPosition));
        if (mTransitionPosition != nullptr) {
            mViewInHierarchy->setTag(R::id::transitionPosition, nullptr);
        }
    }

    void onAnimationCancel() {
        if (mTransitionPosition == nullptr) {
            mTransitionPosition = new int[2];
        }
        mTransitionPosition[0] = (int)lround(mStartX + mMovingView->getTranslationX());
        mTransitionPosition[1] = (int)lround(mStartY + mMovingView->getTranslationY());
        mViewInHierarchy->setTag(R::id::transitionPosition, mTransitionPosition);
    }
    void onAnimationEnd() {}

    void onAnimationPause() {
        mPausedX = mMovingView->getTranslationX();
        mPausedY = mMovingView->getTranslationY();
        mMovingView->setTranslationX(mTerminalX);
        mMovingView->setTranslationY(mTerminalY);
    }
    void onAnimationResume() {
        mMovingView->setTranslationX(mPausedX);
        mMovingView->setTranslationY(mPausedY);
    }

    void onTransitionEnd(Transition& transition) override {
        mMovingView->setTranslationX(mTerminalX);
        mMovingView->setTranslationY(mTerminalY);
        transition.removeListener(this);
    }
};

} // anonymous namespace

Animator* TranslationAnimationCreator::createAnimation(View* view, TransitionValues* values,
        int viewPosX, int viewPosY, float startX, float startY, float endX, float endY,
        const TimeInterpolator* interpolator, Transition* transition) {
    float terminalX = view->getTranslationX();
    float terminalY = view->getTranslationY();
    int* startPosition = static_cast<int*>(values->view->getTag(R::id::transitionPosition));
    if (startPosition != nullptr) {
        startX = startPosition[0] - viewPosX + terminalX;
        startY = startPosition[1] - viewPosY + terminalY;
    }
    int startPosX = viewPosX + (int)lround(startX - terminalX);
    int startPosY = viewPosY + (int)lround(startY - terminalY);

    view->setTranslationX(startX);
    view->setTranslationY(startY);
    if (startX == endX && startY == endY) {
        return nullptr;
    }
    Path path;
    path.moveTo(startX, startY);
    path.lineTo(endX, endY);
    ObjectAnimator* anim = ObjectAnimator::ofFloat(view, View::TRANSLATION_X, View::TRANSLATION_Y, path);

    TransitionPositionListener* listener = new TransitionPositionListener(view, values->view,
            startPosX, startPosY, terminalX, terminalY);
    transition->addListener(listener);

    Animator::AnimatorListener al;
    al.onAnimationCancel = [listener](Animator&) {
        listener->onAnimationCancel();
    };
    al.onAnimationEnd    = [listener](Animator&, bool) {
        listener->onAnimationEnd();
    };
    Animator::AnimatorPauseListener apl;
    apl.onAnimationPause  = [listener](Animator&) {
        listener->onAnimationPause();
    };
    apl.onAnimationResume = [listener](Animator&) {
        listener->onAnimationResume();
    };
    anim->addListener(al);
    anim->addPauseListener(apl);
    anim->setInterpolator(interpolator);
    return anim;
}

} // namespace cdroid
