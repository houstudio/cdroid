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
#include <drawable/pathparser.h>
#include <drawable/hwpathparser.h>
#include <drawable/hwvectordrawable.h>
#include <porting/cdlog.h>
namespace cdroid{

/**
 * @param pathString The string representing a path, the same as "d" string in svg file.
 * @return the generated Path object.
 */
std::shared_ptr<cdroid::Path> PathParser::createPathFromPathData(const std::string& pathString) {
    if (pathString.empty()) {
        LOGE("Path string can not be null.");
    }
    //Path path = new Path();
    //nParseStringForPath(path.mNativePath, pathString, pathString.length());
    hwui::PathParser::ParseResult result;
    Cairo::RefPtr<cdroid::Path> path =std::make_shared<cdroid::Path>();
    hwui::PathParser::parseAsciiStringForPath(path, &result, pathString.c_str(), pathString.length());
    return path;
}

/**
 * Interpret PathData as path commands and insert the commands to the given path.
 *
 * @param data The source PathData to be converted.
 * @param outPath The Path object where path commands will be inserted.
 */
void PathParser::createPathFromPathData(std::shared_ptr<cdroid::Path>& outPath,const PathParser::PathData& data) {
    //nCreatePathFromPathData(outPath.mNativePath, data.mNativePathData);
    hwui::VectorDrawableUtils::verbsToPath(outPath, *(hwui::PathData*)data.mNativePathData);
}

/**
 * @param pathDataFrom The source path represented in PathData
 * @param pathDataTo The target path represented in PathData
 * @return whether the <code>nodesFrom</code> can morph into <code>nodesTo</code>
 */
bool PathParser::canMorph(const PathParser::PathData& pathDataFrom,const PathParser::PathData& pathDataTo) {
    //return nCanMorph(pathDataFrom.mNativePathData, pathDataTo.mNativePathData);
    hwui::PathData*from = (hwui::PathData*)pathDataFrom.mNativePathData;
    hwui::PathData*to = (hwui::PathData*)pathDataTo.mNativePathData;
    return hwui::VectorDrawableUtils::canMorph(*from,*to);
}

/**
 * Interpolate between the <code>fromData</code> and <code>toData</code> according to the
 * <code>fraction</code>, and put the resulting path data into <code>outData</code>.
 *
 * @param outData The resulting PathData of the interpolation
 * @param fromData The start value as a PathData.
 * @param toData The end value as a PathData
 * @param fraction The fraction to interpolate.
 */
bool PathParser::interpolatePathData(PathParser::PathData& outData,const PathParser::PathData& fromData,const PathParser::PathData& toData,
        float fraction) {
    hwui::PathData*out = (hwui::PathData*)outData.mNativePathData;
    hwui::PathData*from= (hwui::PathData*)fromData.mNativePathData;
    hwui::PathData*to = (hwui::PathData*)toData.mNativePathData;
    return hwui::VectorDrawableUtils::interpolatePathData(*out,*from,*to,fraction);
}

/**
 * PathData class is a wrapper around the native PathData object, which contains
 * the result of parsing a path string. Specifically, there are verbs and points
 * associated with each verb stored in PathData. This data can then be used to
 * generate commands to manipulate a Path.
 */
PathParser::PathData::PathData() {
    mNativePathData = new hwui::PathData();
}

PathParser::PathData::PathData(const PathParser::PathData& data) {
    mNativePathData = new hwui::PathData(*(hwui::PathData*)data.mNativePathData);
}

PathParser::PathData::PathData(const std::string& pathString) {
    if (pathString.empty()) {
        LOGE("Invalid pathData: %s",pathString.c_str());
    }
    mNativePathData = new hwui::PathData();
    hwui::PathParser::ParseResult result;
    hwui::PathParser::getPathDataFromAsciiString((hwui::PathData*)mNativePathData, &result, pathString.c_str(), pathString.length());
}

long PathParser::PathData::getNativePtr() {
    return (long)mNativePathData;
}

/**
 * Update the path data to match the source.
 * Before calling this, make sure canMorph(target, source) is true.
 *
 * @param source The source path represented in PathData
 */
void PathParser::PathData::setPathData(const PathParser::PathData& source) {
    hwui::PathData*thiz =(hwui::PathData*)mNativePathData;
    hwui::PathData*other=(hwui::PathData*)source.mNativePathData;
    *thiz = *other;
}

PathParser::PathData&PathParser::PathData::operator=(const PathParser::PathData&other){
    if(&other!=this){
        *(hwui::PathData*)mNativePathData=*(hwui::PathData*)other.mNativePathData;
    }
    return *this;
}

PathParser::PathData::~PathData() {
    if (mNativePathData != nullptr) {
        delete (hwui::PathData*)mNativePathData;
        mNativePathData = nullptr;
    }
}

}
