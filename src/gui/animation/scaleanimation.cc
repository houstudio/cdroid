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
#include <animation/scaleanimation.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <content/typedarray.h>
#include <content/typedvalue.h>
#include <widget/framework_styleable.h>
namespace cdroid{
using namespace cdroid::internal;

ScaleAnimation::ScaleAnimation(const ScaleAnimation&o):Animation(o){
    mFromX  = o.mFromX;
    mFromXType = o.mFromXType;
    mToX    = o.mToX;
    mToXType= o.mToXType;
    mFromY  = o.mFromY;
    mFromYType=o.mFromYType;
    mToY    = o.mToY;
    mToYType= o.mToYType;
    mPivotXValue = o.mPivotXValue;
    mPivotXType  = o.mPivotXType;
    mPivotYValue = o.mPivotYValue;
    mPivotYType  = o.mPivotYType;

    mFromXData = o.mFromXData;
    mFromYData = o.mFromYData;
    mToXData = o.mToXData;
    mToYData = o.mToYData;
    initializePivotPoint();
}

ScaleAnimation::ScaleAnimation(Context* context,const AttributeSet& attrs)
    :Animation(context,attrs){
    auto a = context->obtainStyledAttributes(attrs, R::styleable::ScaleAnimation);

    // AOSP defaults are 1.0f (identity scale).
    mFromX= a->getFloat(R::styleable::ScaleAnimation_fromXScale, 1.f);
    mToX  = a->getFloat(R::styleable::ScaleAnimation_toXScale,   1.f);
    mFromY= a->getFloat(R::styleable::ScaleAnimation_fromYScale, 1.f);
    mToY  = a->getFloat(R::styleable::ScaleAnimation_toYScale,   1.f);

    // AOSP: Description.parseValue(a.peekValue(pivotX), context); an absent
    // attr (tv stays TYPE_NULL) resolves to ABSOLUTE/0 like AOSP's null.
    TypedValue tv;
    a->peekValue(R::styleable::ScaleAnimation_pivotX, &tv);
    Description d = Description::parseValue(&tv, context);
    mPivotXType = d.type;
    mPivotXValue= d.value;

    tv = TypedValue();
    a->peekValue(R::styleable::ScaleAnimation_pivotY, &tv);
    d = Description::parseValue(&tv, context);
    mPivotYType = d.type;
    mPivotYValue= d.value;

    initializePivotPoint();
}

ScaleAnimation::ScaleAnimation(float fromX, float toX, float fromY, float toY)
    :ScaleAnimation(fromX,toX,fromY,toY,0,0){
    //mResources = nullptr;
    /*mFromX = fromX;
    mToX   = toX;
    mFromY = fromY;
    mToY   = toY;
    mPivotX= 0;
    mPivotY= 0;
    mFromXType = mFromYType = 0;
    mToXType = mToYType = 0;
    mPivotXType = mPivotYType = 0;*/
}

ScaleAnimation::ScaleAnimation(float fromX, float toX, float fromY, float toY, float pivotX, float pivotY)
    :ScaleAnimation(fromX,toX,fromY,toY,ABSOLUTE,pivotX,ABSOLUTE,pivotY){
    //mResources = null;
    /*mFromX = fromX;
    mToX   = toX;
    mFromY = fromY;
    mToY   = toY;

    mPivotXType  = ABSOLUTE;
    mPivotYType  = ABSOLUTE;
    mPivotXValue = pivotX;
    mPivotYValue = pivotY;
    initializePivotPoint();*/
}
ScaleAnimation::ScaleAnimation(float fromX, float toX, float fromY, float toY,
            int pivotXType, float pivotXValue, int pivotYType, float pivotYValue) {
    //mResources = null;
    mFromX = (fromX==0.f)?0.0001f:fromX;
    mToX   = (toX==0.f)?0.0001f:toX;
    mFromY = (fromY==0.f)?0.0001f:fromY;
    mToY   = (toY==0.f)?0.0001f:toY;

    mPivotXValue = pivotXValue;
    mPivotXType  = pivotXType;
    mPivotYValue = pivotYValue;
    mPivotYType  = pivotYType;
    initializePivotPoint();
}

ScaleAnimation* ScaleAnimation::clone()const{
    return new ScaleAnimation(*this);
}

void ScaleAnimation::initializePivotPoint() {
    if (mPivotXType == ABSOLUTE) {
        mPivotX = mPivotXValue;
    }
    if (mPivotYType == ABSOLUTE) {
        mPivotY = mPivotYValue;
    }
}

void ScaleAnimation::applyTransformation(float interpolatedTime, Transformation& t) {
    float sx = 1.0f;
    float sy = 1.0f;
    float scale = getScaleFactor();

    if (mFromX != 1.0f || mToX != 1.0f) {
        sx = mFromX + ((mToX - mFromX) * interpolatedTime);
    }
    if (mFromY != 1.0f || mToY != 1.0f) {
        sy = mFromY + ((mToY - mFromY) * interpolatedTime);
    }
    if (mPivotX == 0 && mPivotY == 0) {
        t.getMatrix().scale(sx, sy);
    } else {
        //t.getMatrix()->scale(sx, sy, scale * mPivotX, scale * mPivotY);
        t.getMatrix().translate( scale * mPivotX, scale * mPivotY);
        t.getMatrix().scale(sx, sy);
        t.getMatrix().translate( -scale * mPivotX, -scale * mPivotY);
    }
}

float ScaleAnimation::resolveScale(float scale, int type, int data, int size, int psize) {
    float targetSize;
    if (type == TypedValue::TYPE_FRACTION) {
        targetSize = TypedValue::complexToFraction(data, size, psize);
    } else if (type == TypedValue::TYPE_DIMENSION) {
        //targetSize = TypedValue::complexToDimension(data, mResources.getDisplayMetrics());
    } else {
        return scale;
    }

    if (size == 0) {
        return 1;
    }
    
    return targetSize/(float)size;
}
void ScaleAnimation::initialize(int width, int height, int parentWidth, int parentHeight) {
    Animation::initialize(width, height, parentWidth, parentHeight);

    mFromX = resolveScale(mFromX, mFromXType, mFromXData, width, parentWidth);
    mToX = resolveScale(mToX, mToXType, mToXData, width, parentWidth);
    mFromY = resolveScale(mFromY, mFromYType, mFromYData, height, parentHeight);
    mToY = resolveScale(mToY, mToYType, mToYData, height, parentHeight);

    mPivotX = resolveSize(mPivotXType, mPivotXValue, width, parentWidth);
    mPivotY = resolveSize(mPivotYType, mPivotYValue, height, parentHeight);
}
}
