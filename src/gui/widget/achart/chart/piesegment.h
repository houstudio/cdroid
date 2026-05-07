/**
 * Copyright (C) 2009 - 2012 SC 4ViewSoft SRL
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __ACHART_PIE_SEGMENT_H__
#define __ACHART_PIE_SEGMENT_H__
#include <core/rect.h>
namespace cdroid{
/**
 * Holds An PieChart Segment
 */
class PieSegment{
private:
    float mStartAngle;
    float mEndAngle;
    float mValue;
    int mDataIndex;
public:
    PieSegment()=default;
    PieSegment(PieSegment&& other)=default;
    PieSegment(const PieSegment& other) = default;
    PieSegment(int dataIndex, float value, float startAngle, float angle) {
        mStartAngle = startAngle;
        mEndAngle = angle + startAngle;
        mDataIndex = dataIndex;
        mValue = value;
    }

    bool isInSegment(double angle) const{
        if (angle >= mStartAngle && angle <= mEndAngle) {
            return true;
        }
        double cAngle = std::fmod(angle ,360);
        double startAngle = mStartAngle;
        double stopAngle = mEndAngle;
        while (stopAngle > 360) {
            startAngle -= 360;
            stopAngle -= 360;
        }
        return cAngle >= startAngle && cAngle <= stopAngle;
    }

    float getStartAngle() const{
        return mStartAngle;
    }

    float getEndAngle() const{
        return mEndAngle;
    }

    int getDataIndex() const{
        return mDataIndex;
    }

    float getValue() const{
        return mValue;
    }
};
}/*endof namespace*/
#endif/*__ACHART_PIE_SEGMENT_H__*/
