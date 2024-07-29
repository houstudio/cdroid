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
    std::vector<long> timestamps;
    cdroid::Path mCachedPath;
    GestureStroke(const RectF& bbx, float len,const std::vector<float>& pts,const std::vector<long>&times);
    void makePath(); 
public:
    /**
     * A constructor that constructs a gesture stroke from a list of gesture points.
     *
     * @param points
     */
    GestureStroke(const std::vector<GesturePoint>& points);

    /**
     * Draws the stroke with a given canvas and paint.
     *
     * @param canvas
     */
    void draw(Canvas& canvas);
    cdroid::Path getPath();
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
