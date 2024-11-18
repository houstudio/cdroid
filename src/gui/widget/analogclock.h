#ifndef __ANALOG_CLOCK_H__
#define __ANALOG_CLOCK_H__
#include <widget/imageview.h>
namespace cdroid{
typedef Drawable* Icon;
class AnalogClock:public View{
private:
    int mSecondsHandFps;
    Drawable* mHourHand;
    Drawable* mMinuteHand;
    Drawable* mSecondHand;
    Drawable* mDial;

    TintInfo* mHourHandTintInfo;
    TintInfo* mMinuteHandTintInfo;
    TintInfo* mSecondHandTintInfo;
    TintInfo* mDialTintInfo;

    int mDialWidth;
    int mDialHeight;
    bool mVisible;

    float mSeconds;
    float mMinutes;
    float mHour;
    bool mChanged;    
    Runnable mTick;
private:
    void initAnalog();
    void onVisible();
    void onInvisible();
    Drawable* apply(TintInfo*,Drawable*);
    void onTickProc();
    void onTimeChanged();
    void onTimeChanged(/*LocalTime localTime,*/int64_t nowMillis);
protected:
    void onAttachedToWindow()override;
    void onDetachedFromWindow()override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onSizeChanged(int w, int h, int oldw, int oldh)override;
    void onDraw(Canvas&canvas)override;
public:
    AnalogClock(int w,int h);
    AnalogClock(Context*ctx,const AttributeSet& attrs);
    ~AnalogClock();
    void setDial(Icon icon);
    void setDialTintList(const ColorStateList*);
    const ColorStateList*getDialTintList()const;

    void setHourHand(Icon icon);
    void setHourHandTintList(const ColorStateList*);
    const ColorStateList* getHourHandTintList()const;

    void setMinuteHand(Icon icon);
    void setMinuteHandTintList(const ColorStateList*);
    const ColorStateList* getMinuteHandTintList()const;

    void setSecondHand(Icon icon);
    void setSecondHandTintList(const ColorStateList*);
    ColorStateList* getSecondHandTintList()const;

    void onVisibilityAggregated(bool isVisible)override;
}; 

}//namespace
#endif
