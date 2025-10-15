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
#include <drawable/hwvectordrawable.h>
namespace cdroid {
class PathMeasure{
public:
    struct Segment {
        enum Type { Line, Cubic } type;
        int ptIndex;
        double distance;
    };
private:
    bool mForceClosed;
    std::vector<PointD>mPoints;
    std::vector<Segment>mSegments;
    Cairo::RefPtr<cdroid::Path>mPath;
    int buildSegments();
public:
    PathMeasure();
    PathMeasure(Cairo::RefPtr<cdroid::Path>inPath,bool forceClosed);
    void setPath(Cairo::RefPtr<cdroid::Path>inPath);
    void setPath(Cairo::RefPtr<cdroid::Path>inPath,bool forceClosed);
    double getLength()const;
    bool getSegment(double startD, double stopD, Cairo::RefPtr<cdroid::Path>& dst, bool startWithMoveTo);
    bool getPosTan(double distance,PointD* pos,PointD* tangent) ;
};
}
#endif
