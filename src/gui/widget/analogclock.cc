#include <widget/analogclock.h>
#include <systemclock.h>
#include <cdlog.h>
#include<iomanip>
#include <ctime>
#include <iomanip>

namespace cdroid{

DECLARE_WIDGET(AnalogClock)

AnalogClock::AnalogClock(Context*ctx,const AttributeSet& attrs)
  :View(ctx,attrs){
    initAnalog();

    setDial (attrs.getDrawable("dial"));
    setHourHand( attrs.getDrawable("hand_hour"));
    setMinuteHand( attrs.getDrawable("hand_minute"));
    setSecondHand( attrs.getDrawable("hand_second"));
}

AnalogClock::AnalogClock(int w,int h):View(w,h){
    initAnalog();
}

AnalogClock::~AnalogClock(){
    delete mDial;
    delete mHourHand;
    delete mMinuteHand;
    delete mSecondHand;
    delete mDialTintInfo;
    delete mHourHandTintInfo;
    delete mMinuteHandTintInfo;
    delete mSecondHandTintInfo;
}

void AnalogClock::initAnalog(){
    mDial = mSecondHand = nullptr;
    mMinuteHand = mHourHand = nullptr;
    mHour = mMinutes = mSeconds = 0;
    mDialWidth = mDialHeight =0;
    mSecondsHandFps = 1;
    mVisible = false;
    mDialTintInfo = new TintInfo;
    mHourHandTintInfo = new TintInfo;
    mMinuteHandTintInfo = new TintInfo;
    mSecondHandTintInfo = new TintInfo;
}

Drawable* AnalogClock::apply(TintInfo*ti,Drawable*drawable){
    if (drawable == nullptr) return nullptr;

    Drawable* newDrawable = drawable->mutate();

    if (ti->mHasTintList) {
        newDrawable->setTintList(ti->mTintList);
    }
    /*if (ti->mHasTintBlendMode) {
        newDrawable->setTintBlendMode(ti->mTintBlendMode);
    }*/

    // All drawables should have the same state as the View itself.
    if (drawable->isStateful()) {
        newDrawable->setState(getDrawableState());
    }
    return newDrawable;
}

void AnalogClock::setDial(Icon icon) {
    delete mDial;
    mDial = icon;//.loadDrawable(getContext());
    if(mDial){
        mDialWidth  = mDial->getIntrinsicWidth();
        mDialHeight = mDial->getIntrinsicHeight();
        if (mDialTintInfo->mHasTintList /*|| mDialTintInfo->mHasTintBlendMode*/) {
           mDial = apply(mDialTintInfo,mDial);
        }
        mDial->setFilterBitmap(true);
    }
    mChanged = true;
    requestLayout();
}

void AnalogClock::setDialTintList(const ColorStateList* tint) {
    if(mDialTintInfo->mTintList!=tint){
        mDialTintInfo->mTintList = tint;
        mDialTintInfo->mHasTintList = (tint!=nullptr);
        mDial = apply(mDialTintInfo,mDial);
    }
}

const ColorStateList* AnalogClock::getDialTintList()const {
    return mDialTintInfo->mTintList;
}

void AnalogClock::setHourHand(Icon icon) {
    mHourHand = icon;//.loadDrawable(getContext());

    if (mHourHandTintInfo->mHasTintList/* || mHourHandTintInfo->mHasTintBlendMode*/) {
        mHourHand = apply(mHourHandTintInfo,mHourHand);
    }

    if( (mDial==nullptr) && mHourHand){
        const int32_t dw = mHourHand->getIntrinsicWidth()*2;
        const int32_t dh = mHourHand->getIntrinsicHeight()*2;
        if( (dw>mDialWidth) || (dh>mDialHeight) ){
            mDialWidth = dw;
            mDialHeight= dh;
        }
    }
    if(mHourHand)
        mHourHand->setFilterBitmap(true);
    mChanged = true;
    requestLayout();
}

void AnalogClock::setHourHandTintList(const ColorStateList* tint) {
    if(mHourHandTintInfo->mTintList!=tint){
        mHourHandTintInfo->mTintList = tint;
        mHourHandTintInfo->mHasTintList = (tint!=nullptr);
        mHourHand = apply(mHourHandTintInfo,mHourHand);
    }
}

const ColorStateList* AnalogClock::getHourHandTintList()const{
    return mHourHandTintInfo->mTintList;
}

void AnalogClock::setMinuteHand(Icon icon) {
    delete mMinuteHand;
    mMinuteHand = icon;//.loadDrawable(getContext());

    if (mHourHandTintInfo->mHasTintList /*|| mHourHandTintInfo.mHasTintBlendMode*/) {
        mHourHand = apply(mHourHandTintInfo,mHourHand);
    }

    if( (mDial==nullptr) && mMinuteHand){
        const int32_t dw = mMinuteHand->getIntrinsicWidth()*2;
        const int32_t dh = mMinuteHand->getIntrinsicHeight()*2;
        if( (dw>mDialWidth) || (dh>mDialHeight) ){
            mDialWidth = dw;
            mDialHeight= dh;
        }
    }
    if(mMinuteHand)
        mMinuteHand->setFilterBitmap(true);
    mChanged = true;
    requestLayout();
}

void AnalogClock::setSecondHand(Icon icon) {
    delete mSecondHand;
    mSecondHand = icon;//.loadDrawable(getContext());

    if (mHourHandTintInfo->mHasTintList /*|| mHourHandTintInfo.mHasTintBlendMode*/) {
        mHourHand = apply(mHourHandTintInfo,mHourHand);
    }

    if( (mDial==nullptr) && mMinuteHand){
        const int32_t dw = mSecondHand->getIntrinsicWidth()*2;
        const int32_t dh = mSecondHand->getIntrinsicHeight()*2;
        if( (dw>mDialWidth) || (dh>mDialHeight) ){
            mDialWidth = dw;
            mDialHeight= dh;
        }
    }
    if(mSecondHand)
        mSecondHand->setFilterBitmap(true);
    mChanged = true;
    invalidate();
}

void AnalogClock::setMinuteHandTintList(const ColorStateList* tint) {
    if(mMinuteHandTintInfo->mTintList!=tint){
        mMinuteHandTintInfo->mTintList = tint;
        mMinuteHandTintInfo->mHasTintList = (tint!=nullptr);
        mMinuteHand = apply(mMinuteHandTintInfo,mMinuteHand);
    }
}

const ColorStateList* AnalogClock::getMinuteHandTintList()const{
    return mMinuteHandTintInfo->mTintList;
}

void AnalogClock::onVisibilityAggregated(bool isVisible) {
    View::onVisibilityAggregated(isVisible);

    if (isVisible) {
        onVisible();
    } else {
        onInvisible();
    }
}

void AnalogClock::onAttachedToWindow(){
    mTick=[this](){
        std::time_t t = std::time(NULL);
        struct std::tm when= *std::localtime(&t);
        std::get_time(&when,"%R");
        mHour    = (float)when.tm_hour;
        mMinutes = (float)when.tm_min;
        mSeconds = (float)when.tm_sec;
        mChanged = true;
        this->invalidate(true);
        this->postDelayed(mTick,500);
    };
    post(mTick);
}


void AnalogClock::onDetachedFromWindow() {
    /*if (mReceiverAttached) {
        getContext().unregisterReceiver(mIntentReceiver);
        mReceiverAttached = false;
    }*/
    View::onDetachedFromWindow();
    removeCallbacks(mTick);
    if(mHourHand)unscheduleDrawable(*mHourHand);
    if(mMinuteHand)unscheduleDrawable(*mMinuteHand);
    if(mSecondHand)unscheduleDrawable(*mSecondHand);
    if(mDial)unscheduleDrawable(*mDial);
}

void AnalogClock::onVisible() {
    if (!mVisible) {
        mVisible = true;
        //mTick.run();
    }

}

void AnalogClock::onInvisible() {
    if (mVisible) {
        removeCallbacks(mTick);
        mVisible = false;
    }
}

void AnalogClock::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    const int widthMode  = MeasureSpec::getMode(widthMeasureSpec);
    const int widthSize  = MeasureSpec::getSize(widthMeasureSpec);
    const int heightMode = MeasureSpec::getMode(heightMeasureSpec);
    const int heightSize = MeasureSpec::getSize(heightMeasureSpec);

    float hScale = 1.0f;
    float vScale = 1.0f;

    if (widthMode != MeasureSpec::UNSPECIFIED && widthSize < mDialWidth) {
        hScale = (float) widthSize / (float) mDialWidth;
    }

    if (heightMode != MeasureSpec::UNSPECIFIED && heightSize < mDialHeight) {
        vScale = (float )heightSize / (float) mDialHeight;
    }

    const float scale = std::min(hScale, vScale);

    setMeasuredDimension(resolveSizeAndState((int) (mDialWidth * scale), widthMeasureSpec, 0),
            resolveSizeAndState((int) (mDialHeight * scale), heightMeasureSpec, 0));
}

void AnalogClock::onSizeChanged(int w, int h, int oldw, int oldh){
    View::onSizeChanged(w,h,oldw,oldh);
    mChanged = true;
}

void AnalogClock::onDraw(Canvas&canvas){
    View::onDraw(canvas);

    bool changed = mChanged;
    if (changed) {
        mChanged = false;
    }

    const int availableWidth = mRight - mLeft;
    const int availableHeight = mBottom - mTop;

    int x,y,w,h;
    bool scaled = false;
    x = availableWidth / 2;
    y = availableHeight / 2;
    if(mDial){
        w = mDial->getIntrinsicWidth();
        h = mDial->getIntrinsicHeight();

        LOGV("Dial.size=%dx%d  %p",w,h,mDial);
        if (availableWidth < w || availableHeight < h) {
            scaled = true;
            float scale = std::min((float) availableWidth / (float) w,
                              (float) availableHeight / (float) h);
            canvas.save();
            canvas.scale(scale, scale);
        }

        if (changed) {
            mDial->setBounds(x - (w / 2), y - (h / 2), w,h);
        }
        mDial->draw(canvas);
    }

    canvas.save();
    canvas.translate(x,y);
    canvas.rotate_degrees((mHour+mMinutes/60.f) / 12.0f * 360.0f);
    if (changed) {
        w = mHourHand->getIntrinsicWidth();
        h = mHourHand->getIntrinsicHeight();
        mHourHand->setBounds( - (w / 2), - (h / 2), w,h);
        LOGV("HourHand.size=%dx%d  %p",w,h,mHourHand);
    }
    mHourHand->draw(canvas);
    canvas.restore();

    canvas.save();
    canvas.translate(x,y);
    canvas.rotate_degrees((mMinutes+mSeconds/60.f) / 60.0f * 360.0f);

    if (changed && mMinuteHand) {
        w = mMinuteHand->getIntrinsicWidth();
        h = mMinuteHand->getIntrinsicHeight();
        mMinuteHand->setBounds( - (w / 2), - (h / 2),w,h);
        LOGV("MinuteHand.size=%dx%d  %p",w,h,mMinuteHand);
    }
    if(mMinuteHand)
	mMinuteHand->draw(canvas);
    canvas.restore();

    if (mSecondHand  && mSecondsHandFps > 0) {
        canvas.save();
        canvas.translate(x,y);
        canvas.rotate_degrees(mSeconds / 60.0f * 360.0f);
        if (changed) {
            w = mSecondHand->getIntrinsicWidth();
            h = mSecondHand->getIntrinsicHeight();
            mSecondHand->setBounds( - (w / 2), - (h / 2),w,h);
        }
        mSecondHand->draw(canvas);
        canvas.restore();
    }

    if (scaled) {
        canvas.restore();
    }
}

}
