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
namespace hwui{
class PathMeasure{
private:
    using Point =CPoint<double>;
    double mTotalLength;
    Cairo::RefPtr<cdroid::Path>mPath;
    double distance(const Point&p1,const Point&p2);
    double curveLength(const Point& p0, const Point& p1, const Point& p2, const Point& p3);
    Point interpolate(const Point& p1, const Point& p2, double t);
    Point interpolateCurve(const Point& p0, const Point& p1, const Point& p2, const Point& p3, double t);
    double calculateTotalLength();
public:
    PathMeasure(Cairo::RefPtr<cdroid::Path>inPath,bool);
    double getLength();
    bool getSegment(double startD, double stopD, Cairo::RefPtr<cdroid::Path>& dst, bool startWithMoveTo);
};
}
}
#endif
