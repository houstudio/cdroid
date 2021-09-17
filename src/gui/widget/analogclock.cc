#include <widget/analogclock.h>
#include <systemclock.h>
#include <cdlog.h>
#include<iomanip>
#include <ctime>


namespace cdroid{

AnalogClock::AnalogClock(Context*ctx,const AttributeSet& attrs):View(ctx,attrs){
    mDial=mSecondHand=mMinuteHand=mHourHand=nullptr;
    mHour=mMinutes=mSeconds=0;
    setClockDrawable(ctx->getDrawable(attrs.getString("dial")) , DIAL);
    setClockDrawable(ctx->getDrawable(attrs.getString("second")),SECOND);
    setClockDrawable(ctx->getDrawable(attrs.getString("minute")),MINUTE);
    setClockDrawable(ctx->getDrawable(attrs.getString("HOUR")) , HOUR);
}

AnalogClock::AnalogClock(int w,int h):View(w,h){
    mDial=mSecondHand=mMinuteHand=mHourHand=nullptr;
    mHour=mMinutes=mSeconds=0;
}

void AnalogClock::setClockDrawable(Drawable*d,int id){
    switch(id){
    case DIAL  :
        delete mDial;
        mDial=d;
        break;
    case SECOND:
        delete mSecondHand;
        mSecondHand=d;
        break;
    case MINUTE:
        delete mMinuteHand;
        mMinuteHand=d;
    case HOUR:
        delete mHourHand;
        mHourHand=d;
        break;
    default:return;
    }
    invalidate(true);
}

Drawable*AnalogClock::getClockDrawable(int id){
    switch(id){
	case DIAL:  return mDial;
	case SECOND:return mSecondHand;
	case MINUTE:return mMinuteHand;
	case HOUR  :return mHourHand;
	default:return nullptr;
    }
}

void AnalogClock::onAttachedToWindow(){
    mRunner=[&](){
        std::time_t t = std::time(NULL);
        struct std::tm when= *std::localtime(&t);
        std::get_time(&when,"%R");
        mHour=when.tm_hour;
        mMinutes=when.tm_min;
        mSeconds=when.tm_sec;
        invalidate(true);
        postDelayed(mRunner,800);
    };
    postDelayed(mRunner,800);
}

void AnalogClock::onDraw(Canvas&canvas){
    View::onDraw(canvas);
    int x=getWidth()/2;
    int y=getHeight()/2;
    int w,h;
    if(mDial){
        mDial->setBounds(0,0,getWidth(),getHeight());
        mDial->draw(canvas);
    }
    canvas.translate(x,y);
    if(mHourHand){
        canvas.save();
        canvas.rotate_degrees(360.f*mHour / 12.0f );
        w = mHourHand->getIntrinsicWidth();
        h = mHourHand->getIntrinsicHeight();
        mHourHand->setBounds(-5,-4,10,80);
        mHourHand->draw(canvas);
        canvas.restore();
    }
    if(mMinuteHand){
        canvas.save();
        canvas.rotate_degrees(360.f*mMinutes / 60.f );
        w = mHourHand->getIntrinsicWidth();
        h = mHourHand->getIntrinsicHeight();
        mHourHand->setBounds(-5,-4,10,90);
        mHourHand->draw(canvas);
        canvas.restore();
    }
    if(mSecondHand){
        canvas.save();
        
        w = mSecondHand->getIntrinsicWidth();
        h = mSecondHand->getIntrinsicHeight();
        //canvas.rotate(360.f*mSeconds / 60.f,0.5f*w,h);
        canvas.rotate_degrees(360.f*mSeconds / 60.0f );
        mSecondHand->setBounds(-5,-4,10,100);//x - (w / 2), y - (h / 2),w, h);
        mSecondHand->draw(canvas);
        canvas.restore();
    }
}

}
