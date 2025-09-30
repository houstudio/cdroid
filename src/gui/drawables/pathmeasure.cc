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
#include <porting/cdlog.h>
#include <drawables/pathmeasure.h>
namespace cdroid {

PathMeasure::PathMeasure(){
}

PathMeasure::PathMeasure(Cairo::RefPtr<cdroid::Path>inPath,bool){
    mPath=inPath;
}

void PathMeasure::setPath(Cairo::RefPtr<cdroid::Path>inPath){
    mPath=inPath;
}

double PathMeasure::distance(const PointD& p1, const PointD& p2) {
    const double dx = p2.x - p1.x;
    const double dy = p2.y - p1.y;
    return std::sqrt(dx*dx+dy*dy);
}

double PathMeasure::curveLength(const PointD& p0, const PointD& p1, const PointD& p2, const PointD& p3) {
    // Approximate the length of a cubic Bezier curve using a simple method
    double length = 0.0;
    PointD prev = p0;
    const int steps = 36;
    for (int i = 1; i <= steps; ++i) {
        double t = static_cast<double>(i) / steps;
        PointD point = interpolateCurve(p0, p1, p2, p3, t);
        length += distance(prev, point);
        prev = point;
    }
    return length;
}

PointD PathMeasure::interpolate(const PointD& p1, const PointD& p2, double t) {
    return { p1.x + t * (p2.x - p1.x), p1.y + t * (p2.y - p1.y) };
}

PointD PathMeasure::interpolateCurve(const PointD& p0, const PointD& p1, const PointD& p2, const PointD& p3, double t) {
    double u = 1.0 - t;
    double tt = t * t;
    double uu = u * u;
    double uuu = uu * u;
    double ttt = tt * t;

    PointD p = {
        uuu * p0.x + 3.0 * uu * t * p1.x + 3.0 * u * tt * p2.x + ttt * p3.x,
        uuu * p0.y + 3.0 * uu * t * p1.y + 3.0 * u * tt * p2.y + ttt * p3.y
    };
    return p;
}

void PathMeasure::bezierSplitSingle(const PointD& p0, const PointD& p1, const PointD& p2, const PointD& p3, double t,
        PointD& left0, PointD& left1, PointD& left2, PointD& left3,
        PointD& right0, PointD& right1, PointD& right2, PointD& right3){
    PointD q11 = interpolate(p0, p1, t);
    PointD q12 = interpolate(p1, p2, t);
    PointD q13 = interpolate(p2, p3, t);

    PointD q21 = interpolate(q11, q12, t);
    PointD q22 = interpolate(q12, q13, t);

    PointD q31 = interpolate(q21, q22, t);

    // 左半段
    left0 = p0;
    left1 = q11;
    left2 = q21;
    left3 = q31;

    // 右半段
    right0 = q31;
    right1 = q22;
    right2 = q13;
    right3 = p3;
}
void PathMeasure::bezierSplit(const PointD& p0, const PointD& p1, const PointD& p2, const PointD& p3,
                 double t0, double t1, PointD& q0, PointD& q1, PointD& q2, PointD& q3)
{
    // 先分割到 t1，得到 [0, t1] 的控制点
    PointD l0, l1, l2, l3, r0, r1, r2, r3;
    double t1_rel = t1;
    bezierSplitSingle(p0, p1, p2, p3, t1_rel, l0, l1, l2, l3, r0, r1, r2, r3);

    // 再在左半段分割到 t0/t1
    double t0_rel = t0 / t1;
    bezierSplitSingle(l0, l1, l2, l3, t0_rel, q0, q1, q2, q3, r0, r1, r2, r3);
}
#define CURVE_SKIP_POINTS 4
double PathMeasure::getLength() {
    PointD first_point,last_point;
    double length =0;
    auto m_path=mPath->copy_path();
    for (int i = 0; i < m_path->num_data; ) {
        cairo_path_data_t* data = &m_path->data[i];
        switch (data->header.type) {
        case CAIRO_PATH_MOVE_TO:
            last_point = { data[1].point.x, data[1].point.y };
            first_point=last_point;
            i += 2;
            break;
        case CAIRO_PATH_LINE_TO: {
            PointD current_point = { data[1].point.x, data[1].point.y };
            length += distance(last_point, current_point);
            last_point = current_point;
            i += 2;
            break;
        }
        case CAIRO_PATH_CURVE_TO: {
            PointD p1 = { data[1].point.x, data[1].point.y };
            PointD p2 = { data[2].point.x, data[2].point.y };
            PointD p3 = { data[3].point.x, data[3].point.y };
            length += curveLength(last_point, p1, p2, p3);
            last_point = p3;
            i += CURVE_SKIP_POINTS;
            break;
        }
        case CAIRO_PATH_CLOSE_PATH:
            length += distance(last_point,first_point);
            i += 1;
            break;
        default:    break;
        }
    }
    cairo_path_destroy(m_path);
    return length;
}

int PathMeasure::buildSegments(std::vector<PathMeasure::Segment>&segs,std::vector<double>& accumulatedLen){
    accumulatedLen.clear();
    segs.clear();
    double x0 = 0, y0 = 0;          // 当前点
    double startX = 0, startY = 0; // 最近 moveTo 点

    auto m_path = mPath->copy_path();
    int  i = 0;

    while (i < m_path->num_data) {
        cairo_path_data_t* data = &m_path->data[i];
        const int type = data->header.type;
        const int n    = data->header.length;
        i += n;

        if (type == CAIRO_PATH_MOVE_TO) {
            startX = x0 = data[1].point.x;
            startY = y0 = data[1].point.y;
        } else if (type == CAIRO_PATH_LINE_TO) {
            double x1 = data[1].point.x;
            double y1 = data[1].point.y;
            double len = std::hypot(x1 - x0, y1 - y0);
            segs.push_back({Segment::Line, {x0, y0}, {x1, y1}, {}, {}, len});
            if(accumulatedLen.empty())
                accumulatedLen.push_back(len);
            else accumulatedLen.push_back(accumulatedLen.back() + len);
            x0 = x1; y0 = y1;
        } else if (type == CAIRO_PATH_CURVE_TO) {
            double x1 = data[1].point.x, y1 = data[1].point.y;
            double x2 = data[2].point.x, y2 = data[2].point.y;
            double x3 = data[3].point.x, y3 = data[3].point.y;
            /* 用任意快速弧长估算，这里直接采样 16 段 */
            double len = 0;
            PointD prev = {x0, y0};
            for (int k = 1; k <= 16; ++k) {
                double t = k / 16.0;
                double mt = 1.0-t;
                double x = mt*mt*mt*x0 + 3*mt*mt*t*x1 + 3*mt*t*t*x2 + t*t*t*x3;
                double y = mt*mt*mt*y0 + 3*mt*mt*t*y1 + 3*mt*t*t*y2 + t*t*t*y3;

                PointD pt = {x,y};
                len += std::hypot(pt.x - prev.x, pt.y - prev.y);
                prev = pt;
            }
            segs.push_back({Segment::Cubic, {x0, y0}, {x1, y1}, {x2, y2}, {x3, y3}, len});
            if(accumulatedLen.empty())accumulatedLen.push_back(len);
            else accumulatedLen.push_back(accumulatedLen.back() + len);
            x0 = x3; y0 = y3;
        }  else if (type == CAIRO_PATH_CLOSE_PATH) {
            double len = std::hypot(startX - x0, startY - y0);
            if (len > 0) {
                segs.push_back({Segment::Line, {x0, y0}, {startX, startY}, {}, {}, len});
                accumulatedLen.push_back(accumulatedLen.back() + len);
            }
            x0 = startX; y0 = startY;
        }
    }
    for(int i=0;i<segs.size();i++){
        LOGV("SEG[%d] type=%d len=%f p0=(%.2f,%.2f) p1=(%.2f,%.2f) p2=(%.2f,%.2f) p3=(%.2f,%.2f)",
            i, segs[i].type, segs[i].len,
            segs[i].p0.x, segs[i].p0.y, segs[i].p1.x, segs[i].p1.y,
            segs[i].p2.x, segs[i].p2.y, segs[i].p3.x, segs[i].p3.y);
    }
    return segs.size();
}

bool PathMeasure::getSegment(double startD, double stopD,Cairo::RefPtr<cdroid::Path>& dst, bool startWithMoveTo)
{
    if (!dst) dst = std::make_shared<cdroid::Path>();
    else dst->reset();

    if (startD < 0) startD = 0;
    if (stopD>1.0) stopD = 1.0;
    if (stopD  < startD) return false;

    /* 1. 打散成 Segment（只算一次，可缓存） */
    std::vector<Segment> segs;
    std::vector<double>  accLen;
    buildSegments(segs, accLen);

    const double total = accLen.back();
    startD*=total;
    stopD*=total;
    LOGV("nsegs=%d/%d totallen=%f",segs.size(),accLen.size(),total);
    if (stopD > total) stopD = total;

    bool needsMove = startWithMoveTo;
    for (int i = 0; i < segs.size(); ++i) {
        const Segment& s = segs[i];
        const double segStart = (i==0)?0:accLen[i-1];
        const double segEnd   = accLen[i];
        const double t0 = (startD >segStart) ? (startD - segStart) / s.len : 0.0;
        const double t1 = (stopD < segEnd) ? (stopD  - segStart) / s.len : 1.0;
        if (s.type == Segment::Line) {
            PointD p0 =interpolate(s.p0, s.p1, t0);
            PointD p1 = interpolate(s.p0, s.p1, t1);
            if (needsMove) { 
                dst->move_to(p0.x, p0.y); 
                needsMove = false; 
                LOGV("MOVE(%.2f,%.2f)",p0.x, p0.y);
            }
            dst->line_to(p1.x, p1.y);
            LOGV("LINE(%.2f,%.2f)",p1.x, p1.y);
        } else { // Cubic
            PointD q0, q1, q2, q3;
            if ((t0 == 0.0) && (t1 == 1.0)) {
                q0 = s.p0; q1 = s.p1; q2 = s.p2; q3 = s.p3;
            } else if(t1>t0+1e-6){
                bezierSplit(s.p0, s.p1, s.p2, s.p3, t0, t1, q0, q1, q2, q3);
            }
            if (needsMove) { 
                dst->move_to(q0.x, q0.y); 
                needsMove = false;
            }
            //dst->move_to(q0.x, q0.y);
            dst->curve_to(q1.x, q1.y, q2.x, q2.y, q3.x, q3.y);
            LOGV("[%d]CUBIC(%.2f,%.2f, %.2f,%.2f, %.2f,%.2f, %.2f,%.2f), seg( %.2f,%.2f)",i,q0.x,q0.y,q1.x, q1.y, q2.x, q2.y, q3.x, q3.y,segStart,segEnd);
        }
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
        switch (data->header.type) {
        case CAIRO_PATH_MOVE_TO:
            prev = { data[1].point.x, data[1].point.y };
            i += 2;
            break;
        case CAIRO_PATH_LINE_TO: {
            curr = { data[1].point.x, data[1].point.y };
            double segLen = this->distance(prev, curr);
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
            i += 2;
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
            i += CURVE_SKIP_POINTS;
            break;
        }
        case CAIRO_PATH_CLOSE_PATH:
            // 可选：处理闭合段
            i += 1;
            break;
        default:
            i += 1;
            break;
        }
    }
    cairo_path_destroy(m_path);
    return found;
}
}/*endof namespace*/
