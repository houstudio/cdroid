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
#include <iostream>
#include <gesture/gesturestore.h>
#include <gesture/gestureutils.h>
#include <gesture/gesturestroke.h>
#include <porting/cdlog.h>

namespace cdroid{
GestureStroke::GestureStroke(const std::vector<GesturePoint>& points) {
    const int count = (int)points.size();
    std::vector<float> tmpPoints(count * 2);
    std::vector<int64_t> times(count);

    RectF bx{0,0,0,0};
    float len = 0;
    int index = 0;
    mCachedPath = nullptr;

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

GestureStroke::GestureStroke(const GestureStroke&other){
    mCachedPath = nullptr;
    points = other.points;
    length = other.length;
    timestamps = other.timestamps;
    boundingBox= other.boundingBox;
}

GestureStroke::~GestureStroke(){
    delete mCachedPath;
}

/**
 * A faster constructor specially for cloning a stroke.
 */
GestureStroke::GestureStroke(const RectF& bbx, float len,const std::vector<float>& pts,const std::vector<int64_t>&times) {
    boundingBox = bbx;//new RectF(bbx.left, bbx.top, bbx.width, bbx.height);
    length = len;
    points = pts;
    mCachedPath = nullptr;
    timestamps = times;
}

static void quad_to(Canvas&c,double x1, double y1, double x2, double y2){
    double x0, y0;
    c.get_current_point(x0,y0);

    //Control points for cubic bezier curve
    double cp1x = x0 + 2.f / 3.f * (x1 - x0);
    double cp1y = y0 + 2.f / 3.f * (y1 - y0);
    double cp2x = cp1x + (x2 - x0) / 3.f;
    double cp2y = cp1y + (y2 - y0) / 3.f;
    c.curve_to(cp1x, cp1y, cp2x, cp2y, x2, y2);
}

/**
 * Draws the stroke with a given canvas and paint.
 *
 * @param canvas
 */
void GestureStroke::draw(Canvas& canvas) {
    if (mCachedPath == nullptr) {
        makePath();
    }
    //canvas.drawPath(*mCachedPath);
    std::vector<float>& localPoints = points;
    const int count = (int)localPoints.size();

    float mX = 0, mY = 0;
    for (int i = 0; i < count; i += 2) {
        const float x = localPoints[i];
        const float y = localPoints[i + 1];
        if (i==0) {
            canvas.move_to(x, y);
            mX = x;  mY = y;
        } else {
            float dx = std::abs(x - mX);
            float dy = std::abs(y - mY);
            if (dx >= TOUCH_TOLERANCE || dy >= TOUCH_TOLERANCE) {
                quad_to(canvas,mX, mY, (x + mX) / 2, (y + mY) / 2);
                mX = x;  mY = y;
            }
        }
    }
}

cdroid::Path* GestureStroke::getPath() {
    if (mCachedPath == nullptr) {
        makePath();
    }

    return mCachedPath;
}

void GestureStroke::makePath() {
    std::vector<float>& localPoints = points;
    const int count = (int)localPoints.size();

    Path* path=nullptr;

    float mX = 0, mY = 0;

    for (int i = 0; i < count; i += 2) {
        float x = localPoints[i];
        float y = localPoints[i + 1];
        if (path == nullptr) {
            path = new Path;
            path->move_to(x, y);
            mX = x;  mY = y;
        } else {
            float dx = std::abs(x - mX);
            float dy = std::abs(y - mY);
            if (dx >= TOUCH_TOLERANCE || dy >= TOUCH_TOLERANCE) {
                path->quad_to(mX, mY, (x + mX) / 2, (y + mY) / 2);
                mX = x;  mY = y;
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

    const int count = (int)pts.size();

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
    const int count = (int)points.size();

    // Write number of points
    GestureIOHelper::writeInt(out,count / 2);

    for (int i = 0; i < count; i += 2) {
        // Write X
        GestureIOHelper::writeFloat(out,points[i]);//pts[i]);
        // Write Y
        GestureIOHelper::writeFloat(out,points[i+1]);//pts[i + 1]);
        // Write timestamp
        GestureIOHelper::writeLong(out,timestamps[i/2]);//times[i / 2]);
    }
}

GestureStroke* GestureStroke::deserialize(std::istream& in){
    // Number of points
    const int count = GestureIOHelper::readInt(in);

    std::vector<GesturePoint> points;
    for (int i = 0; i < count; i++) {
        points.push_back(GesturePoint::deserialize(in));
    }
    LOGD("\t\t%d points",count);
    return new GestureStroke(points);
}

/**
 * Invalidates the cached path that is used to render the stroke.
 */
void GestureStroke::clearPath() {
    if (mCachedPath != nullptr) mCachedPath->reset();//rewind();
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
