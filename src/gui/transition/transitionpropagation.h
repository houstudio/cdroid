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
#ifndef __CDROID_TRANSITION_TRANSITIONPROPAGATION_H__
#define __CDROID_TRANSITION_TRANSITIONPROPAGATION_H__

#include <string>
#include <vector>

namespace cdroid {

class ViewGroup;
class Transition;
class TransitionValues;

/**
 * Extend TransitionPropagation to customize start delays for Animators created
 * in Transition#createAnimator. A Transition such as Explode defaults to using
 * CircularPropagation; Views closer to the epicenter move out later and in
 * sooner, giving the appearance of inertia. With no TransitionPropagation, all
 * Views react simultaneously.
 *
 * Ported from android-36 android.transition.TransitionPropagation.
 */
class TransitionPropagation {
  public:
    virtual ~TransitionPropagation() = default;

    /**
     * Called by Transition to alter the Animator start delay. All start delays
     * are adjusted such that the minimum becomes zero. Returned values may be
     * negative.
     */
    virtual long getStartDelay(ViewGroup* sceneRoot, Transition* transition,
                               TransitionValues* startValues, TransitionValues* endValues) = 0;

    /** Captures the values this propagation monitors in the start/end scene. */
    virtual void captureValues(TransitionValues* transitionValues) = 0;

    /** Property names captured by #captureValues (prevents duplicate capturing). */
    virtual std::vector<std::string> getPropagationProperties() = 0;
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_TRANSITIONPROPAGATION_H__
