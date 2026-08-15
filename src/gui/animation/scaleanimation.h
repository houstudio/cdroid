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
#ifndef __SCALEANIMATION_H__
#define __SCALEANIMATION_H__
#include <animation/animation.h>
#include <core/typedvalue.h>

namespace cdroid{
class ScaleAnimation:public Animation{
private :
    float mFromX;
    float mToX;
    float mFromY;
    float mToY;

    int mFromXType= TypedValue::TYPE_NULL;
    int mToXType  = TypedValue::TYPE_NULL;
    int mFromYType= TypedValue::TYPE_NULL;
    int mToYType  = TypedValue::TYPE_NULL;

    int mFromXData = 0;
    int mToXData = 0;
    int mFromYData = 0;
    int mToYData = 0;

    int mPivotXType = ABSOLUTE;
    int mPivotYType = ABSOLUTE;
    float mPivotXValue = 0.0f;
    float mPivotYValue = 0.0f;

    float mPivotX;
    float mPivotY;
private:
    void initializePivotPoint();
protected:
    ScaleAnimation(const ScaleAnimation&);
    void applyTransformation(float interpolatedTime, Transformation& t)override;
    float resolveScale(float scale, int type, int data, int size, int psize);
public:
    ScaleAnimation(Context* context,const AttributeSet& attrs);
    ScaleAnimation(float fromX, float toX, float fromY, float toY);
    ScaleAnimation(float fromX, float toX, float fromY, float toY,
            float pivotX, float pivotY);
    ScaleAnimation(float fromX, float toX, float fromY, float toY,
            int pivotXType, float pivotXValue, int pivotYType, float pivotYValue);
    void initialize(int width, int height, int parentWidth, int parentHeight)override;
    ScaleAnimation*clone()const override;
};

}

#endif
