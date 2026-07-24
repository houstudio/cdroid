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
#ifndef __CDROID_TRANSITION_TRANSITIONVALUES_H__
#define __CDROID_TRANSITION_TRANSITIONVALUES_H__

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <core/any.h> // nonstd::any (any-lite, third-party — do not modify)

namespace cdroid{

class View;
class Transition;

/**
 * Data structure which holds cached values for the transition.
 * Ported from android-36 android.transition.TransitionValues.
 *
 * The view field is the target which all of the values pertain to. The values
 * field is a map which holds information for fields according to names selected
 * by the transitions. These names should be unique to avoid clobbering values
 * stored by other transitions, such as the convention
 * "project:transition_name:property_name". For example, the platform might store
 * a property "alpha" in a transition "Fader" as "android:fader:alpha".
 *
 * These values are cached during the capture phases of a scene change, once for
 * the start values and again for the end values. They are then passed into the
 * transitions via Transition#createAnimator.
 */
class TransitionValues{
public:
    /** @deprecated Use TransitionValues(View) instead */
    TransitionValues() = default;
    explicit TransitionValues(View* view);

    // The View with these values. Nullable only via the deprecated default ctor
    // (android marks it @NonNull; the default ctor is the documented exception).
    View* view = nullptr;

    // The set of values tracked by transitions for this scene.
    // NOTE: android uses ArrayMap<String,Object> (insertion-ordered); CDROID uses
    // std::map (key-sorted). Order is not semantically significant for capture or
    // matching, only for the debug toString() output.
    std::map<std::string, nonstd::any> values;

    bool equals(const TransitionValues& other) const;
    int  hashCode() const;
    std::string toString() const;

private:
    friend class Transition;
    friend class TransitionSet; // captureStart/EndValues adds child transitions
    // The Transitions that targeted this view (package-private in android).
    std::vector<Transition*> targetedTransitions;
};

// TransitionValues objects are referenced simultaneously from the
// TransitionValuesMaps and from Transition's mStartValuesList/mEndValuesList
// (in android both hold the *same* object reference). std::shared_ptr reproduces
// that shared-identity / garbage-collected reference semantics in C++, and keeps
// references valid while Transition::matchInstances mutates the maps in place.
using TransitionValuesPtr = std::shared_ptr<TransitionValues>;

/**
 * Type-dispatched equality for the nonstd::any values stored in
 * TransitionValues#values. android compares with Object#equals; CDROID dispatches
 * on the held std::type_info for the value types transitions capture. Exposed
 * (not file-local) so Transition::isValueChanged can reuse it.
 */
bool anyValuesEqual(const nonstd::any& a, const nonstd::any& b);

} // namespace cdroid
#endif // __CDROID_TRANSITION_TRANSITIONVALUES_H__
