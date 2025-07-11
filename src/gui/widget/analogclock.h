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

}/*endof namespace*/
#endif
