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
#ifndef __CDROID_TRANSITION_TRANSITIONUTILS_H__
#define __CDROID_TRANSITION_TRANSITIONUTILS_H__

#include <animation/animator.h>

#include <vector>

namespace cdroid {

class Transition;
class AnimatorSet;
class View;
class ViewGroup;

/**
 * Utility methods for transitions. Ported from android-36 android.transition.TransitionUtils.
 * Only the helpers currently consumed are implemented here; the remaining static
 * methods (Bitmap/drawable/View copying, color utils, etc.) are deferred to the
 * ChangeBounds/Crossfade batch.
 */
class TransitionUtils {
  public:
    /** Combine two animators into an AnimatorSet that plays them together (null-safe). */
    static Animator* mergeAnimators(Animator* animator1, Animator* animator2);

    /**
     * Copy a view's current rendering into a static overlay view. android renders the
     * view hierarchy into a Bitmap via DisplayList/Canvas. CDROID's cairo 2D substrate
     * does not yet expose an equivalent snapshot, so this returns nullptr (deferred to
     * the Crossfade batch). The overlay path in Visibility::onDisappear then produces no
     * animator for removed-from-hierarchy views; the visibility-change path is unaffected.
     */
    static View* copyViewImage(ViewGroup* sceneRoot, View* view, ViewGroup* parent);
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_TRANSITIONUTILS_H__
