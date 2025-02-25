#include <drawables/pathparser.h>
#include <drawables/hwpathparser.h>
#include <drawables/hwvectordrawable.h>
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
    hw::PathParser::ParseResult result;
    Cairo::RefPtr<cdroid::Path>path;
    hw::PathParser::parseAsciiStringForPath(path, &result, pathString.c_str(), pathString.length());
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
    hw::PathData* pd=(hw::PathData*)data.mNativePathData;
    hw::VectorDrawableUtils::verbsToPath(outPath, *pd);
}

/**
 * @param pathDataFrom The source path represented in PathData
 * @param pathDataTo The target path represented in PathData
 * @return whether the <code>nodesFrom</code> can morph into <code>nodesTo</code>
 */
bool PathParser::canMorph(const PathParser::PathData& pathDataFrom,const PathParser::PathData& pathDataTo) {
    //return nCanMorph(pathDataFrom.mNativePathData, pathDataTo.mNativePathData);
    hw::PathData*from=(hw::PathData*)pathDataFrom.mNativePathData;
    hw::PathData*to =(hw::PathData*)pathDataTo.mNativePathData;
    return hw::VectorDrawableUtils::canMorph(*from,*to);
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
    hw::PathData*out=(hw::PathData*)outData.mNativePathData;
    hw::PathData*from=(hw::PathData*)fromData.mNativePathData;
    hw::PathData*to=(hw::PathData*)toData.mNativePathData;
    return hw::VectorDrawableUtils::interpolatePathData(*out,*from,*to,fraction);
}

/**
 * PathData class is a wrapper around the native PathData object, which contains
 * the result of parsing a path string. Specifically, there are verbs and points
 * associated with each verb stored in PathData. This data can then be used to
 * generate commands to manipulate a Path.
 */
PathParser::PathData::PathData() {
    mNativePathData = new hw::PathData();
}

PathParser::PathData::PathData(const PathParser::PathData& data) {
    mNativePathData = new hw::PathData(*data.mNativePathData);
}

PathParser::PathData::PathData(const std::string& pathString) {
    if (pathString.empty()) {
        LOGE("Invalid pathData: %s",pathString.c_str());
    }
    hw::PathData*pd = new hw::PathData();
    hw::PathParser::ParseResult result;
    hw::PathParser::getPathDataFromAsciiString(pd, &result, pathString.c_str(), pathString.length());

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
    hw::PathData*thiz =(hw::PathData*)mNativePathData;
    hw::PathData*other=(hw::PathData*)source.mNativePathData;
    *thiz = *other;
}

PathParser::PathData::~PathData() {
    if (mNativePathData != nullptr) {
        delete (hw::PathData*)mNativePathData;
        mNativePathData = nullptr;
    }
}

}
