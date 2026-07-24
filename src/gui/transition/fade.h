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
#ifndef __CDROID_TRANSITION_FADE_H__
#define __CDROID_TRANSITION_FADE_H__

#include <string>

#include <transition/visibility.h>

namespace cdroid{

class Context;
class AttributeSet;
class Animator;

/**
 * This transition tracks changes to the visibility of target views and fades views in
 * or out when they become visible or non-visible. Ported from android-36 android.transition.Fade.
 */
class Fade: public Visibility{
public:
    static constexpr int IN  = Visibility::MODE_IN;
    static constexpr int OUT = Visibility::MODE_OUT;

    Fade() = default;
    explicit Fade(int fadingMode){ setMode(fadingMode); }
    Fade(Context* context, AttributeSet* attrs);

    void captureStartValues(TransitionValues& transitionValues) override;
    Animator* onAppear(ViewGroup* sceneRoot, View* view,
            TransitionValues* startValues, TransitionValues* endValues) override;
    Animator* onDisappear(ViewGroup* sceneRoot, View* view,
            TransitionValues* startValues, TransitionValues* endValues) override;

    Fade* clone() const override{ Fade* c = new Fade(*this); copyCloneFields(c); return c; }

private:
    Animator* createAnimation(View* view, float startAlpha, float endAlpha);
    static float getStartAlpha(TransitionValues* startValues, float fallbackValue);

    static constexpr const char* PROPNAME_TRANSITION_ALPHA = "android:fade:transitionAlpha";

    /**
     * Promotes the view to a hardware layer for the duration of the fade (when the view
     * has overlapping rendering) and resets transitionAlpha afterwards.
     * (android: private static nested class extending AnimatorListenerAdapter. In CDROID
     * the Animator side is callback-member driven, so this is a plain nested struct whose
     * methods are invoked from Animator::AnimatorListener lambdas.)
     */
    struct FadeAnimatorListener{
        View* mView;
        bool mLayerTypeChanged = false;
        explicit FadeAnimatorListener(View* v): mView(v){}
        void onAnimationStart(Animator& animation);
        void onAnimationEnd(Animator& animation);
    };
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_FADE_H__
