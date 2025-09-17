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
#ifndef __HWUI_VECTORDRAWABLE_UTILS_H__
#define __HWUI_VECTORDRAWABLE_UTILS_H__
#include <core/path.h>
#include <cairomm/context.h>
#include <drawables/hwvectordrawable.h>
namespace cdroid {
namespace hwui {

class VectorDrawableUtils {
public:
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
