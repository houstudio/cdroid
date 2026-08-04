/*********************************************************************************
 * navigation-fragment MVP demo: NavHostFragment (hosted in a FragmentActivity) +
 * a NavGraph whose destinations are FragmentNavigator.Destination entries mapping
 * route -> Fragment class. setGraph navigates to "a" (AFrag); any key navigates to
 * "b" (BFrag) via FragmentNavigator -> FragmentTransaction.replace. Verifies that
 * navigation drives Fragment swaps end-to-end.
 *********************************************************************************/
#include <cdroid.h>
#include <navigation/navhostfragment.h>
#include <navigation/fragmentnavigator.h>
#include <navigation/navgraph.h>
#include <navigation/navcontroller.h>
#include <navigation/navigatorprovider.h>
#include <fragment/fragment.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenttransaction.h>
#include <fragment/fragmentactivity.h>
#include <fragment/fragmentfactory.h>
#include <widget/textview.h>
#include <view/keyevent.h>
#include <porting/cdlog.h>

using cdroid::fragment::Fragment;
using cdroid::fragment::FragmentActivity;

class AFrag : public Fragment{
public:
    cdroid::View* onCreateView(cdroid::LayoutInflater*, cdroid::ViewGroup*, cdroid::Bundle*) override{
        cdroid::TextView* tv = new cdroid::TextView("Fragment A\n(press a key -> B)", 800, 600);
        tv->setTextColor(0xFFFFFFFF);
        tv->setBackgroundColor(0xFF102030);
        return tv;
    }
};
REGISTER_FRAGMENT(AFrag);

class BFrag : public Fragment{
public:
    cdroid::View* onCreateView(cdroid::LayoutInflater*, cdroid::ViewGroup*, cdroid::Bundle*) override{
        cdroid::TextView* tv = new cdroid::TextView("Fragment B", 800, 600);
        tv->setTextColor(0xFFFFFFFF);
        tv->setBackgroundColor(0xFF301020);
        return tv;
    }
};
REGISTER_FRAGMENT(BFrag);

class NavDemoWindow : public FragmentActivity{
    cdroid::NavHostFragment* mNavHost;
    bool mGraphBuilt = false;
public:
    NavDemoWindow() : FragmentActivity(0, 0, -1, -1){
        LOGD("[nav_frag_demo] NavDemoWindow ctor start");
        setFocusable(true);
        setClickable(true);
        mNavHost = new cdroid::NavHostFragment();
        LOGD("[nav_frag_demo] NavHostFragment created");
        getSupportFragmentManager()->beginTransaction()
            ->replace(getFragmentContainerId(), mNavHost)
            .commit();
        LOGD("[nav_frag_demo] NavDemoWindow ctor done (commit returned)");
    }
    void onResume() override{
        LOGD("[nav_frag_demo] onResume ENTER");
        FragmentActivity::onResume();
        if(mGraphBuilt) return;
        mGraphBuilt = true;
        cdroid::NavController* nc = mNavHost->getNavController();
        if(!nc) return;
        cdroid::NavGraph* graph = new cdroid::NavGraph(nc->getNavigatorProvider());
        cdroid::FragmentNavigator* fn = dynamic_cast<cdroid::FragmentNavigator*>(
            nc->getNavigatorProvider()->getNavigator("fragment"));
        if(fn){
            auto* a = dynamic_cast<cdroid::FragmentNavigator::Destination*>(fn->createDestination());
            a->setRoute("a"); a->setClassName("AFrag");
            auto* b = dynamic_cast<cdroid::FragmentNavigator::Destination*>(fn->createDestination());
            b->setRoute("b"); b->setClassName("BFrag");
            graph->addDestination(a);
            graph->addDestination(b);
            graph->setStartDestination("a");
        }
        LOGD("[nav_frag_demo] setGraph (expect AFrag)");
        mNavHost->setGraph(graph);
        LOGD("[nav_frag_demo] navigate('b') (expect BFrag)");
        mNavHost->getNavController()->navigate("b");
        LOGD("[nav_frag_demo] popBackStack (expect AFrag)");
        mNavHost->getNavController()->popBackStack();
        LOGD("[nav_frag_demo] navigate/pop sequence done");
    }
    bool onTouchEvent(MotionEvent&e)override{
        LOGD("%.f,%.f",e.getX(),e.getY());
        return FragmentActivity::onTouchEvent(e);
    }
    bool performClick()override{
        cdroid::NavController* nc = mNavHost->getNavController();
        if(nc){
            LOGD("[nav_frag_demo] navigate('b') (expect BFrag)");
            nc->navigate("b");
        }
    }
    bool onKeyDown(int keyCode, cdroid::KeyEvent& /*evt*/) override{
        cdroid::NavController* nc = mNavHost->getNavController();
        LOGD("recv keycode %d",keyCode);
        if(nc){
            LOGD("[nav_frag_demo] navigate('b') (expect BFrag)");
            nc->navigate("b");
        }
        return true;
    }
};

int main(int argc, const char* argv[]){
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    LOGD("[nav_frag_demo] main ENTER (before App)");
    cdroid::App app(argc, argv);
    LOGD("[nav_frag_demo] main: App constructed, before NavDemoWindow");
    new NavDemoWindow();
    LOGD("[nav_frag_demo] main: NavDemoWindow created, calling exec");
    return app.exec();
}
