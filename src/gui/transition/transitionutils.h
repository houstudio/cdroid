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
#include <cairomm/surface.h>
#include <cairomm/matrix.h>

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
     * Creates a snapshot View of <code>view</code>, laid out at its position mapped
     * into <code>sceneRoot</code> coordinates. android renders the view into a Bitmap
     * wrapped in an ImageView; CDROID renders into a Cairo::ImageSurface (the same
     * surface role the Crossfade snapshot plays) and wraps it the same way.
     *
     * Ownership: the caller owns the returned view and must delete it once it is no
     * longer in the overlay (android relies on GC here; Visibility::onDisappear is
     * the sole caller and deletes it in its cleanup paths).
     *
     * @param sceneRoot The ViewGroup in which the view copy will be displayed.
     * @param view The view to create a copy of.
     * @param parent The parent of view.
     */
    static View* copyViewImage(ViewGroup* sceneRoot, View* view, ViewGroup* parent);

    /**
     * Creates an ImageSurface of the given view, using the matrix to transform to the
     * destination coordinates. <code>matrix</code> will be modified during the bitmap
     * creation. If the bitmap is large, it is scaled uniformly down to at most
     * MAX_IMAGE_SIZE pixels. A view that is not attached to a window is temporarily
     * added to sceneRoot's overlay for the duration of the rendering.
     *
     * @param view The view to create a bitmap for.
     * @param matrix The matrix converting view-local coordinates to the coordinates
     *               the bitmap will be displayed in.
     * @param bounds The bounds of the bitmap in the destination coordinate system.
     * @param sceneRoot A ViewGroup attached to the window to temporarily contain the
     *                  view if it isn't attached.
     * @return A bitmap of the given view, or a null RefPtr if bounds has no area.
     */
    static Cairo::RefPtr<Cairo::ImageSurface> createViewBitmap(View* view, Cairo::Matrix& matrix,
            Cairo::Rectangle& bounds, ViewGroup* sceneRoot);
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_TRANSITIONUTILS_H__
