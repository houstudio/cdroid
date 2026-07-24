#ifndef __GUI_ENVIRONMENT_H__
#define __GUI_ENVIRONMENT_H__
#include <gtest/gtest.h>
#include <core/app.h>
#include <core/looper.h>
#include <core/systemclock.h>
#include <widget/cdwindow.h>
#include <widget/drawerlayout.h>
#include <widget/framelayout.h>
#include <widget/linearlayout.h>
#include <view/gravity.h>
#include <porting/cdlog.h>

class GUIEnvironment: public testing::Environment{
private:
    int argc;
    const char**argv;
    static GUIEnvironment*mInst;
    static cdroid::Window*       mStage;       // the one shared Window (full screen)
    static cdroid::DrawerLayout* mDrawerLayout;// root: holds content + results drawer
    static cdroid::ViewGroup*    mContent;     // test-screen area (DrawerLayout content)
    static cdroid::LinearLayout* mDrawerPanel; // START drawer host; interior built in testmain
public:
    GUIEnvironment(int c,const char*v[]):argc(c),argv(v){
        mInst=this;
    }
    void SetUp()override{
        /* The single App that backs every GUI test. Intentionally never
           deleted: App registers an AtExit callback capturing `this`, so it
           must outlive AtExit teardown. */
        new cdroid::App(argc, argv);
        /* The single shared Window ("stage"). Tests add their views to its
           content area (see content()), not to the Window directly. -1/-1 =
           the display's full size (see Window ctor). */
        mStage = new cdroid::Window(0, 0, -1, -1);

        /* Root tree:
             Window
             └─ DrawerLayout
                ├─ content FrameLayout (gravity NO_GRAVITY) = "test screen"
                └─ drawer  LinearLayout (gravity START)      = results panel
           DrawerLayout drawers overlay the content (they don't shrink it): the
           drawer is opened to read results and closed for a full-screen test.
           The drawer interior (header + suite list + detail) is built lazily by
           GuiTestListener in testmain.cc. */
        mDrawerLayout = new cdroid::DrawerLayout(1, 1);

        mContent = new cdroid::FrameLayout(1, 1);
        mContent->setBackgroundColor(0xFF23282E); // the "canvas": distinct from the dark drawer
        mDrawerLayout->addView(mContent, 0,
            new cdroid::DrawerLayout::LayoutParams(-1, -1, cdroid::Gravity::NO_GRAVITY));

        mDrawerPanel = new cdroid::LinearLayout(1, 1);
        mDrawerPanel->setOrientation(cdroid::LinearLayout::VERTICAL);
        const int drawerWidth = 320; // tweakable
        mDrawerLayout->addView(mDrawerPanel, 1,
            new cdroid::DrawerLayout::LayoutParams(drawerWidth, -1, cdroid::Gravity::START));

        mStage->addView(mDrawerLayout);
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
    /* The one shared Window. */
    static cdroid::Window*stage(){
        return mStage;
    }
    /* Where every test case adds its views (the DrawerLayout "content"). Cleared
       between cases by the listener — the results drawer is a sibling, so it is
       left untouched. */
    static cdroid::ViewGroup*content(){
        return mContent;
    }
    /* The DrawerLayout itself, for open/closeDrawer(). */
    static cdroid::DrawerLayout*drawerLayout(){
        return mDrawerLayout;
    }
    /* The START drawer panel (its interior is populated by the listener). */
    static cdroid::LinearLayout*drawerPanel(){
        return mDrawerPanel;
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
