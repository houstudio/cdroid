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
    DECLARE_UIEVENT(const std::string,Formatter,int64_t);
private:
    int64_t mBase;
    int64_t mNow; // the currently displayed time
    bool mVisible;
    bool mStarted;
    bool mRunning;
    bool mLogged;
    bool mColonBlinking;
    Runnable mTickRunnable;
    std::string mFormat;
    Formatter mFormatter;
    OnChronometerTickListener mOnChronometerTickListener;
    bool mCountDown;
private:
    void init();
    void tickRunner();
    void updateText(int64_t now);
    void updateRunning();
    static std::string formatDuration(int64_t ms);
protected:
    void dispatchChronometerTick();
public:
    Chronometer(int w,int h);
    Chronometer(Context*ctx,const AttributeSet&);
    void setCountDown(bool countDown);
    bool isCountDown()const;
    bool isTheFinalCountDown()const;
    void setBase(int64_t base);
    int64_t getBase()const;
    void setFormat(const std::string& format);
    std::string getFormat()const;
    void setOnChronometerTickListener(OnChronometerTickListener listener);
    OnChronometerTickListener getOnChronometerTickListener()const;
    void start();
    void stop();
    void setStarted(bool started);
    std::string getContentDescription()const override;
    std::string getAccessibilityClassName()const override;
};
}//namespace


#endif
