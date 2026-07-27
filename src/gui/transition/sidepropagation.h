/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.SidePropagation.
 *********************************************************************************/
#ifndef __CDROID_TRANSITION_SIDEPROPAGATION_H__
#define __CDROID_TRANSITION_SIDEPROPAGATION_H__

#include <transition/visibilitypropagation.h>
#include <view/gravity.h>

namespace cdroid {

/**
 * A TransitionPropagation that propagates based on the distance to a side and, orthogonally,
 * the distance to the epicenter. Default propagation for Slide.
 * Ported from android-36 android.transition.SidePropagation.
 */
class SidePropagation: public VisibilityPropagation {
  public:
    void setSide(int side) {
        mSide = side;
    }
    void setPropagationSpeed(float propagationSpeed);
    long getStartDelay(ViewGroup* sceneRoot, Transition* transition,
                       TransitionValues* startValues, TransitionValues* endValues) override;

  private:
    int distance(ViewGroup* sceneRoot, int viewX, int viewY, int epicenterX, int epicenterY,
                 int left, int top, int right, int bottom);
    int getMaxDistance(ViewGroup* sceneRoot);

    float mPropagationSpeed = 3.0f;
    int mSide = Gravity::BOTTOM;
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_SIDEPROPAGATION_H__
