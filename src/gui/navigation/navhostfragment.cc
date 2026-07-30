#include <navigation/navhostfragment.h>
#include <navigation/fragmentnavigator.h>
#include <navigation/navigatorprovider.h>
#include <navigation/navgraph.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmentfactory.h>
#include <widget/framelayout.h>

namespace cdroid{

NavHostFragment::NavHostFragment(){}

NavHostFragment::~NavHostFragment(){
    delete mNavController;
}

void NavHostFragment::onCreate(Bundle* savedInstanceState){
    fragment::Fragment::onCreate(savedInstanceState);
    mNavController = onCreateNavController();
    if(mNavController){
        mNavController->setLifecycleOwner(this);
        // Register the FragmentNavigator bound to this host's child FragmentManager.
        FragmentNavigator* fragNav = new FragmentNavigator(getChildFragmentManager(), getId());
        mNavController->getNavigatorProvider()->addNavigator(fragNav);
    }
}

NavController* NavHostFragment::onCreateNavController(){
    return new NavController(getContext());
}

cdroid::View* NavHostFragment::onCreateView(cdroid::LayoutInflater* /*inflater*/,
                                            cdroid::ViewGroup* /*container*/,
                                            cdroid::Bundle* /*savedInstanceState*/){
    // The NavHost's own view is a FrameLayout that serves as the container for the
    // child Fragments FragmentNavigator swaps in (identified by this Fragment's id).
    cdroid::FrameLayout* view = new cdroid::FrameLayout(-1, -1);
    view->setId(getId());
    return view;
}

void NavHostFragment::setGraph(NavGraph* graph){
    if(mNavController && graph) mNavController->setGraph(graph);
}

}//namespace cdroid
