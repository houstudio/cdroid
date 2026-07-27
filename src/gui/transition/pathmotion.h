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
#ifndef __CDROID_TRANSITION_PATHMOTION_H__
#define __CDROID_TRANSITION_PATHMOTION_H__

#include <core/path.h> // cdroid::Path (android.graphics.Path naming)

namespace cdroid {

class Context;
class AttributeSet;

/**
 * This base class can be extended to provide motion along a Path to Transitions.
 * Ported from android-36 android.transition.PathMotion.
 *
 * Transitions such as ChangeBounds move Views, typically in a straight path
 * between the start and end positions. Applications that desire curved motion
 * extend PathMotion and implement #getPath to control two-dimensional interpolation.
 */
class PathMotion {
  public:
    PathMotion() = default;
    PathMotion(Context* context, AttributeSet* attrs) {
        (void)context;
        (void)attrs;
    }
    virtual ~PathMotion() = default;

    /**
     * Provide a Path to interpolate between (startX,startY) and (endX,endY).
     * The returned Path must start at (startX,startY) (via Path#moveTo) and end
     * at (endX,endY).
     */
    virtual Path getPath(float startX, float startY, float endX, float endY) = 0;
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_PATHMOTION_H__
