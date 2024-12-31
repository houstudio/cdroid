#include <drawables/rotatedrawable.h>
#include <cdlog.h>

using namespace Cairo;
namespace cdroid{
#define MAX_LEVEL 10000
RotateDrawable::RotateState::RotateState()
    :DrawableWrapperState(){
    mFromDegrees =.0;
    mToDegrees = 360.0;
    mPivotX = mPivotY = 0.5;
    mPivotXRel = mPivotYRel = true;
    mCurrentDegrees = 0;
}

RotateDrawable::RotateState::RotateState(const RotateState& orig)
    :DrawableWrapperState(orig){
    mFromDegrees= orig.mFromDegrees;
    mToDegrees  = orig.mToDegrees;
    mPivotX = orig.mPivotX;
    mPivotY = orig.mPivotY;
    mPivotXRel= orig.mPivotXRel;
    mPivotYRel= orig.mPivotYRel;
    mCurrentDegrees = orig.mCurrentDegrees;
}

RotateDrawable*RotateDrawable::RotateState::newDrawable(){
    return new RotateDrawable(std::dynamic_pointer_cast<RotateState>(shared_from_this()));
}

//////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<DrawableWrapper::DrawableWrapperState> RotateDrawable::mutateConstantState(){
    mState = std::make_shared<RotateState>(*mState);
    return mState;
}

RotateDrawable::RotateDrawable(std::shared_ptr<RotateState>state):DrawableWrapper(state){
    mState = state;
}

RotateDrawable::RotateDrawable(Drawable*d)
    :RotateDrawable(std::make_shared<RotateState>()){
    setDrawable(d);
}

static float lerp(float start,float stop,float amount){
    return  start + (stop - start) * amount;
}

bool RotateDrawable::onLevelChange(int level){
    DrawableWrapper::onLevelChange(level);
    const float value = level / (float) MAX_LEVEL;
    const float degrees = lerp(mState->mFromDegrees,mState->mToDegrees, value);
    mState->mCurrentDegrees = degrees;
    invalidateSelf();
    return true;
}

void RotateDrawable::onBoundsChange(const Rect& bounds){
    Drawable*d = getDrawable();
    const float px = mState->mPivotXRel ? (bounds.width * mState->mPivotX) : mState->mPivotX;
    const float py = mState->mPivotYRel ? (bounds.height * mState->mPivotY) : mState->mPivotY;
    Rect bound= bounds;
    bound.left = -int(px);
    bound.top = -int(py);
    d->setBounds(bound);
}

float RotateDrawable::getFromDegrees()const{
    return mState->mFromDegrees;
}

float RotateDrawable::getToDegrees()const{
    return mState->mToDegrees;
}

void RotateDrawable::setFromDegrees(float fromDegrees) {
    if (mState->mFromDegrees != fromDegrees) {
        mState->mFromDegrees = fromDegrees;
        invalidateSelf();
    }
}

void RotateDrawable::setToDegrees(float toDegrees) {
    if (mState->mToDegrees != toDegrees) {
        mState->mToDegrees = toDegrees;
        invalidateSelf();
    }
}

float RotateDrawable::getPivotX()const{
    return mState->mPivotX;
}

float RotateDrawable::getPivotY()const{
    return mState->mPivotY;
}

void RotateDrawable::setPivotX(float pivotX) {
    if (mState->mPivotX != pivotX) {
        mState->mPivotX = pivotX;
        invalidateSelf();
    }
}

void RotateDrawable::setPivotY(float pivotY) {
    if (mState->mPivotY != pivotY) {
        mState->mPivotY = pivotY;
        invalidateSelf();
    }
}

void RotateDrawable::setPivotXRelative(bool relative) {
    if (mState->mPivotXRel != relative) {
        mState->mPivotXRel = relative;
        invalidateSelf();
    }
}

bool RotateDrawable::isPivotYRelative()const{
    return mState->mPivotYRel;
}

void RotateDrawable::setPivotYRelative(bool relative){
    if(mState->mPivotYRel!=relative){
        mState->mPivotYRel=relative;
        invalidateSelf();
    }
}

std::shared_ptr<Drawable::ConstantState>RotateDrawable::getConstantState(){
    return mState;
}

static inline float sdot(float a,float b,float c,float d){
    return a * b + c * d;
}

void RotateDrawable::draw(Canvas& canvas) {
    Drawable*d = getDrawable();
    const Rect& bounds = getBounds();
    const float px = mState->mPivotXRel ? (bounds.width * mState->mPivotX) : mState->mPivotX;
    const float py = mState->mPivotYRel ? (bounds.height * mState->mPivotY) : mState->mPivotY;
    LOGV("%p bounds(%d,%d %d,%d) pivot=%f,%f pxy=%f,%f degrees=%f",this,bounds.left,bounds.top,bounds.width,bounds.height,
         mState->mPivotX, mState->mPivotY,px,py,mState->mCurrentDegrees);

    const float radians = M_PI*mState->mCurrentDegrees/180.f;
    const float fsin = sin(radians);
    const float fcos = cos(radians);
    Matrix mtx(fcos,fsin, -fsin,fcos,
            sdot(fsin,py,1-fcos,px), sdot(-fsin,px,1-fcos,py));

    canvas.save();
    canvas.translate(bounds.left,bounds.top);
    canvas.transform(mtx);
    d->setBounds(0,0,bounds.width,bounds.height);
    d->draw(canvas);
    d->setBounds(bounds);
    canvas.restore();
    LOGV("pos=%d,%d/%.f,%.f level=%d degress=%d",bounds.left,bounds.top,px,py,getLevel(),int(mState->mCurrentDegrees));
}

Drawable*RotateDrawable::inflate(Context*ctx,const AttributeSet&atts){
    Drawable*d = createWrappedDrawable(ctx,atts);
    RotateDrawable*rd = new RotateDrawable(d);
    const float px = atts.getFraction("pivotX",1,1,0.5f);
    const float py = atts.getFraction("pivotY",1,1,0.5f);
    rd->setPivotX(px);
    rd->setPivotY(py);
    rd->setPivotXRelative(px<=1.f);
    rd->setPivotYRelative(py<=1.f);
    rd->setFromDegrees(atts.getFloat("fromDegrees",0));
    rd->setToDegrees(atts.getFloat("toDegrees",360.0));
    rd->onLevelChange(0);
    return rd;
}

}
