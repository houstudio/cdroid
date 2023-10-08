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
    void setDialTintList(ColorStateList*);
    ColorStateList*getDialTintList()const;

    void setHourHand(Icon icon);
    void setHourHandTintList(ColorStateList*);
    ColorStateList* getHourHandTintList()const;

    void setMinuteHand(Icon icon);
    void setMinuteHandTintList(ColorStateList*);
    ColorStateList* getMinuteHandTintList()const;

    void setSecondHand(Icon icon);
    void setSecondHandTintList(ColorStateList*);
    ColorStateList* getSecondHandTintList()const;

    void onVisibilityAggregated(bool isVisible)override;
}; 

}//namespace
#endif
