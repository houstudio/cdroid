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

bool PathMeasure::getSegment(double startD, double stopD, Cairo::RefPtr<cdroid::Path>& dst, bool startWithMoveTo) {
    double length = 0.0,segment_length;
    PointD p1 , p2 , p3;
    PointD first_point,last_point,current_point;
    bool segment_started = false,segment_found = false;
    auto m_path = mPath->copy_path();
    for (int i = 0; i < m_path->num_data && (length<stopD); ) {
        cairo_path_data_t* data = &m_path->data[i];
        switch (data->header.type) {
        case CAIRO_PATH_MOVE_TO:
            last_point = { data[1].point.x, data[1].point.y };
            first_point = last_point;
            if (startWithMoveTo && !segment_started && length >= startD) {
                dst->move_to(last_point.x, last_point.y);
                segment_started = true;
            }
            i += 2;
            break;
        case CAIRO_PATH_LINE_TO:
            current_point = { data[1].point.x, data[1].point.y };
            segment_length = distance(last_point, current_point);
            if (length + segment_length >= startD && length <= stopD) {
                if (!segment_started) {
                    PointD p =interpolate(last_point, current_point, (startD - length) / segment_length);
                    dst->move_to(p.x,p.y);
                    segment_started = true;
                }
                if (length + segment_length > stopD) {
                    PointD p = interpolate(last_point, current_point, (stopD - length) / segment_length);
                    dst->line_to(p.x,p.y);
                    current_point = p;
                    segment_found = true ;
                } else {
                    dst->line_to(current_point.x, current_point.y);
                }
            }
            length += segment_length;
            last_point = current_point;
            i += 2;
            break;
        case CAIRO_PATH_CURVE_TO:
            p1 = { data[1].point.x, data[1].point.y };
            p2 = { data[2].point.x, data[2].point.y };
            p3 = { data[3].point.x, data[3].point.y };
            segment_length = curveLength(last_point, p1, p2, p3);
            if (length + segment_length >= startD && length <= stopD) {
                if (!segment_started) {
                    PointD p = interpolateCurve(last_point, p1, p2, p3, (startD - length)/segment_length);
                    dst->move_to(p.x,p.y);
                    LOGV("(%f,%f),(%f,%f),(%f,%f),(%f,%f),(%f,%f),(%f,%f)",last_point.x,last_point.y,p1.x, p1.y, p2.x, p2.y,p3.x,p3.y,p.x,p.y);
                    segment_started = true;
                }
                if (length + segment_length > stopD) {
                    PointD p = interpolateCurve(last_point, p1, p2, p3, (stopD - length) / segment_length);
                    dst->curve_to(p1.x, p1.y, p2.x, p2.y,p.x,p.y);
                    segment_found = true;
                    LOGV("(%f,%f),(%f,%f),(%f,%f),(%f,%f),(%f,%f)",p1.x, p1.y, p2.x, p2.y,p3.x,p3.y,p.x,p.y);
                } else {
                    dst->curve_to(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
                }
            }
            length += segment_length;
            LOGV("segment[%d/%d]=%f/%f",i,m_path->num_data,stopD-startD,length);
            i += CURVE_SKIP_POINTS;
            last_point = p3;
            break;
        case CAIRO_PATH_CLOSE_PATH:
            segment_length = distance(last_point, first_point);
            if (length + segment_length >= startD && length <= stopD) {
                if (!segment_started) {
                    PointD p = interpolate(last_point, first_point, (startD - length) / segment_length);
                    dst->move_to(p.x,p.y);
                    segment_started = true;
                }
                if (length + segment_length > stopD) {
                    PointD p = interpolate(last_point, first_point, (stopD - length) / segment_length);
                    dst->line_to(p.x,p.y);
                    segment_found = true;
                } else {
                    dst->line_to(first_point.x, first_point.y);
                }
            }
            length += segment_length;
            last_point = first_point;
            i += 1;
            break;
        default:  break;
        }
    }
    cairo_path_destroy(m_path);
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
