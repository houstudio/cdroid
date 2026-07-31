/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.Explode.
 *********************************************************************************/
#ifndef __CDROID_TRANSITION_EXPLODE_H__
#define __CDROID_TRANSITION_EXPLODE_H__

#include <transition/visibility.h>

namespace cdroid {

class Context;
class AttributeSet;

/**
 * Tracks visibility changes and moves views in/out from the edges of the scene, radiating
 * from the epicenter (or scene center). Ported from android-36 android.transition.Explode.
 */
class Explode: public Visibility {
  public:
    Explode();
    Explode(Context* context, AttributeSet* attrs);

    Animator* onAppear(ViewGroup* sceneRoot, View* view,
                       TransitionValues* startValues, TransitionValues* endValues) override;
    Animator* onDisappear(ViewGroup* sceneRoot, View* view,
                          TransitionValues* startValues, TransitionValues* endValues) override;

    // TransitionManager clones the transition before running it; without this override the clone
    // loses the Explode type and onAppear/onDisappear never run (same issue Slide/Fade had).
    Explode* clone() const override {
        Explode* c = new Explode(*this);
        copyCloneFields(c);
        return c;
    }

  protected:
    void captureStartValues(TransitionValues& transitionValues) override;
    void captureEndValues(TransitionValues& transitionValues) override;

  private:
    void captureValues(TransitionValues& transitionValues);
    void calculateOut(View* sceneRoot, Rect& bounds, int* outVector);
    static double calculateMaxDistance(View* sceneRoot, int focalX, int focalY);

    static constexpr const char* PROPNAME_SCREEN_BOUNDS = "android:explode:screenBounds";
    int mTempLoc[2] = {0, 0};
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_EXPLODE_H__
