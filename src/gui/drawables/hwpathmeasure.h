#ifndef __PATH_MEASURE_H__
#define __PATH_MEASURE_H__
#include <core/path.h>
#include <cairomm/context.h>
#include <drawables/hwvectordrawable.h>
namespace cdroid {
namespace hw{
class PathMeasure{
public:
    PathMeasure(Cairo::RefPtr<cdroid::Path>inPath,bool);
    double getLength();
    bool getSegment(double startD, double stopD, Cairo::RefPtr<cdroid::Path>& dst, bool startWithMoveTo);
};
}
}
#endif
