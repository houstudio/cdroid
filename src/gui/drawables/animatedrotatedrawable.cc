#include <drawables/animatedrotatedrawable.h>
#include <drawables/bitmapdrawable.h>
#include <systemclock.h>
#include <cdlog.h>
#include <fstream>

using namespace Cairo;
namespace cdroid{

AnimatedRotateDrawable::AnimatedRotateState::AnimatedRotateState(){
    mPivotX=mPivotY=0;
    mPivotXRel=mPivotYRel=false;
    mFrameDuration=150;
    mFramesCount=12;
}

AnimatedRotateDrawable::AnimatedRotateState::AnimatedRotateState(const AnimatedRotateState& orig)
    :DrawableWrapperState(orig){
    mPivotXRel = orig.mPivotXRel;
    mPivotX = orig.mPivotX;
    mPivotYRel = orig.mPivotYRel;
    mPivotY = orig.mPivotY;
    mFramesCount = orig.mFramesCount;
    mFrameDuration = orig.mFrameDuration;
}

AnimatedRotateDrawable* AnimatedRotateDrawable::AnimatedRotateState::newDrawable(){
    return new AnimatedRotateDrawable(std::dynamic_pointer_cast<AnimatedRotateState>(shared_from_this()));
}
int  AnimatedRotateDrawable::AnimatedRotateState::getChangingConfigurations()const{
    return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////
AnimatedRotateDrawable::AnimatedRotateDrawable()
    :AnimatedRotateDrawable(std::make_shared<AnimatedRotateState>()){
}

AnimatedRotateDrawable::AnimatedRotateDrawable(std::shared_ptr<AnimatedRotateState> state):DrawableWrapper(state){
    mState  = state;
    mRunning= false;
    mIncrement= 360./state->mFramesCount;
    mCurrentDegrees = .0f;

    updateLocalState();
    mNextFrame = [this](){
        mCurrentDegrees += mIncrement;
        if( (mIncrement < 0) && (mCurrentDegrees < -360.f - mIncrement) ){
            mCurrentDegrees = 0.f;
        }else if ((mIncrement > 0) && (mCurrentDegrees > 360.0f - mIncrement)) {
            mCurrentDegrees = 0.0f;
        }
        invalidateSelf();
        nextFrame();
    };
}

AnimatedRotateDrawable::~AnimatedRotateDrawable(){
    stop();
    mNextFrame = nullptr;
}

std::shared_ptr<DrawableWrapper::DrawableWrapperState> AnimatedRotateDrawable::mutateConstantState(){
    mState = std::make_shared<AnimatedRotateState>(*mState);
    return mState;
}

float AnimatedRotateDrawable::getPivotX()const{
    return mState->mPivotX;
}

float AnimatedRotateDrawable::getPivotY()const{
    return mState->mPivotY;
}

void AnimatedRotateDrawable::setPivotX(float pivotX){
    mState->mPivotX=pivotX;
}

void AnimatedRotateDrawable::setPivotY(float pivotY){
     mState->mPivotY=pivotY;
}

bool AnimatedRotateDrawable::isPivotXRelative()const{
    return mState->mPivotXRel;
}

void AnimatedRotateDrawable::setPivotXRelative(bool relative){
    mState->mPivotXRel=relative;
}

bool AnimatedRotateDrawable::isPivotYRelative()const{
    return mState->mPivotYRel;
}

void AnimatedRotateDrawable::setPivotYRelative(bool relative){
    mState->mPivotYRel=relative;
}

void AnimatedRotateDrawable::start() {
    LOGV("AnimatedRotateDrawable.start %p, mRunning=%d",this,mRunning);
    if (!mRunning) {
        mRunning = true;
        nextFrame();
    }
}

void AnimatedRotateDrawable::stop() {
    LOGV("AnimatedRotateDrawable.stoped %p,running=%d",this,mRunning);
    mRunning = false;
    unscheduleSelf(mNextFrame);
}

bool AnimatedRotateDrawable::isRunning() {
    return mRunning;
}

void AnimatedRotateDrawable::nextFrame() {
    unscheduleSelf(mNextFrame);
    if(mRunning){
        scheduleSelf(mNextFrame,SystemClock::uptimeMillis()+mState->mFrameDuration);
        mCurrentDegrees += mIncrement;
    }
}

bool AnimatedRotateDrawable::setVisible(bool visible, bool restart){
    const bool changed = DrawableWrapper::setVisible(visible, restart);
    if (visible) {
        if (changed || restart) {
            mCurrentDegrees = 0.0f;
            nextFrame();
        }
    } else {
        unscheduleSelf(mNextFrame);
    }
    return changed;
}

void AnimatedRotateDrawable::setFramesCount(int framesCount) {
    mState->mFramesCount = framesCount;
    mIncrement = 360.0f / mState->mFramesCount;
}

void AnimatedRotateDrawable::setFramesDuration(int framesDuration) {
    mState->mFrameDuration = framesDuration;
}

void AnimatedRotateDrawable::updateLocalState(){
    mIncrement = 360.0f / mState->mFramesCount;

    // Force the wrapped drawable to use filtering and AA, if applicable,
    // so that it looks smooth when rotated.
    Drawable* drawable = getDrawable();
    if (drawable != nullptr) {
        drawable->setFilterBitmap(true);
        if (dynamic_cast<BitmapDrawable*>(drawable)) {
            ((BitmapDrawable*)drawable)->setAntiAlias(true);
        }
    }
}
static inline float sdot(float a,float b,float c,float d){
    return a * b + c * d;
}

void AnimatedRotateDrawable::draw(Canvas& canvas) {
    Drawable* drawable = getDrawable();
    const Rect bounds = drawable->getBounds();
    const int w = bounds.width;
    const int h = bounds.height;

    float px = bounds.left+(mState->mPivotXRel ? (w * mState->mPivotX) : mState->mPivotX);
    float py = bounds.top+(mState->mPivotYRel ? (h * mState->mPivotY) : mState->mPivotY);
    LOGV("%p bounds(%d,%d %d,%d) pivot=%f,%f pxy=%f,%f degrees=%f",this,bounds.left,bounds.top,bounds.width,bounds.height,
         mState->mPivotX, mState->mPivotY,px,py,mCurrentDegrees);

    const float radians=M_PI*2.f*mCurrentDegrees/360.f;
    const float fsin=sin(radians);
    const float fcos=cos(radians);
    const bool filteredBitmap = drawable->isFilterBitmap();
#if 0//Anti clockwise
    Matrix mtx(fcos,-fsin, fsin,fcos,
            sdot(-fsin,py,1-fcos,px), sdot(fsin,px,1-fcos,py));
#else//Clockwise
    Matrix mtx(fcos,fsin, -fsin,fcos,
            sdot(fsin,py,1-fcos,px), sdot(-fsin,px,1-fcos,py));
    canvas.save();
    canvas.translate(bounds.left,bounds.top);
    canvas.transform(mtx);
    if(!filteredBitmap)
        drawable->setFilterBitmap(true);
    drawable->setBounds(0,0,w,h);
    drawable->draw(canvas);
    drawable->setBounds(bounds);
    if(!filteredBitmap)
        drawable->setFilterBitmap(false);
    canvas.translate(-bounds.left,-bounds.top);
    canvas.restore();
#endif
}

Drawable*AnimatedRotateDrawable::inflate(Context*ctx,const AttributeSet&atts){
    AnimatedRotateDrawable*ad = new AnimatedRotateDrawable();
    ad->setPivotX(atts.getFraction("pivotX",1,1,0));
    ad->setPivotY(atts.getFraction("pivotY",1,1,0));
    ad->setPivotXRelative(true);
    ad->setPivotYRelative(true);
    ad->setFramesCount(atts.getInt("framesCount"));
    ad->setFramesDuration(atts.getInt("frameDuration"));
    Drawable*child = createWrappedDrawable(ctx,atts);
    ad->setDrawable(child);
    ad->setFilterBitmap(true);
    return ad;
}
}

