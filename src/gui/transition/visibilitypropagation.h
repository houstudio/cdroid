/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.VisibilityPropagation.
 *********************************************************************************/
#ifndef __CDROID_TRANSITION_VISIBILITYPROPAGATION_H__
#define __CDROID_TRANSITION_VISIBILITYPROPAGATION_H__

#include <string>
#include <vector>

#include <transition/transitionpropagation.h>

namespace cdroid{

class View;

/**
 * Base class for TransitionPropagations that care about View visibility and the center
 * position of the View. Ported from android-36 android.transition.VisibilityPropagation.
 */
class VisibilityPropagation: public TransitionPropagation{
public:
    void captureValues(TransitionValues* values) override;
    std::vector<std::string> getPropagationProperties() override;

    int getViewVisibility(TransitionValues* values);
    int getViewX(TransitionValues* values){ return getViewCoordinate(values, 0); }
    int getViewY(TransitionValues* values){ return getViewCoordinate(values, 1); }

    static constexpr const char* PROPNAME_VISIBILITY  = "android:visibilityPropagation:visibility";
    static constexpr const char* PROPNAME_VIEW_CENTER = "android:visibilityPropagation:center";

private:
    static int getViewCoordinate(TransitionValues* values, int coordinateIndex);
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_VISIBILITYPROPAGATION_H__
