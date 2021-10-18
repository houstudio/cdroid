/*************************************************************************
	> File Name: chronometer.h
	> Author: 
	> Mail: 
	> Created Time: Sat 15 May 2021 02:44:22 AM UTC
 ************************************************************************/

#ifndef __CHRONOMETER_H__
#define __CHRONOMETER_H__
#include <widget/textview.h>
namespace cdroid{
class Chronometer:public TextView{
public:
    DECLARE_UIEVENT(void,OnChronometerTickListener,Chronometer&);
    DECLARE_UIEVENT(const std::string,Formatter,long);
private:
    long mBase;
    long mNow; // the currently displayed time
    bool mVisible;
    bool mStarted;
    bool mRunning;
    bool mLogged;
    Runnable mTickRunnable;
    std::string mFormat;
    Formatter mFormatter;
    OnChronometerTickListener mOnChronometerTickListener;
    bool mCountDown;
private:
    void tickRunner();
    void updateText(long now);
    void updateRunning();
    static std::string formatDuration(long ms);
protected:
    void dispatchChronometerTick();
public:
    Chronometer(int w,int h);
    Chronometer(Context*ctx,const AttributeSet&,const std::string&defstyle=nullptr);
    void setCountDown(bool countDown);
    bool isCountDown()const;
    bool isTheFinalCountDown()const;
    void setBase(long base);
    long getBase()const;
    void setFormat(const std::string& format);
    std::string getFormat()const;
    void setOnChronometerTickListener(OnChronometerTickListener listener);
    OnChronometerTickListener getOnChronometerTickListener()const;
    void start();
    void stop();
    void setStarted(bool started);
    std::string getContentDescription()const;
};
}//namespace


#endif
