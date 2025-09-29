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

#ifndef __PATH_MEASURE_H__
#define __PATH_MEASURE_H__
#include <core/path.h>
#include <cairomm/context.h>
#include <drawables/hwvectordrawable.h>
namespace cdroid {
class PathMeasure{
public:
    struct Segment {
        enum Type { Line, Cubic } type;
        PointD p0, p1, p2, p3;   // Cubic 时全部有效；Line 时仅 p0/p1 有效
        double  len;               // 该段长度（预先算好）
    };
private:
    double mTotalLength;
    Cairo::RefPtr<cdroid::Path>mPath;
    double distance(const PointD&p1,const PointD&p2);
    double curveLength(const PointD& p0, const PointD& p1, const PointD& p2, const PointD& p3);
    PointD interpolate(const PointD& p1, const PointD& p2, double t);
    PointD interpolateCurve(const PointD& p0, const PointD& p1, const PointD& p2, const PointD& p3, double t);
    void bezierSplit(const PointD& p0, const PointD& p1, const PointD& p2, const PointD& p3, double t0, double t1,
                PointD& q0, PointD& q1, PointD& q2, PointD& q3);
    void bezierSplitSingle(const PointD& p0, const PointD& p1, const PointD& p2, const PointD& p3, double t,
        PointD& left0, PointD& left1, PointD& left2, PointD& left3,
        PointD& right0, PointD& right1, PointD& right2, PointD& right3);
    double calculateTotalLength();
    int buildSegments(std::vector<Segment>&segs,std::vector<double>& accumulatedLen);
public:
    PathMeasure();
    PathMeasure(Cairo::RefPtr<cdroid::Path>inPath,bool);
    void setPath(Cairo::RefPtr<cdroid::Path>inPath);
    double getLength();
    bool getSegment(double startD, double stopD, Cairo::RefPtr<cdroid::Path>& dst, bool startWithMoveTo);
    bool getPosTan(double distance,PointD* pos,PointD* tangent) ;
};
}
#endif
