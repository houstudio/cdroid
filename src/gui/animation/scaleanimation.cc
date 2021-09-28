#include <animation/scaleanimation.h>
#include <cdtypes.h>
#include <cdlog.h>
namespace cdroid{

ScaleAnimation::ScaleAnimation(const ScaleAnimation&o):Animation(o){
    mFromX  = o.mFromX;
    mToX    = o.mToX;
    mFromY  = o.mFromY;
    mToY    = o.mToY;
    mPivotXValue = o.mPivotXValue;
    mPivotXType  = o.mPivotXType;
    mPivotYValue = o.mPivotYValue;
    mPivotYType  = o.mPivotYType;
    initializePivotPoint();
}

ScaleAnimation::ScaleAnimation(Context* context,const AttributeSet& attrs)
    :Animation(context,attrs){
}

ScaleAnimation::ScaleAnimation(float fromX, float toX, float fromY, float toY) {
    //mResources = nullptr;
    mFromX = fromX;
    mToX   = toX;
    mFromY = fromY;
    mToY   = toY;
    mPivotX= 0;
    mPivotY= 0;
}

ScaleAnimation::ScaleAnimation(float fromX, float toX, float fromY, float toY,
            float pivotX, float pivotY) {
    //mResources = null;
    mFromX = fromX;
    mToX   = toX;
    mFromY = fromY;
    mToY   = toY;

    mPivotXType  = ABSOLUTE;
    mPivotYType  = ABSOLUTE;
    mPivotXValue = pivotX;
    mPivotYValue = pivotY;
    initializePivotPoint();
}
ScaleAnimation::ScaleAnimation(float fromX, float toX, float fromY, float toY,
            int pivotXType, float pivotXValue, int pivotYType, float pivotYValue) {
    //mResources = null;
    mFromX = fromX;
    mToX   = toX;
    mFromY = fromY;
    mToY   = toY;

    mPivotXValue = pivotXValue;
    mPivotXType  = pivotXType;
    mPivotYValue = pivotYValue;
    mPivotYType  = pivotYType;
    initializePivotPoint();
}

Animation* ScaleAnimation::clone(){
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
