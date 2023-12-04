#include <animation/translateanimation.h>
#include <cdtypes.h>
#include <cdlog.h>
namespace cdroid{

TranslateAnimation::TranslateAnimation(const TranslateAnimation&o):Animation(o){
    mFromXValue= o.mFromXValue;
    mToXValue  = o.mToXValue;
    mFromYValue= o.mFromYValue;
    mToYValue  = o.mToYValue;
    mFromXType = o.mFromXType;
    mToXType   = o.mToXType;
    mFromYType = o.mFromYType;
    mToYType   = o.mToYType;
}

TranslateAnimation::TranslateAnimation(Context* context,const AttributeSet& attrs)
    :Animation(context,attrs){
    mFromXValue = getPivotType(attrs.getString("toXDelta"),mFromXType);
    mFromYValue = getPivotType(attrs.getString("toYDelta"),mFromYType);
    mToXValue = getPivotType(attrs.getString("toYDelta"),mToXType);
    mToYValue = getPivotType(attrs.getString("toYDelta"),mToYType);
}

TranslateAnimation::TranslateAnimation(float fromXDelta, float toXDelta, float fromYDelta, float toYDelta) {
    mFromXValue = fromXDelta;
    mToXValue = toXDelta;
    mFromYValue = fromYDelta;
    mToYValue = toYDelta;

    mFromXType = ABSOLUTE;
    mToXType = ABSOLUTE;
    mFromYType = ABSOLUTE;
    mToYType = ABSOLUTE;
}

TranslateAnimation::TranslateAnimation(int fromXType, float fromXValue, int toXType, float toXValue,
        int fromYType, float fromYValue, int toYType, float toYValue) {

    mFromXValue = fromXValue;
    mToXValue = toXValue;
    mFromYValue = fromYValue;
    mToYValue = toYValue;

    mFromXType = fromXType;
    mToXType = toXType;
    mFromYType = fromYType;
    mToYType = toYType;
}

void TranslateAnimation::applyTransformation(float interpolatedTime, Transformation& t) {
    float dx = mFromXDelta;
    float dy = mFromYDelta;
    if (mFromXDelta != mToXDelta) {
        dx = mFromXDelta + ((mToXDelta - mFromXDelta) * interpolatedTime);
    }
    if (mFromYDelta != mToYDelta) {
        dy = mFromYDelta + ((mToYDelta - mFromYDelta) * interpolatedTime);
    }
    t.getMatrix().translate(dx, dy);
    LOGV("x:%.2f(%.2f,%.2f) y:%.2f(%.2f,%.2f) interpolatedTime=%.4f",dx,mFromXDelta,mToXDelta,dy,mFromYDelta,mToYDelta,interpolatedTime);
}

void TranslateAnimation::initialize(int width, int height, int parentWidth, int parentHeight) {
    Animation::initialize(width, height, parentWidth, parentHeight);
    mFromXDelta = resolveSize(mFromXType, mFromXValue, width, parentWidth);
    mToXDelta = resolveSize(mToXType, mToXValue, width, parentWidth);
    mFromYDelta = resolveSize(mFromYType, mFromYValue, height, parentHeight);
    mToYDelta = resolveSize(mToYType, mToYValue, height, parentHeight);
}

Animation*TranslateAnimation::clone(){
    TranslateAnimation*anim=new TranslateAnimation(*this);
    return anim;     
}
///////////////////////////////////////////////////////////////////////////////////////////////

TranslateXAnimation::TranslateXAnimation(float fromXDelta, float toXDelta)
   :TranslateAnimation(fromXDelta, toXDelta,0,0){
}

TranslateXAnimation::TranslateXAnimation(int fromXType, float fromXValue, int toXType, float toXValue)
    :TranslateAnimation(fromXType, fromXValue, toXType, toXValue, ABSOLUTE, 0, ABSOLUTE, 0){
}

void TranslateXAnimation::applyTransformation(float interpolatedTime, Transformation& t) {
    Matrix&m = t.getMatrix();
    float dx = mFromYDelta + ((mToXDelta - mFromXDelta) * interpolatedTime);
    m.translate(dx,/*mTmpValues[Matrix.MTRANS_X]*/m.y0);
    LOGV("x:%.2f(%.2f,%.2f)interpolatedTime=%.4f",dx,mFromXDelta,mToXDelta,interpolatedTime);
}

TranslateYAnimation::TranslateYAnimation(float fromYDelta, float toYDelta)
   :TranslateAnimation(0, 0, fromYDelta, toYDelta){
}

///////////////////////////////////////////////////////////////////////////////////////////////

TranslateYAnimation::TranslateYAnimation(int fromYType, float fromYValue, int toYType, float toYValue)
    :TranslateAnimation(ABSOLUTE, 0, ABSOLUTE, 0, fromYType, fromYValue, toYType, toYValue){
}

void TranslateYAnimation::applyTransformation(float interpolatedTime, Transformation& t) {
    Matrix& m = t.getMatrix();
    float dy = mFromYDelta + ((mToYDelta - mFromYDelta) * interpolatedTime);
    m.translate(/*mTmpValues[Matrix.MTRANS_X]*/m.x0, dy);
    LOGV("time= %f y=%.f",interpolatedTime,dy);
}

}
