/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.CircularPropagation.
 *********************************************************************************/
#ifndef __CDROID_TRANSITION_CIRCULARPROPAGATION_H__
#define __CDROID_TRANSITION_CIRCULARPROPAGATION_H__

#include <transition/visibilitypropagation.h>

namespace cdroid {

/**
 * A propagation that varies with the distance to the epicenter (or scene center). Default
 * propagation for Explode. Ported from android-36 android.transition.CircularPropagation.
 */
class CircularPropagation: public VisibilityPropagation {
  public:
    void setPropagationSpeed(float propagationSpeed);
    long getStartDelay(ViewGroup* sceneRoot, Transition* transition,
                       TransitionValues* startValues, TransitionValues* endValues) override;

  private:
    static double distance(float x1, float y1, float x2, float y2);
    float mPropagationSpeed = 3.0f;
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_CIRCULARPROPAGATION_H__
