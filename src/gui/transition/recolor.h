/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.Recolor (@hide).
 *********************************************************************************/
#ifndef __CDROID_TRANSITION_RECOLOR_H__
#define __CDROID_TRANSITION_RECOLOR_H__

#include <transition/transition.h>

namespace cdroid{

class Context;
class AttributeSet;

/**
 * Tracks changes to the background color (when a ColorDrawable) and the text color of
 * TextViews and animates them. Ported from android-36 android.transition.Recolor (@hide).
 */
class Recolor: public Transition{
public:
    Recolor() = default;
    Recolor(Context* context, AttributeSet* attrs): Transition(context, attrs){}

    void captureStartValues(TransitionValues& transitionValues) override;
    void captureEndValues(TransitionValues& transitionValues) override;
    Animator* createAnimator(ViewGroup* sceneRoot,
            TransitionValues* startValues, TransitionValues* endValues) override;

    Recolor* clone() const override{ Recolor* c = new Recolor(*this); copyCloneFields(c); return c; }

private:
    void captureValues(TransitionValues& transitionValues);

    static constexpr const char* PROPNAME_BACKGROUND = "android:recolor:background";
    static constexpr const char* PROPNAME_TEXT_COLOR = "android:recolor:textColor";
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_RECOLOR_H__
