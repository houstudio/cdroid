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
#include <numeric>
#include <cfloat>
#include <porting/cdlog.h>
#include <core/pathmeasure.h>
namespace cdroid {

PathMeasure::PathMeasure(){
    mForceClosed = false;
}

PathMeasure::PathMeasure(Cairo::RefPtr<cdroid::Path>inPath,bool forceClosed){
    mPath = inPath;
    mForceClosed = forceClosed;
    buildSegments();
}

void PathMeasure::setPath(Cairo::RefPtr<cdroid::Path>inPath){
    mPath = inPath;
    buildSegments();
}

void PathMeasure::setPath(Cairo::RefPtr<cdroid::Path>inPath,bool forceClosed){
    mPath = inPath;
    mForceClosed = forceClosed;
    buildSegments();
}

namespace{
    double distancePoint(const PointD& p1, const PointD& p2){
        const double dx = p2.x - p1.x;
        const double dy = p2.y - p1.y;
        return std::sqrt(dx*dx+dy*dy);
    }

    PointD interpolate(const PointD& p1, const PointD& p2, double t){
        return { p1.x + t * (p2.x - p1.x), p1.y + t * (p2.y - p1.y) };
    }

    PointD interpolateCurve(const PointD& p0, const PointD& p1, const PointD& p2, const PointD& p3, double t){
        const double u = 1.0 - t;
        const double tt = t * t;
        const double uu = u * u;
        const double uuu = uu * u;
        const double ttt = tt * t;

        return{
            uuu * p0.x + 3.0 * uu * t * p1.x + 3.0 * u * tt * p2.x + ttt * p3.x,
            uuu * p0.y + 3.0 * uu * t * p1.y + 3.0 * u * tt * p2.y + ttt * p3.y
        };
    }

#define CURVE_STEPS 36
    double curveLength(const PointD& p0, const PointD& p1, const PointD& p2, const PointD& p3){
        // Approximate the length of a cubic Bezier curve using a simple method
        double length = 0.0;
        PointD prev = p0;
        const int steps = CURVE_STEPS;
        for (int i = 1; i <= steps; ++i) {
            const double t = static_cast<double>(i) / steps;
            PointD point = interpolateCurve(p0, p1, p2, p3, t);
            length += distancePoint(prev, point);
            prev = point;
        }
        return length;
    }

    double pointToLineDistance(const PointD& point, const PointD& lineStart, const PointD& lineEnd) {
        const double dx = lineEnd.x - lineStart.x;
        const double dy = lineEnd.y - lineStart.y;
        const double lineLengthSq = dx * dx + dy * dy;

        if (lineLengthSq == 0) {
            return distancePoint(point, lineStart);
        }

        double t = ((point.x - lineStart.x) * dx + (point.y - lineStart.y) * dy) / lineLengthSq;
        t = std::max(0.0, std::min(1.0, t));

        PointD projection={ lineStart.x + t * dx, lineStart.y + t * dy };

        return distancePoint(point, projection);
    }

    //Is Curve Flat Enough
    bool isFlatEnough(const PointD& p0, const PointD& p1, const PointD& p2, const PointD& p3, double tolerance) {
        const double dist1 = pointToLineDistance(p1, p0, p3);
        const double dist2 = pointToLineDistance(p2, p0, p3);
        return std::max(dist1, dist2) <= tolerance;
    }

    // 二分法分割三次贝塞尔曲线
    void chopCubicAt(const PointD src[4], PointD dst[7], double t) {
        double ab_x = src[0].x + (src[1].x - src[0].x) * t;
        double ab_y = src[0].y + (src[1].y - src[0].y) * t;

        double bc_x = src[1].x + (src[2].x - src[1].x) * t;
        double bc_y = src[1].y + (src[2].y - src[1].y) * t;

        double cd_x = src[2].x + (src[3].x - src[2].x) * t;
        double cd_y = src[2].y + (src[3].y - src[2].y) * t;

        double abc_x = ab_x + (bc_x - ab_x) * t;
        double abc_y = ab_y + (bc_y - ab_y) * t;

        double bcd_x = bc_x + (cd_x - bc_x) * t;
        double bcd_y = bc_y + (cd_y - bc_y) * t;

        dst[0] = src[0];
        dst[1] = {ab_x, ab_y};
        dst[2] = {abc_x, abc_y};
        dst[3] = {abc_x + (bcd_x - abc_x) * t, abc_y + (bcd_y - abc_y) * t};
        dst[4] = {bcd_x, bcd_y};
        dst[5] = {cd_x, cd_y};
        dst[6] = src[3];
    }

    // 二分法分割三次贝塞尔曲线（中点）
    void chopCubicAtHalf(const PointD src[4], PointD dst[7]) {
        double x12 = (src[0].x + src[1].x) * 0.5;
        double y12 = (src[0].y + src[1].y) * 0.5;
        double x23 = (src[1].x + src[2].x) * 0.5;
        double y23 = (src[1].y + src[2].y) * 0.5;
        double x34 = (src[2].x + src[3].x) * 0.5;
        double y34 = (src[2].y + src[3].y) * 0.5;

        double x123 = (x12 + x23) * 0.5;
        double y123 = (y12 + y23) * 0.5;
        double x234 = (x23 + x34) * 0.5;
        double y234 = (y23 + y34) * 0.5;

        double x1234 = (x123 + x234) * 0.5;
        double y1234 = (y123 + y234) * 0.5;

        dst[0] = src[0];
        dst[1] = {x12, y12};
        dst[2] = {x123, y123};
        dst[3] = {x1234, y1234};
        dst[4] = {x234, y234};
        dst[5] = {x34, y34};
        dst[6] = src[3];
    }

    // 递归计算三次贝塞尔曲线长度
    double computeCubicLength(const PointD& p0, const PointD& p1, const PointD& p2, const PointD& p3, double tolerance) {
        if (isFlatEnough(p0, p1, p2, p3, tolerance)) {
            return distancePoint(p0, p3);
        } else {
            PointD pts[4] = {p0, p1, p2, p3};
            PointD pieces[7];
            chopCubicAtHalf(pts, pieces);

            return computeCubicLength(pieces[0], pieces[1], pieces[2], pieces[3], tolerance) +
                   computeCubicLength(pieces[3], pieces[4], pieces[5], pieces[6], tolerance);
        }
    }

    double curveLength(const PointD& p0, const PointD& p1, const PointD& p2, const PointD& p3, double tolerance = 1e-3) {
        if ((p0.x == p1.x) && (p0.y == p1.y) && (p2.x == p3.x) && (p2.y == p3.y)) {
            return distancePoint(p0, p3);
        }
        return computeCubicLength(p0, p1, p2, p3, tolerance);
    }

    void bezierSplit(const PointD& p0, const PointD& p1, const PointD& p2, const PointD& p3,
           double t0, double t1, PointD& result0, PointD& result1, PointD& result2, PointD& result3){
        // 1st De Casteljau
        PointD p01 = interpolate(p0, p1, t0);
        PointD p12 = interpolate(p1, p2, t0);
        PointD p23 = interpolate(p2, p3, t0);
        PointD p012 = interpolate(p01, p12, t0);
        PointD p123 = interpolate(p12, p23, t0);
        PointD p0123 = interpolate(p012, p123, t0);

        // 2nd De Casteljau
        PointD q01 = interpolate(p0, p1, t1);
        PointD q12 = interpolate(p1, p2, t1);
        PointD q23 = interpolate(p2, p3, t1);
        PointD q012 = interpolate(q01, q12, t1);
        PointD q123 = interpolate(q12, q23, t1);
        PointD q0123 = interpolate(q012, q123, t1);

        const double t = (t1 - t0) / (1.0 - t0);
        PointD r01 = interpolate(p01, p12, t);
        PointD r12 = interpolate(p12, p23, t);
        PointD r012 = interpolate(r01, r12, t);

        result0 = p0123;
        result1 = r01;
        result2 = r012;
        result3 = q0123;
    }
}

double PathMeasure::getLength() const{
    return std::accumulate(mSegments.begin(),mSegments.end(),0,
        [](double sum,const Segment&s){ return sum + s.distance;});
}

int PathMeasure::buildSegments(){
    bool hasMove = false;
    int i = 0 ,ptIndex = -1;
    PointD pt0 = {0,0};
    PointD ptStart = {0,0};

    const auto m_path = mPath->copy_path();
    mPoints.clear();
    mSegments.clear();
    while (i < m_path->num_data) {
        const cairo_path_data_t* data = &m_path->data[i];
        const int type = data->header.type;
        i += data->header.length;

        if (type == CAIRO_PATH_MOVE_TO) {
            ptStart = pt0 = {data[1].point.x,data[1].point.y};
            ptIndex += 1;
            mPoints.push_back(pt0);
            hasMove = true;
        } else if (type == CAIRO_PATH_LINE_TO) {
            const PointD pt1 = {data[1].point.x,data[1].point.y};
            const double distance = std::hypot(pt1.x - pt0.x, pt1.y - pt0.y);
            mPoints.push_back(pt1);
            mSegments.push_back({Segment::Line,ptIndex, distance});
            ptIndex += 1;
            pt0 = pt1;
        } else if (type == CAIRO_PATH_CURVE_TO) {
            const PointD pt1 = {data[1].point.x,data[1].point.y};
            const PointD pt2 = {data[2].point.x,data[2].point.y};
            const PointD pt3 = {data[3].point.x,data[3].point.y};
            /* 用任意快速弧长估算，这里直接采样 16 段 */
            const double distance = curveLength(pt0,pt1,pt2,pt3,0.1);
            /*PointD prev = pt0;
            for (int k = 1; k <= CURVE_STEPS; ++k) {
                const double t = k / double(CURVE_STEPS);
                const double mt = 1.0-t;
                double ptx = mt*mt*mt*pt0.x + 3*mt*mt*t*pt1.x + 3*mt*t*t*pt2.x + t*t*t*pt3.x;
                double pty = mt*mt*mt*pt0.y + 3*mt*mt*t*pt1.y + 3*mt*t*t*pt2.y + t*t*t*pt3.y;

                distance += std::hypot(ptx - prev.x, pty - prev.y);
                prev = {ptx,pty};
            }*/
            mPoints.push_back(pt1);
            mPoints.push_back(pt2);
            mPoints.push_back(pt3);
            mSegments.push_back({Segment::Cubic,ptIndex, distance});
            ptIndex += 3;
            pt0 = pt3;
        }  else if (type == CAIRO_PATH_CLOSE_PATH) {
            const double distance = std::hypot(ptStart.x - pt0.x, ptStart.y - pt0.y);
            if (distance > 0) {
                mSegments.push_back({Segment::Line,ptIndex, distance});
                ptIndex += 1;
                mPoints.push_back(ptStart);
            }
            pt0 = ptStart;
        }
    }
    if (mForceClosed && hasMove && (std::hypot(pt0.x - ptStart.x, pt0.y - ptStart.y) > FLT_EPSILON)) {
        const double distance = std::hypot(ptStart.x - pt0.x, ptStart.y - pt0.y);
        mSegments.push_back({Segment::Line,ptIndex, distance});
        mPoints.push_back(ptStart);
    }
    return mSegments.size();
}

bool PathMeasure::getSegment(double start, double stop,Cairo::RefPtr<cdroid::Path>& dst, bool startWithMoveTo){
    if (!dst) dst = std::make_shared<cdroid::Path>();
    else dst->reset();

    if (start < 0) start = 0;
    if (stop>1.0) stop = 1.0;
    if (stop  < start) return false;

    if(mSegments.empty()){
        buildSegments();
    }

    const double total = getLength();
    const double startD= start*total;
    const double stopD = stop*total;
    LOGV("nsegs=%d/%d totallen=%f",mSegments.size(),mSegments.size(),total);

    bool needsMove = startWithMoveTo;
    double segStart = 0.0,segEnd = 0.0;
    for (auto& s:mSegments) {
        const PointD*pts = mPoints.data()+s.ptIndex;
        segEnd += s.distance;
        const double t0 = (startD >segStart) ? (startD - segStart) / s.distance : 0.0;
        const double t1 = (stopD < segEnd) ? (stopD  - segStart) / s.distance : 1.0;
        if (s.type == Segment::Line) {
            PointD p0 = interpolate(pts[0],pts[1],t0);
            PointD p1 = interpolate(pts[0],pts[1],t1);
            if (needsMove) { 
                dst->move_to(p0.x, p0.y);
                //needsMove = false;
            }
            dst->line_to(p1.x, p1.y);
        } else { // Cubic
            PointD q0, q1, q2, q3;
            if ((t0 == 0.0) && (t1 == 1.0)) {
                q0 = pts[0]; q1 = pts[1]; q2 = pts[1]; q3 = pts[3];
            } else if(t1>t0+FLT_EPSILON){
                bezierSplit(pts[0],pts[1],pts[2],pts[3],t0,t1, q0, q1, q2, q3);
            }
            if (needsMove) { 
                dst->move_to(q0.x, q0.y); 
                //needsMove = false;
            }
            dst->curve_to(q1.x, q1.y, q2.x, q2.y, q3.x, q3.y);
        }
        segStart += s.distance;
        if(segEnd >= stopD)break;
    }
    return true;
}

bool PathMeasure::getPosTan(double distance,PointD* pos,PointD* tangent){
    PointD prev, curr;
    bool found = false;
    double length = 0.0 , segLength = 0.0;
    const auto m_path = mPath->copy_path();

    for (int i = 0; i < m_path->num_data && !found; ) {
        cairo_path_data_t* data = &m_path->data[i];
        i+= data->header.length;
        switch (data->header.type) {
        case CAIRO_PATH_MOVE_TO:
            prev = {data[1].point.x, data[1].point.y};
            break;
        case CAIRO_PATH_LINE_TO:
            curr = {data[1].point.x, data[1].point.y};
            segLength = distancePoint(prev, curr);
            if (length + segLength >= distance) {
                const double t = (distance - length) / segLength;
                if(pos)*pos = interpolate(prev,curr,t);
                if(tangent)*tangent = {curr.x - prev.x,curr.y - prev.y};
                found = true;
            }
            length += segLength;
            prev = curr;
            break;
        case CAIRO_PATH_CURVE_TO: {
            PointD p1 = { data[1].point.x, data[1].point.y };
            PointD p2 = { data[2].point.x, data[2].point.y };
            PointD p3 = { data[3].point.x, data[3].point.y };
            segLength = curveLength(prev, p1, p2, p3,0.1);
            if (length + segLength >= distance) {
                const double t = (distance - length) / segLength;
                const double u = 1.0 - t;
                if(pos)*pos = interpolateCurve(prev, p1, p2, p3, t);
                if(tangent){/*Tangent is the first derivative of Bézier curve*/
                    tangent->x =  3 * u * u * (p1.x - prev.x) +
                        6 * u * t * (p2.x - p1.x) + 3 * t * t * (p3.x - p2.x);
                    tangent->y =  3 * u * u * (p1.y - prev.y) +
                        6 * u * t * (p2.y - p1.y) + 3 * t * t * (p3.y - p2.y);
                }
                found = true;
            }
            length += segLength;
            prev = p3;
            break;
        }
        case CAIRO_PATH_CLOSE_PATH:
        default:    break;
        }
    }
    cairo_path_destroy(m_path);
    return found;
}
}/*endof namespace*/
