#include <iostream>
#include <gesture/gestureutils.h>
#include <gesture/gesturestroke.h>
namespace cdroid{
GestureStroke::GestureStroke(const std::vector<GesturePoint>& points) {
    const int count = points.size();
    std::vector<float> tmpPoints(count * 2);
    std::vector<long> times(count);

    RectF bx{0,0,0,0};
    float len = 0;
    int index = 0;

    for (int i = 0; i < count; i++) {
        const GesturePoint p = points.at(i);
        tmpPoints[i * 2] = p.x;
        tmpPoints[i * 2 + 1] = p.y;
        times[index] = p.timestamp;

        if (i==0) {
            bx.set(p.x,p.y,0,0);
            len = 0;
        } else {
            len += std::hypot(p.x - tmpPoints[(i - 1) * 2], p.y - tmpPoints[(i -1) * 2 + 1]);
            bx.Union(p.x, p.y);
        }
        index++;
    }

    timestamps = times;
    this->points = tmpPoints;
    boundingBox = bx;
    length = len;
}

/**
 * A faster constructor specially for cloning a stroke.
 */
GestureStroke::GestureStroke(const RectF& bbx, float len,const std::vector<float>& pts,const std::vector<long>&times) {
    boundingBox = bbx;//new RectF(bbx.left, bbx.top, bbx.width, bbx.height);
    length = len;
    points = pts;
    timestamps = times;
}

/**
 * Draws the stroke with a given canvas and paint.
 *
 * @param canvas
 */
void GestureStroke::draw(Canvas& canvas) {
    if (1/*mCachedPath == null*/) {
        makePath();LOGD("TODO");
    }

    //canvas.drawPath(mCachedPath);
}

cdroid::Path GestureStroke::getPath() {
    if (1/*mCachedPath == null*/) {
        makePath();LOGD("TODO");
    }

    return mCachedPath;
}

void GestureStroke::makePath() {
    std::vector<float>& localPoints = points;
    const int count = localPoints.size();

    Path path;

    float mX = 0;
    float mY = 0;

    for (int i = 0; i < count; i += 2) {
        float x = localPoints[i];
        float y = localPoints[i + 1];
        if (i==0/*path == null*/) {
            path.move_to(x, y);
            mX = x;
            mY = y;
        } else {
            float dx = std::abs(x - mX);
            float dy = std::abs(y - mY);
            if (dx >= TOUCH_TOLERANCE || dy >= TOUCH_TOLERANCE) {
                path.quad_to(mX, mY, (x + mX) / 2, (y + mY) / 2);
                mX = x;
                mY = y;
            }
        }
    }

    mCachedPath = path;
}

/**
 * Converts the stroke to a Path of a given number of points.
 *
 * @param width the width of the bounding box of the target path
 * @param height the height of the bounding box of the target path
 * @param numSample the number of points needed
 *
 * @return the path
 */
cdroid::Path* GestureStroke::toPath(float width, float height, int numSample) {
    std::vector<float> pts = GestureUtils::temporalSampling(*this, numSample);
    RectF& rect = boundingBox;

    GestureUtils::translate(pts, -rect.left, -rect.top);

    float sx = width / rect.width;
    float sy = height / rect.height;
    float scale = sx > sy ? sy : sx;
    GestureUtils::scale(pts, scale, scale);

    float mX = 0;
    float mY = 0;

    Path* path=nullptr;

    const int count = pts.size();

    for (int i = 0; i < count; i += 2) {
        float x = pts[i];
        float y = pts[i + 1];
        if (path == nullptr) {
            path = new Path();
            path->move_to(x, y);
            mX = x;
            mY = y;
        } else {
            float dx = std::abs(x - mX);
            float dy = std::abs(y - mY);
            if (dx >= TOUCH_TOLERANCE || dy >= TOUCH_TOLERANCE) {
                path->quad_to(mX, mY, (x + mX) / 2, (y + mY) / 2);
                mX = x;
                mY = y;
            }
        }
    }

    return path;
}

void GestureStroke::serialize(std::ostream& out){
    //final float[] pts = points;
    //final long[] times = timestamps;
    const int count = points.size();

    // Write number of points
    //out.writeInt(count / 2);

    for (int i = 0; i < count; i += 2) {
        // Write X
        //out.writeFloat(points[i]);//pts[i]);
        // Write Y
        //out.writeFloat(points[i+1]);//pts[i + 1]);
        // Write timestamp
        //out.writeLong(timestamps[i/2]);//times[i / 2]);
    }
}

GestureStroke* GestureStroke::deserialize(std::istream& in){
    // Number of points
    const int count =0;// in.readInt();

    std::vector<GesturePoint> points;
    for (int i = 0; i < count; i++) {
        points.push_back(GesturePoint::deserialize(in));
    }

    return new GestureStroke(points);
}

/**
 * Invalidates the cached path that is used to render the stroke.
 */
void GestureStroke::clearPath() {
    LOGD("TODO");
    //if (mCachedPath != nullptr) mCachedPath.reset();//rewind();
}

/**
 * Computes an oriented bounding box of the stroke.
 *
 * @return OrientedBoundingBox
 */
OrientedBoundingBox* GestureStroke::computeOrientedBoundingBox() {
    return GestureUtils::computeOrientedBoundingBox(points);
}
}/*endof namespace*/
