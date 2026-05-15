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
    static constexpr int POSITION_MATRIX_FLAG = 0x01;    // must match flags in SkPathMeasure.h
    static constexpr int TANGENT_MATRIX_FLAG  = 0x02;
private:
    struct Segment {
        enum Type { Line, Cubic };
        int32_t type:2;
        int32_t ptIndex:30;
        double distance;
    };
    struct Contour {
        size_t segmentStart;
        size_t segmentEnd;
        double length;
        bool isClosed;
    };
private:
    bool mForceClosed;
    size_t mContourIndex;
    std::vector<PointD>mPoints;
    std::vector<Segment>mSegments;
    std::vector<Contour>mContours;
    Cairo::RefPtr<cdroid::Path>mPath;
    int buildSegments();
public:
    PathMeasure();
    PathMeasure(const Cairo::RefPtr<cdroid::Path>& inPath,bool forceClosed);
    void setPath(const Cairo::RefPtr<cdroid::Path>& inPath,bool forceClosed);
    double getLength()const;
    bool getPosTan(double distance,double* pos,double* tangent);
    bool getMatrix(double distance, Cairo::Matrix& matrix, int flags);
    bool getSegment(double startD, double stopD, Cairo::RefPtr<cdroid::Path>& dst, bool startWithMoveTo);
    bool isClosed() const;
    bool nextContour();
};
}
#endif
