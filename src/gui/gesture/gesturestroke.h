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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __GESTURE_STROKE_H__
#define __GESTURE_STROKE_H__
#include <vector>
#include <core/rect.h>
#include <core/path.h>
#include <core/canvas.h>
#include <gesture/gesturepoint.h>
#include <gesture/orientedboundingbox.h>

namespace cdroid{
class GestureStroke {
public:
    static constexpr float TOUCH_TOLERANCE = 3;
    RectF boundingBox;
    float length;
    std::vector<float> points;
private:
    std::vector<int64_t> timestamps;
    cdroid::Path* mCachedPath;
    GestureStroke(const RectF& bbx, float len,const std::vector<float>& pts,const std::vector<int64_t>&times);
    void makePath(); 
public:
    GestureStroke(const GestureStroke&);
    GestureStroke(const std::vector<GesturePoint>& points);
    virtual ~GestureStroke();
    /**
     * Draws the stroke with a given canvas and paint.
     *
     * @param canvas
     */
    void draw(Canvas& canvas);
    cdroid::Path* getPath();
    /**
     * Converts the stroke to a Path of a given number of points.
     *
     * @param width the width of the bounding box of the target path
     * @param height the height of the bounding box of the target path
     * @param numSample the number of points needed
     *
     * @return the path
     */
    cdroid::Path* toPath(float width, float height, int numSample);
    void serialize(std::ostream& out);
    static GestureStroke* deserialize(std::istream& in);

    /**
     * Invalidates the cached path that is used to render the stroke.
     */
    void clearPath();

    /**
     * Computes an oriented bounding box of the stroke.
     *
     * @return OrientedBoundingBox
     */
    OrientedBoundingBox* computeOrientedBoundingBox();
};
}/*endof namespace*/
#endif/*__GESTURE_STROKE_H__*/
