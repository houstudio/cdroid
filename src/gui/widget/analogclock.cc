#include <widget/analogclock.h>
#include <systemclock.h>
#include <cdlog.h>
#include<iomanip>
#include <ctime>


namespace cdroid{

DECLARE_WIDGET(AnalogClock)

AnalogClock::AnalogClock(Context*ctx,const AttributeSet& attrs)
  :View(ctx,attrs){
    initAnalog();

    setDial (ctx->getDrawable(attrs,"dial"));
    setHourHand( ctx->getDrawable(attrs,"hand_hour"));
    setMinuteHand( ctx->getDrawable(attrs,"hand_minute"));
    setSecondHand( ctx->getDrawable(attrs,"hand_second"));

    mDialWidth = mDial->getIntrinsicWidth();
    mDialHeight= mDial->getIntrinsicHeight();
}

AnalogClock::AnalogClock(int w,int h):View(w,h){
    initAnalog();
}


void AnalogClock::initAnalog(){
    mDial = mSecondHand = nullptr;
    mMinuteHand = mHourHand = nullptr;
    mHour = mMinutes = mSeconds = 0;
    mDialWidth = mDialHeight =0;
    mSecondsHandFps = 1;
    mVisible = false;
    mHourHandTintInfo = mMinuteHandTintInfo = nullptr;
    mSecondHandTintInfo = mDialTintInfo = nullptr;
}

void AnalogClock::setDial(Icon icon) {
    mDial = icon;//.loadDrawable(getContext());
    mDialWidth  = mDial->getIntrinsicWidth();
    mDialHeight = mDial->getIntrinsicHeight();
    /*if (mDialTintInfo->mHasTintList || mDialTintInfo->mHasTintBlendMode) {
        mDial = mDialTintInfo->apply(mDial);
    }*/

    mChanged = true;
    requestLayout();
}

void AnalogClock::setHourHand(Icon icon) {
    mHourHand = icon;//.loadDrawable(getContext());
    /*if (mHourHandTintInfo.mHasTintList || mHourHandTintInfo.mHasTintBlendMode) {
        mHourHand = mHourHandTintInfo.apply(mHourHand);
    }*/

    mChanged = true;
    requestLayout();
}

void AnalogClock::setMinuteHand(Icon icon) {
    mMinuteHand = icon;//.loadDrawable(getContext());
    /*if (mHourHandTintInfo.mHasTintList || mHourHandTintInfo.mHasTintBlendMode) {
        mHourHand = mHourHandTintInfo.apply(mHourHand);
    }*/

    mChanged = true;
    requestLayout();
}

void AnalogClock::setSecondHand(Icon icon) {
    mSecondHand = icon;//.loadDrawable(getContext());
    /*if (mHourHandTintInfo.mHasTintList || mHourHandTintInfo.mHasTintBlendMode) {
        mHourHand = mHourHandTintInfo.apply(mHourHand);
    }*/

    mChanged = true;
    invalidate();
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
    mTick=[&](){
        std::time_t t = std::time(NULL);
        struct std::tm when= *std::localtime(&t);
        std::get_time(&when,"%R");
        mHour=when.tm_hour;
        mMinutes=when.tm_min;
        mSeconds=when.tm_sec;
        mChanged = true;
        invalidate(true);
        postDelayed(mTick,500);
    };
    postDelayed(mTick,800);
}


void AnalogClock::onDetachedFromWindow() {
    /*if (mReceiverAttached) {
        getContext().unregisterReceiver(mIntentReceiver);
        mReceiverAttached = false;
    }*/
    View::onDetachedFromWindow();
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

    int x = availableWidth / 2;
    int y = availableHeight / 2;

    int w = mDial->getIntrinsicWidth();
    int h = mDial->getIntrinsicHeight();

    bool scaled = false;
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

    if (changed) {
        w = mMinuteHand->getIntrinsicWidth();
        h = mMinuteHand->getIntrinsicHeight();
        mMinuteHand->setBounds( - (w / 2), - (h / 2),w,h);
        LOGV("MinuteHand.size=%dx%d  %p",w,h,mMinuteHand);
    }
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
