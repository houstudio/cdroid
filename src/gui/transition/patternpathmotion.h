/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.PatternPathMotion.
 *
 * Matrix note: android uses android.graphics.Matrix. CDROID uses Cairo::Matrix directly.
 * cairo's translate/scale/rotate are post-multiply (m = m * op); starting from identity they
 * exactly reproduce android's setX (m = I * X = X) followed by postX (m = m * X) sequence.
 *********************************************************************************/
#ifndef __CDROID_TRANSITION_PATTERNPATHMOTION_H__
#define __CDROID_TRANSITION_PATTERNPATHMOTION_H__

#include <cairomm/matrix.h>
#include <core/path.h>

#include <transition/pathmotion.h>

namespace cdroid{

class Context;
class AttributeSet;

/**
 * A PathMotion that takes a Path pattern and translates/rotates/scales it to fit between two
 * points. Ported from android-36 android.transition.PatternPathMotion.
 */
class PatternPathMotion: public PathMotion{
public:
    PatternPathMotion();
    PatternPathMotion(Context* context, AttributeSet* attrs);
    explicit PatternPathMotion(const Path& patternPath);

    Path getPatternPath() const{ return mOriginalPatternPath; }
    void setPatternPath(const Path& patternPath);

    Path getPath(float startX, float startY, float endX, float endY) override;

private:
    Path mOriginalPatternPath;
    Path mPatternPath;
    Cairo::Matrix mTempMatrix;
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_PATTERNPATHMOTION_H__
