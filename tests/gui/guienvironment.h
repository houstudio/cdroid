#ifndef __GUI_ENVIRONMENT_H__
#define __GUI_ENVIRONMENT_H__
#include <gtest/gtest.h>
#include <core/app.h>
#include <core/looper.h>
#include <core/messagequeue.h>
#include <core/systemclock.h>
#include <widget/cdwindow.h>
#include <widget/framelayout.h>
#include <widget/linearlayout.h>
#include <view/gravity.h>
#include <porting/cdlog.h>
#include <unistd.h>
#include <limits.h>
#include <string>
#include <vector>

class GUIEnvironment: public testing::Environment{
private:
    int argc;
    const char**argv;
    static GUIEnvironment*mInst;
    static cdroid::Window*       mStage;  // the one shared Window (full screen)
    static cdroid::LinearLayout* mPanel;  // left results pane; interior built in testmain
    static cdroid::ViewGroup*    mContent;// right test-screen pane
public:
    GUIEnvironment(int c,const char*v[]):argc(c),argv(v){
        mInst=this;
    }
    void SetUp()override{
        /* Ensure cdroid.pak is loaded from the test output root when the
           binary is run from a different working directory. */
        if(argc > 0 && argv[0]){
            char binaryPath[PATH_MAX];
            if(realpath(argv[0], binaryPath)){
                std::string path(binaryPath);
                auto pos = path.find_last_of('/');
                if(pos != std::string::npos) path = path.substr(0, pos);
                while(!path.empty()){
                    std::string candidate = path + "/cdroid.pak";
                    if(access(candidate.c_str(), F_OK) == 0){
                        chdir(path.c_str());
                        break;
                    }
                    pos = path.find_last_of('/');
                    if(pos == std::string::npos) break;
                    path = path.substr(0, pos);
                }
            }
        }
        new cdroid::App(argc, argv);
        /* The single shared Window ("stage"). Tests add their views to its
           content area (see content()), not to the Window directly. -1/-1 =
           the display's full size (see Window ctor). */
        mStage = new cdroid::Window(0, 0, -1, -1);

        /* Root tree — side-by-side panes. A DrawerLayout drawer used to
           overlay the test screen, hiding the UI under inspection; a
           permanent split keeps both visible at once:
             Window
             └─ LinearLayout (horizontal)
                ├─ results LinearLayout (fixed 320px) = left pane
                └─ content  FrameLayout  (weight 1)   = right pane, test screen
           The results interior (header + suite list + detail) is built lazily
           by GuiTestListener in testmain.cc. */
        cdroid::LinearLayout* root = new cdroid::LinearLayout(&cdroid::App::getInstance());
        root->setOrientation(cdroid::LinearLayout::HORIZONTAL);

        mPanel = new cdroid::LinearLayout(&cdroid::App::getInstance());
        mPanel->setOrientation(cdroid::LinearLayout::VERTICAL);
        const int panelWidth = 320; // tweakable
        root->addView(mPanel, 0,
            new cdroid::LinearLayout::LayoutParams(panelWidth, -1));

        mContent = new cdroid::FrameLayout(&cdroid::App::getInstance());
        mContent->setBackgroundColor(0xFF23282E); // the "canvas": distinct from the dark panel
        root->addView(mContent, 1,
            new cdroid::LinearLayout::LayoutParams(0, -1, 1.0f));

        mStage->addView(root);
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
    /* Where every test case adds its views (the right pane). Cleared between
       cases by the listener — the results panel is a sibling, so it is left
       untouched. */
    static cdroid::ViewGroup*content(){
        return mContent;
    }
    /* The left results pane (its interior is populated by the listener). */
    static cdroid::LinearLayout*panel(){
        return mPanel;
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
        if(lp->pollOnce(5) == cdroid::Looper::POLL_TIMEOUT){
            /* POLL_TIMEOUT only says THIS 5ms poll expired — a message due a
               few ms out (Choreographer posts doFrame via sendMessageAtTime,
               so a scheduled traversal is a delayed message) still counts as
               pending work: pollOnce would time out again while it matures.
               Idle = the queue has nothing scheduled at all; otherwise grind
               with pollAll(1), which dispatches due messages as they mature.
               Careful: any self-reposting delayed message (e.g. scrollbar
               fade: awakenScrollBars posts a ~1.6s runner and every scroll
               re-arms it) keeps the queue non-empty forever — keep test
               trees free of those (the harness disables its own scrollbars),
               or use pumpUntil(pred). */
            cdroid::Message* head = lp->getQueue()->peek();
            if(head == nullptr) break;
            lp->pollAll(1);
        }
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
