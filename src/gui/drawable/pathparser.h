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
#ifndef __PARTH_PARSER_H__
#define __PARTH_PARSER_H__
#include <core/path.h>
namespace cdroid{
class PathParser {
public:
    /**
     * PathData class is a wrapper around the native PathData object, which contains
     * the result of parsing a path string. Specifically, there are verbs and points
     * associated with each verb stored in PathData. This data can then be used to
     * generate commands to manipulate a Path.
     */
    class PathData {
    private:
        friend PathParser;
        void* mNativePathData;
    public:
        PathData();
        PathData(const PathData& data);
        PathData(const std::string& pathString);
        long getNativePtr();

        /**
         * Update the path data to match the source.
         * Before calling this, make sure canMorph(target, source) is true.
         *
         * @param source The source path represented in PathData
         */
        void setPathData(const PathData& source);
        PathData&operator=(const PathData&other);
        ~PathData();
    };

    /**
     * @param pathString The string representing a path, the same as "d" string in svg file.
     * @return the generated Path object.
     */
    static std::shared_ptr<cdroid::Path> createPathFromPathData(const std::string& pathString);

    /**
     * Interpret PathData as path commands and insert the commands to the given path.
     *
     * @param data The source PathData to be converted.
     * @param outPath The Path object where path commands will be inserted.
     */
    static void createPathFromPathData(std::shared_ptr<cdroid::Path>& outPath,const PathData& data);

    /**
     * @param pathDataFrom The source path represented in PathData
     * @param pathDataTo The target path represented in PathData
     * @return whether the <code>nodesFrom</code> can morph into <code>nodesTo</code>
     */
    static bool canMorph(const PathData& pathDataFrom,const PathData& pathDataTo);


    /**
     * Interpolate between the <code>fromData</code> and <code>toData</code> according to the
     * <code>fraction</code>, and put the resulting path data into <code>outData</code>.
     *
     * @param outData The resulting PathData of the interpolation
     * @param fromData The start value as a PathData.
     * @param toData The end value as a PathData
     * @param fraction The fraction to interpolate.
     */
    static bool interpolatePathData(PathData& outData,const PathData& fromData,const PathData& toData, float fraction);
};
}/*endof namespace*/
#endif/*__PARTH_PARSER_H__*/
