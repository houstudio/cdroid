/*********************************************************************************
 * navigation-fragment MVP demo: NavHostFragment (hosted in a FragmentWindow) +
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
#include <fragment/fragmentwindow.h>
#include <fragment/fragmentfactory.h>
#include <widget/textview.h>
#include <view/keyevent.h>
#include <porting/cdlog.h>

using cdroid::fragment::Fragment;
using cdroid::fragment::FragmentWindow;

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

class NavDemoWindow : public FragmentWindow{
    cdroid::NavHostFragment* mNavHost;
    bool mGraphBuilt = false;
public:
    NavDemoWindow() : FragmentWindow(0, 0, -1, -1){
        mNavHost = new cdroid::NavHostFragment();
        getSupportFragmentManager()->beginTransaction()
            ->replace(getFragmentContainerId(), mNavHost)
            .commit();
    }
    void onActive() override{
        FragmentWindow::onActive();
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
    bool onKeyDown(int /*keyCode*/, cdroid::KeyEvent& /*evt*/) override{
        cdroid::NavController* nc = mNavHost->getNavController();
        if(nc){
            LOGD("[nav_frag_demo] navigate('b') (expect BFrag)");
            nc->navigate("b");
        }
        return true;
    }
};

int main(int argc, const char* argv[]){
    cdroid::App app(argc, argv);
    new NavDemoWindow();
    return app.exec();
}
