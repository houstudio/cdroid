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
// TransitionListener. C++: state held via shared_ptr (mTransitionPosition/mPausedX/Y are
// mutable and shared by the animator-side and transition-side EventSet lambdas wired in
// createAnimation). mTransitionPosition (int[2]) is owned by the view's tag, not by this state.
struct TransitionPositionState {
    View* mViewInHierarchy = nullptr;  // holds the transitionPosition tag
    View* mMovingView = nullptr;       // the view being translated (may be an overlay copy)
    int mStartX = 0;
    int mStartY = 0;
    int* mTransitionPosition = nullptr; // heap int[2] stored in the view's tag
    float mPausedX = 0;
    float mPausedY = 0;
    float mTerminalX = 0;
    float mTerminalY = 0;
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

    // android: new TransitionPositionListener(...). EventSet: one shared_ptr<State> captured by
    // the transition-side TransitionListener and the animator-side AnimatorListener/PauseListener.
    auto st = std::make_shared<TransitionPositionState>();
    st->mMovingView = view;
    st->mViewInHierarchy = values->view;
    st->mStartX = startPosX - (int)lround(view->getTranslationX());
    st->mStartY = startPosY - (int)lround(view->getTranslationY());
    st->mTerminalX = terminalX;
    st->mTerminalY = terminalY;
    st->mTransitionPosition = static_cast<int*>(values->view->getTag(R::id::transitionPosition));
    if (st->mTransitionPosition != nullptr) {
        values->view->setTag(R::id::transitionPosition, nullptr);
    }

    Transition::TransitionListener listener;
    listener.onTransitionEnd = [st](Transition&) {
        st->mMovingView->setTranslationX(st->mTerminalX);
        st->mMovingView->setTranslationY(st->mTerminalY);
    };
    transition->addListener(listener);

    Animator::AnimatorListener al;
    al.onAnimationCancel = [st](Animator&) {
        if (st->mTransitionPosition == nullptr) {
            st->mTransitionPosition = new int[2];
        }
        st->mTransitionPosition[0] = (int)lround(st->mStartX + st->mMovingView->getTranslationX());
        st->mTransitionPosition[1] = (int)lround(st->mStartY + st->mMovingView->getTranslationY());
        st->mViewInHierarchy->setTag(R::id::transitionPosition, st->mTransitionPosition);
    };
    al.onAnimationEnd = [st](Animator&, bool) {};
    Animator::AnimatorPauseListener apl;
    apl.onAnimationPause = [st](Animator&) {
        st->mPausedX = st->mMovingView->getTranslationX();
        st->mPausedY = st->mMovingView->getTranslationY();
        st->mMovingView->setTranslationX(st->mTerminalX);
        st->mMovingView->setTranslationY(st->mTerminalY);
    };
    apl.onAnimationResume = [st](Animator&) {
        st->mMovingView->setTranslationX(st->mPausedX);
        st->mMovingView->setTranslationY(st->mPausedY);
    };
    anim->addListener(al);
    anim->addPauseListener(apl);
    anim->setInterpolator(interpolator);
    return anim;
}

} // namespace cdroid
