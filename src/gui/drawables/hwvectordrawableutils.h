#ifndef __HWUI_VECTORDRAWABLE_UTILS_H__
#define __HWUI_VECTORDRAWABLE_UTILS_H__
#include <core/path.h>
#include <cairomm/context.h>
#include <drawables/hwvectordrawable.h>
namespace cdroid {
namespace hwui {

class VectorDrawableUtils {
public:
    static double PathMeasure(const Cairo::RefPtr<cdroid::Path>&path);
    static bool canMorph(const PathData& morphFrom, const PathData& morphTo);
    static bool interpolatePathData(PathData& outData, const PathData& morphFrom,
                                                const PathData& morphTo, float fraction);
    static void verbsToPath(Cairo::RefPtr<cdroid::Path>& outPath, const PathData& data);
    static void interpolatePaths(PathData& outPathData, const PathData& from, const PathData& to,
                                 float fraction);
};
}  /*namespace hwui*/
}  /*namespace cddroid*/
#endif /*__HWUI_VECTORDRAWABLE_UTILS_H__*/
