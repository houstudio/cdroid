/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.PatternPathMotion.
 *********************************************************************************/
#include <transition/patternpathmotion.h>

#include <cmath>
#include <stdexcept>

#include <core/attributeset.h>
#include <core/context.h>
#include <core/pathmeasure.h>
#include <drawable/pathparser.h>

namespace cdroid {

PatternPathMotion::PatternPathMotion() {
    mPatternPath.lineTo(1, 0);
    mOriginalPatternPath = mPatternPath;
}

PatternPathMotion::PatternPathMotion(Context* context, AttributeSet* attrs)
    : PathMotion(context, attrs) {
    if (attrs != nullptr) {
        std::string pathData = attrs->getAttributeValue("patternPathData");
        if (pathData.empty()) {
            throw std::runtime_error("pathData must be supplied for patternPathMotion");
        }
        auto parsed = PathParser::createPathFromPathData(pathData); // shared_ptr<Path>
        if (parsed) {
            setPatternPath(*parsed);
        }
    }
}

PatternPathMotion::PatternPathMotion(const Path& patternPath) {
    setPatternPath(patternPath);
}

void PatternPathMotion::setPatternPath(const Path& patternPath) {
    Cairo::RefPtr<Path> ref(new Path(patternPath));
    PathMeasure pathMeasure(ref, false);
    double length = pathMeasure.getLength();
    double pos[2] = {0, 0};
    pathMeasure.getPosTan(length, pos, nullptr);
    float endX = (float)pos[0];
    float endY = (float)pos[1];
    pathMeasure.getPosTan(0, pos, nullptr);
    float startX = (float)pos[0];
    float startY = (float)pos[1];

    if (startX == endX && startY == endY) {
        throw std::invalid_argument("pattern must not end at the starting point");
    }

    // setTranslate(-startX,-startY); postScale(scale,scale); postRotate(-angle).
    // cairo translate/scale/rotate post-multiply from identity == android setX then postX.
    mTempMatrix = Cairo::Matrix(); // identity
    mTempMatrix.translate(-startX, -startY);
    float dx = endX - startX;
    float dy = endY - startY;
    float distance = (float)std::hypot(dx, dy);
    float scale = 1 / distance;
    mTempMatrix.scale(scale, scale);
    double angle = std::atan2(dy, dx);
    mTempMatrix.rotate(-angle);
    Path src = patternPath; // const param → mutable copy (Path::transform is non-const)
    src.transform(mTempMatrix, mPatternPath);
    mOriginalPatternPath = patternPath;
}

Path PatternPathMotion::getPath(float startX, float startY, float endX, float endY) {
    double dx = endX - startX;
    double dy = endY - startY;
    float length = (float)std::hypot(dx, dy);
    double angle = std::atan2(dy, dx);

    // setScale(length,length); postRotate(angle); postTranslate(startX,startY).
    mTempMatrix = Cairo::Matrix(); // identity
    mTempMatrix.scale(length, length);
    mTempMatrix.rotate(angle);
    mTempMatrix.translate(startX, startY);
    Path path;
    mPatternPath.transform(mTempMatrix, path);
    return path;
}

} // namespace cdroid
