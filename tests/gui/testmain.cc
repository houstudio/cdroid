#include <string>
#include <guienvironment.h>
#include <porting/cdlog.h>
#include <core/windowmanager.h>
#include <widget/cdwindow.h>
#include <widget/textview.h>

namespace cdroid{ class Window; }

GUIEnvironment*GUIEnvironment::mInst=nullptr;
cdroid::Window*GUIEnvironment::mStage=nullptr;

/* ---- persistent result panel: one overlay window, updated per case ---- */
namespace {
    cdroid::Window*   gResultWin =nullptr;
    cdroid::TextView* gResultView=nullptr;
    int gPassed=0, gFailed=0;
    std::string gLog;                 // recent result lines (tail-bounded)

    void ensureResultPanel(){
        if(gResultWin) return;
        cdroid::Window*stage=GUIEnvironment::stage();
        int w = stage ? stage->getWidth()  : 1280;
        int h = stage ? stage->getHeight() : 720;
        gResultWin  = new cdroid::Window(0, 0, w, h/4);
        gResultView = new cdroid::TextView("", w, h/4);
        gResultView->setTextSize(20);
        gResultView->setTextColor(0xFFFFFFFF);
        gResultView->setBackgroundColor(0xCC000000);
        gResultView->setPadding(8, 4, 8, 4);
        gResultWin->addView(gResultView);
    }
    std::string summary(){
        return "[ Passed:"+std::to_string(gPassed)
             +"  Failed:"+std::to_string(gFailed)
             +"  Total:"+std::to_string(gPassed+gFailed)+" ]";
    }
}

/* One listener does three things at OnTestEnd:
   1. record this case's result onto the panel;
   2. reset the stage (clear its content, drop stray windows) so the next case
      starts clean — keeping the stage and the result panel;
   3. pump a frame so the panel repaints. */
class GuiTestListener:public testing::EmptyTestEventListener{
public:
    void OnTestProgramStart(const testing::UnitTest&)override{
        ensureResultPanel();
    }
    void OnTestEnd(const testing::TestInfo&info)override{
        ensureResultPanel();
        bool failed = info.result()->Failed();
        if(failed) gFailed++; else gPassed++;
        gLog += std::string(failed ? "x " : "+ ")
             + info.test_suite_name() + "." + info.name() + "\n";
        if(gLog.size() > 2000) gLog = gLog.substr(gLog.size() - 2000); // tail
        gResultView->setText(summary() + "\n" + gLog);

        cdroid::Window*stage = GUIEnvironment::stage();
        if(stage) stage->removeAllViews();
        std::vector<cdroid::Window*>wins;
        cdroid::WindowManager&wm = cdroid::WindowManager::getInstance();
        if(wm.getWindows(wins) > 0){
            for(auto* w : wins) if(w != stage && w != gResultWin) wm.removeWindow(w);
        }
        wm.bringToFront(gResultWin);
        pumpFor(20);
    }
};

int main(int argc,char*argv[])
{
    LogParseModules(argc,(const char**)argv);
    testing::InitGoogleTest(&argc,argv);
    ::testing::AddGlobalTestEnvironment(new GUIEnvironment(argc, (const char**)argv));
    ::testing::UnitTest::GetInstance()->listeners().Append(new GuiTestListener);
    return RUN_ALL_TESTS();
}
