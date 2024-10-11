#include <animation/scaleanimation.h>
#include <cdtypes.h>
#include <cdlog.h>
namespace cdroid{

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
    mFromX= attrs.getFloat("fromXScale",.0f);
    mToX  = attrs.getFloat("toXScale",.0f);
    mFromY= attrs.getFloat("fromYScale",.0f);
    mToY  = attrs.getFloat("toYScale",.0f);
    mPivotXValue = getPivotType(attrs.getString("pivotX"),mPivotXType);
    mPivotYValue = getPivotType(attrs.getString("pivotY"),mPivotYType);
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
