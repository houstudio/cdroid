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
    mSegments.clear();
    buildSegments();
}

void PathMeasure::setPath(Cairo::RefPtr<cdroid::Path>inPath,bool forceClosed){
    mPath = inPath;
    mForceClosed = forceClosed;
    mSegments.clear();
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
            double t = static_cast<double>(i) / steps;
            PointD point = interpolateCurve(p0, p1, p2, p3, t);
            length += distancePoint(prev, point);
            prev = point;
        }
        return length;
    }

    void bezierSplit(const PointD& p0, const PointD& p1, const PointD& p2, const PointD& p3,
           double t0, double t1, PointD& result0, PointD& result1, PointD& result2, PointD& result3){
        // 第一次 De Casteljau 分割
        PointD p01 = interpolate(p0, p1, t0);
        PointD p12 = interpolate(p1, p2, t0);
        PointD p23 = interpolate(p2, p3, t0);
        PointD p012 = interpolate(p01, p12, t0);
        PointD p123 = interpolate(p12, p23, t0);
        PointD p0123 = interpolate(p012, p123, t0);
        
        // 第二次 De Casteljau 分割
        PointD q01 = interpolate(p0, p1, t1);
        PointD q12 = interpolate(p1, p2, t1);
        PointD q23 = interpolate(p2, p3, t1);
        PointD q012 = interpolate(q01, q12, t1);
        PointD q123 = interpolate(q12, q23, t1);
        PointD q0123 = interpolate(q012, q123, t1);
        
        // 重新参数化
        double t = (t1 - t0) / (1.0 - t0);
        PointD r01 = interpolate(p01, p12, t);
        PointD r12 = interpolate(p12, p23, t);
        PointD r012 = interpolate(r01, r12, t);
        
        // 结果
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
    int i = 0,ptIndex=-1;
    PointD pt0 = {0,0};
    PointD ptStart={0,0};

    const auto m_path = mPath->copy_path();

    while (i < m_path->num_data) {
        const cairo_path_data_t* data = &m_path->data[i];
        const int type = data->header.type;
        i += data->header.length;

        if (type == CAIRO_PATH_MOVE_TO) {
            ptStart = pt0 = {data[1].point.x,data[1].point.y};
            ptIndex+=1;
            mPoints.push_back(pt0);
            hasMove = true;
        } else if (type == CAIRO_PATH_LINE_TO) {
            const PointD pt1 = {data[1].point.x,data[1].point.y};
            const double y1 = data[1].point.y;
            const double len = std::hypot(pt1.x - pt0.x, pt1.y - pt0.y);
            mPoints.push_back(pt1);
            mSegments.push_back({Segment::Line,ptIndex, len});
            ptIndex += 1;
            pt0 = pt1;
        } else if (type == CAIRO_PATH_CURVE_TO) {
            const PointD pt1 = {data[1].point.x,data[1].point.y};
            const PointD pt2 = {data[2].point.x,data[2].point.y};
            const PointD pt3 = {data[3].point.x,data[3].point.y};
            /* 用任意快速弧长估算，这里直接采样 16 段 */
            double len = 0;
            PointD prev = pt0;
            for (int k = 1; k <= CURVE_STEPS; ++k) {
                const double t = k / double(CURVE_STEPS);
                const double mt = 1.0-t;
                double ptx = mt*mt*mt*pt0.x + 3*mt*mt*t*pt1.x + 3*mt*t*t*pt2.x + t*t*t*pt3.x;
                double pty = mt*mt*mt*pt0.y + 3*mt*mt*t*pt1.y + 3*mt*t*t*pt2.y + t*t*t*pt3.y;

                len += std::hypot(ptx - prev.x, pty - prev.y);
                prev = {ptx,pty};
            }
            mPoints.push_back(pt1);
            mPoints.push_back(pt2);
            mPoints.push_back(pt3);
            mSegments.push_back({Segment::Cubic,ptIndex, len});
            ptIndex += 3;
            pt0 = pt3;
        }  else if (type == CAIRO_PATH_CLOSE_PATH) {
            const double len = std::hypot(ptStart.x - pt0.x, ptStart.y - pt0.y);
            if (len > 0) {
                mSegments.push_back({Segment::Line,ptIndex, len});
                ptIndex += 1;
                mPoints.push_back(ptStart);
            }
            pt0 = ptStart;
        }
    }
    if (mForceClosed && hasMove && (std::hypot(pt0.x - ptStart.x, pt0.y - ptStart.y) > FLT_EPSILON)) {
        const double len = std::hypot(ptStart.x - pt0.x, ptStart.y - pt0.y);
        mSegments.push_back({Segment::Line,ptIndex, len});
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
    double length = 0.0;
    PointD prev, curr, next;
    auto m_path = mPath->copy_path();
    bool found = false;

    for (int i = 0; i < m_path->num_data && !found; ) {
        cairo_path_data_t* data = &m_path->data[i];
        i+= data->header.length;
        switch (data->header.type) {
        case CAIRO_PATH_MOVE_TO:
            prev = { data[1].point.x, data[1].point.y };
            break;
        case CAIRO_PATH_LINE_TO: {
            curr = { data[1].point.x, data[1].point.y };
            double segLen = distancePoint(prev, curr);
            if (length + segLen >= distance) {
                double t = (distance - length) / segLen;
                pos->x = prev.x + t * (curr.x - prev.x);
                pos->y = prev.y + t * (curr.y - prev.y);
                tangent->x = curr.x - prev.x;
                tangent->y = curr.y - prev.y;
                found = true;
            }
            length += segLen;
            prev = curr;
            break;
        }
        case CAIRO_PATH_CURVE_TO: {
            PointD p1 = { data[1].point.x, data[1].point.y };
            PointD p2 = { data[2].point.x, data[2].point.y };
            PointD p3 = { data[3].point.x, data[3].point.y };
            double segLen = curveLength(prev, p1, p2, p3);
            if (length + segLen >= distance) {
                double t = (distance - length) / segLen;
                PointD pt = interpolateCurve(prev, p1, p2, p3, t);
                pos->x = pt.x;
                pos->y = pt.y;
                // 切线为贝塞尔一阶导数
                double u = 1.0 - t;
                tangent->x =
                    3 * u * u * (p1.x - prev.x) +
                    6 * u * t * (p2.x - p1.x) +
                    3 * t * t * (p3.x - p2.x);
                tangent->y =
                    3 * u * u * (p1.y - prev.y) +
                    6 * u * t * (p2.y - p1.y) +
                    3 * t * t * (p3.y - p2.y);
                found = true;
            }
            length += segLen;
            prev = p3;
            break;
        }
        case CAIRO_PATH_CLOSE_PATH:
            // 可选：处理闭合段
        default:
            break;
        }
    }
    cairo_path_destroy(m_path);
    return found;
}
}/*endof namespace*/
