#ifndef __GUI_ENVIRONMENT_H__
#define __GUI_ENVIRONMENT_H__
#include <gtest/gtest.h>
#include <core/app.h>
#include <core/looper.h>
#include <core/systemclock.h>
#include <widget/cdwindow.h>
#include <porting/cdlog.h>

class GUIEnvironment: public testing::Environment{
private:
    int argc;
    const char**argv;
    static GUIEnvironment*mInst;
    static cdroid::Window*mStage;
public:
    GUIEnvironment(int c,const char*v[]):argc(c),argv(v){
        mInst=this;
    }
    void SetUp()override{
        /* The single App that backs every GUI test. Intentionally never
           deleted: App registers an AtExit callback capturing `this`, so it
           must outlive AtExit teardown. */
        new cdroid::App(argc, argv);
        /* The single shared Window ("stage"). Tests add their views to it
           instead of creating a Window per case. width/height -1 = the
           display's full size (see Window ctor). */
        mStage = new cdroid::Window(0, 0, -1, -1);
        printf("GUIEnvironment Setup\r\n");
    }
    void TearDown()override{
        printf("GUIEnvironment TearDown\r\n");
    }
    int getArgc()const{
        return argc;
    }
    const char**getArgv()const{
        return argv;
    }
    static GUIEnvironment*getInstance(){
        return mInst;
    }
    /* The one shared Window. Add views to it; it is cleared between cases by
       the StageResetListener registered in main(). */
    static cdroid::Window*stage(){
        return mStage;
    }
};

/* Drive the shared main Looper for a bounded wall-clock duration.
   Unlike App::exec(), pumpFor() always returns, so gtest can move on to the
   next case. Call from the test body on the main thread only. */
inline void pumpFor(int ms){
    cdroid::Looper*lp = cdroid::Looper::getMainLooper();
    if(!lp) return;
    cdroid::nsecs_t end = cdroid::SystemClock::uptimeMillis() + ms;
    do{ lp->pollAll(1); }while(cdroid::SystemClock::uptimeMillis() < end);
}

/* Pump a fixed number of frames — deterministic, handy for animation tests. */
inline void pumpFrames(int frames,int frameMs=16){
    cdroid::Looper*lp = cdroid::Looper::getMainLooper();
    if(!lp) return;
    for(int i=0;i<frames;i++) lp->pollAll(frameMs);
}

/* Pump until the Looper goes idle (a short pollOnce times out with no pending
   work) or maxMs elapses. Good for draining one-shot work a case posted
   (layout, inflators). Note: continuous work (animations/Choreographer) never
   goes idle, so it runs the full maxMs — use pumpUntil(pred) for those. */
inline void pumpUntilIdle(int maxMs=1000){
    cdroid::Looper*lp = cdroid::Looper::getMainLooper();
    if(!lp) return;
    cdroid::nsecs_t end = cdroid::SystemClock::uptimeMillis() + maxMs;
    while(cdroid::SystemClock::uptimeMillis() < end){
        if(lp->pollOnce(5) == cdroid::Looper::POLL_TIMEOUT) break;
    }
}

/* Pump until pred() is true or maxMs elapses. The event-driven replacement for
   a magic pumpFor(N): terminate on a real completion signal (animation end,
   counter reached, flag set, ...). */
template<class Pred>
inline void pumpUntil(Pred pred,int maxMs=2000){
    cdroid::Looper*lp = cdroid::Looper::getMainLooper();
    if(!lp || pred()) return;
    cdroid::nsecs_t end = cdroid::SystemClock::uptimeMillis() + maxMs;
    do{ lp->pollAll(1); }while(!pred() && cdroid::SystemClock::uptimeMillis() < end);
}
#endif/*__GUI_ENVIRONMENT_H__*/
