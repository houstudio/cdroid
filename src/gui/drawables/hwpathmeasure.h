#ifndef __PATH_MEASURE_H__
#define __PATH_MEASURE_H__
#include <core/path.h>
#include <cairomm/context.h>
#include <drawables/hwvectordrawable.h>
namespace cdroid {
namespace hw{
class PathMeasure{
private:
    using Point =CPoint<double>;
    Cairo::RefPtr<cdroid::Path>mPath;
    double distance(const Point&p1,const Point&p2);
    double curveLength(const Point& p0, const Point& p1, const Point& p2, const Point& p3);
    Point interpolate(const Point& p1, const Point& p2, double t);
    Point interpolateCurve(const Point& p0, const Point& p1, const Point& p2, const Point& p3, double t);
public:
    PathMeasure(Cairo::RefPtr<cdroid::Path>inPath,bool);
    double getLength();
    bool getSegment(double startD, double stopD, Cairo::RefPtr<cdroid::Path>& dst, bool startWithMoveTo);
};
}
}
#endif
