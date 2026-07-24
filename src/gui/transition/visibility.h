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
#ifndef __CDROID_TRANSITION_VISIBILITY_H__
#define __CDROID_TRANSITION_VISIBILITY_H__

#include <string>
#include <vector>

#include <transition/transition.h>
#include <transition/transitionlisteneradapter.h>

namespace cdroid{

class Context;
class AttributeSet;
class View;
class ViewGroup;
class Animator;

/**
 * This transition tracks changes to the visibility of target views in the start and
 * end scenes. Ported from android-36 android.transition.Visibility. Intended as a base
 * for subclasses (Fade, Slide, Explode) which override onAppear/onDisappear.
 */
class Visibility: public Transition{
public:
    static constexpr int MODE_IN  = 0x1;
    static constexpr int MODE_OUT = 0x2;

    Visibility() = default;
    Visibility(Context* context, AttributeSet* attrs): Transition(context, attrs){}

    void setSuppressLayout(bool suppress);
    void setMode(int mode);
    int getMode() const{ return mMode; }
    std::vector<std::string> getTransitionProperties() override;
    void captureStartValues(TransitionValues& transitionValues) override;
    void captureEndValues(TransitionValues& transitionValues) override;
    bool isVisible(TransitionValues* values);

    Animator* createAnimator(ViewGroup* sceneRoot,
            TransitionValues* startValues, TransitionValues* endValues) override;

    virtual Animator* onAppear(ViewGroup* sceneRoot,
            TransitionValues* startValues, int startVisibility,
            TransitionValues* endValues, int endVisibility);
    virtual Animator* onAppear(ViewGroup* sceneRoot, View* view,
            TransitionValues* startValues, TransitionValues* endValues);
    virtual Animator* onDisappear(ViewGroup* sceneRoot,
            TransitionValues* startValues, int startVisibility,
            TransitionValues* endValues, int endVisibility);
    virtual Animator* onDisappear(ViewGroup* sceneRoot, View* view,
            TransitionValues* startValues, TransitionValues* endValues);

    bool isTransitionRequired(TransitionValues* startValues, TransitionValues* newValues) override;

    /**
     * Information about the visibility change between start and end values.
     * (android: private static nested class — kept nested.)
     */
    struct VisibilityInfo{
        bool visibilityChange = false;
        bool fadeIn = false;
        int startVisibility = -1;
        int endVisibility = -1;
        ViewGroup* startParent = nullptr;
        ViewGroup* endParent = nullptr;
    };

    /**
     * Listener that restores a view's final visibility after a disappear animation and
     * suppresses layout during it. In android it implements AnimatorListener +
     * AnimatorPauseListener + TransitionListener (one object); in CDROID the animator
     * side is wired via Animator::AnimatorListener/AnimatorPauseListener callback members
     * (lambdas capturing this listener), while the transition side uses this Transition-
     * ListenerAdapter base (owned by the Transition). (android: private static nested.)
     */
    class DisappearListener: public TransitionListenerAdapter{
    public:
        DisappearListener(View* view, int finalVisibility, bool suppressLayout);

        // Animator-side hooks (invoked from animator listener lambdas wired in onDisappear)
        void onAnimationCancel(Animator& /*animation*/){ mCanceled = true; }
        void onAnimationEnd(Animator& /*animation*/){ hideViewWhenNotCanceled(); }
        void onAnimationPause(Animator& /*animation*/);
        void onAnimationResume(Animator& /*animation*/);

        // Transition-side
        void onTransitionEnd(Transition& transition) override;
        void onTransitionPause(Transition& /*transition*/) override{ suppressLayout(false); }
        void onTransitionResume(Transition& /*transition*/) override{ suppressLayout(true); }

        void hideViewWhenNotCanceled();
        void suppressLayout(bool suppress);

        View* mView;
        int mFinalVisibility;
        ViewGroup* mParent;
        bool mSuppressLayout;
        bool mLayoutSuppressed = false;
        bool mCanceled = false;
    };

private:
    void captureValues(TransitionValues& transitionValues);
    static VisibilityInfo getVisibilityChangeInfo(TransitionValues* startValues, TransitionValues* endValues);

    static constexpr const char* PROPNAME_VISIBILITY     = "android:visibility:visibility";
    static constexpr const char* PROPNAME_PARENT         = "android:visibility:parent";
    static constexpr const char* PROPNAME_SCREEN_LOCATION = "android:visibility:screenLocation";

    static const std::vector<std::string> sTransitionProperties;
    int mMode = MODE_IN | MODE_OUT;
    bool mSuppressLayout = true;

    friend struct VisibilityInfo; // unused placeholder; nested type is self-contained
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_VISIBILITY_H__
