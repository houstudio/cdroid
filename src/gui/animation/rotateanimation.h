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
#ifndef __ROTATE_ANIMATION_H__
#define __ROTATE_ANIMATION_H__

#include <animation/animation.h>
namespace cdroid{

class RotateAnimation:public Animation{
private:
    float mFromDegrees;
    float mToDegrees;

    int mPivotXType = ABSOLUTE;
    int mPivotYType = ABSOLUTE;
    float mPivotXValue = 0.0f;
    float mPivotYValue = 0.0f;

    float mPivotX;
    float mPivotY;
private:
    void initializePivotPoint();
protected:
    RotateAnimation(const RotateAnimation&);
    void applyTransformation(float interpolatedTime, Transformation& t)override;
public:
    RotateAnimation(Context* context,const AttributeSet& attrs);
    RotateAnimation(float fromDegrees, float toDegrees);
    RotateAnimation(float fromDegrees, float toDegrees, float pivotX, float pivotY);
    RotateAnimation(float fromDegrees, float toDegrees, int pivotXType, float pivotXValue,
            int pivotYType, float pivotYValue);
    void initialize(int width, int height, int parentWidth, int parentHeight)override;
    RotateAnimation*clone()const override;
};

}/*endof namespace*/
#endif/*__ROTATE_ANIMATION_H__*/
