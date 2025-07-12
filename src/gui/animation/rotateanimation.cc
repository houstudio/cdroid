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
#include <animation/rotateanimation.h>
namespace cdroid{

RotateAnimation::RotateAnimation(const RotateAnimation&o){
    mFromDegrees = o.mFromDegrees;
    mToDegrees   = o.mToDegrees;

    mPivotXValue = o.mPivotXValue;
    mPivotXType  = o.mPivotXType;
    mPivotYValue = o.mPivotYValue;
    mPivotYType  = o.mPivotYType;
    initializePivotPoint();
}

RotateAnimation::RotateAnimation(Context* context,const AttributeSet& attrs){
    mFromDegrees = attrs.getFloat("fromDegrees", 0.0f);
    mToDegrees   = attrs.getFloat("toDegrees", 0.0f);

    Description d = Description::parseValue(attrs.getString("pivotX"));
    mPivotXType = d.type;
    mPivotXValue = d.value;

    d = Description::parseValue(attrs.getString("pivotY"));
    mPivotYType = d.type;
    mPivotYValue = d.value;

    initializePivotPoint();
}

RotateAnimation::RotateAnimation(float fromDegrees, float toDegrees){
    mFromDegrees= fromDegrees;
    mToDegrees  = toDegrees;
    mPivotX = 0.0f;
    mPivotY = 0.0f;
}

RotateAnimation::RotateAnimation(float fromDegrees, float toDegrees, float pivotX, float pivotY) {
    mFromDegrees= fromDegrees;
    mToDegrees  = toDegrees;

    mPivotXType = ABSOLUTE;
    mPivotYType = ABSOLUTE;
    mPivotXValue= pivotX;
    mPivotYValue= pivotY;
    initializePivotPoint();
}

RotateAnimation::RotateAnimation(float fromDegrees, float toDegrees, int pivotXType, float pivotXValue,
            int pivotYType, float pivotYValue){
    mFromDegrees= fromDegrees;
    mToDegrees  = toDegrees;

    mPivotXValue= pivotXValue;
    mPivotXType = pivotXType;
    mPivotYValue= pivotYValue;
    mPivotYType = pivotYType;
    initializePivotPoint();
}

RotateAnimation*RotateAnimation::clone()const{
    return new RotateAnimation(*this);
}

void RotateAnimation::initializePivotPoint() {
    if (mPivotXType == ABSOLUTE) {
        mPivotX = mPivotXValue;
    }
    if (mPivotYType == ABSOLUTE) {
        mPivotY = mPivotYValue;
    }
}

void RotateAnimation::applyTransformation(float interpolatedTime, Transformation& t){
    float degrees = mFromDegrees + ((mToDegrees - mFromDegrees) * interpolatedTime);
    float scale = getScaleFactor();
        
    if (mPivotX == 0.0f && mPivotY == 0.0f) {
        t.getMatrix().rotate(degrees);
    } else {
        //t.getMatrix().rotate(degrees, mPivotX * scale, mPivotY * scale);
        t.getMatrix().translate(mPivotX * scale, mPivotY * scale);
        t.getMatrix().rotate(degrees); 
        t.getMatrix().translate(-mPivotX * scale, -mPivotY * scale);
    }
}

void RotateAnimation::initialize(int width, int height, int parentWidth, int parentHeight){
    Animation::initialize(width, height, parentWidth, parentHeight);
    mPivotX = resolveSize(mPivotXType, mPivotXValue, width, parentWidth);
    mPivotY = resolveSize(mPivotYType, mPivotYValue, height, parentHeight);
}

}
