#ifndef __ANALOG_CLOCK_H__
#define __ANALOG_CLOCK_H__
#include <widget/imageview.h>
namespace cdroid{

class AnalogClock:public View{
private:
	int mSeconds;
    int mMinutes;
    int mHour;
    Drawable* mHourHand;
    Drawable* mMinuteHand;
	Drawable* mSecondHand;
    Drawable* mDial;
protected:
   void onAttached();
   void onDraw(Canvas&canvas)override;
public:
   enum{
       DIAL,
       HOUR,
       MINUTE,
       SECOND
   };

   AnalogClock(Context*ctx,const AttributeSet& attrs);
   AnalogClock(int w,int h);
   void setClockDrawable(Drawable*d,int id);
   Drawable*getClockDrawable(int id);
   //bool onMessage(DWORD msg,DWORD wp,ULONG lp);
}; 

}//namespace
#endif
