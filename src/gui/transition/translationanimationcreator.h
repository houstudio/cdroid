/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.TranslationAnimationCreator.
 *********************************************************************************/
#ifndef __CDROID_TRANSITION_TRANSLATIONANIMATIONCREATOR_H__
#define __CDROID_TRANSITION_TRANSLATIONANIMATIONCREATOR_H__

#include <animation/animator.h>
#include <animation/interpolators.h>

#include <transition/transition.h>
#include <transition/transitionvalues.h>

namespace cdroid{

class View;

/**
 * Used by Slide and Explode to create an animator from start to end position, taking the
 * canceled (interrupted) position into account so the view does not blink or jump.
 * Ported from android-36 android.transition.TranslationAnimationCreator (package-private).
 */
class TranslationAnimationCreator{
public:
    static Animator* createAnimation(View* view, TransitionValues* values,
            int viewPosX, int viewPosY,
            float startX, float startY, float endX, float endY,
            const TimeInterpolator* interpolator, Transition* transition);
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_TRANSLATIONANIMATIONCREATOR_H__
