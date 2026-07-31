/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <navigation/navhostfragment.h>
#include <navigation/fragmentnavigator.h>
#include <navigation/navigatorprovider.h>
#include <navigation/navgraph.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmentfactory.h>
#include <widget/framelayout.h>

namespace cdroid{

NavHostFragment::NavHostFragment(const std::string& graphRef) : mGraphRef(graphRef){}

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

void NavHostFragment::onResume(){
    fragment::Fragment::onResume();
    // Auto-apply the graph captured at construction, auto-navigating to its startDestination
    // (androidx reads app:navGraph in onInflate and calls setGraph in onCreate). CDROID attaches
    // a child Fragment's View eagerly, so the host container must already exist — resume is the
    // earliest guaranteed point (performResume runs after the View is created). performResume then
    // dispatches the child FragmentManager, catching the start Fragment up to RESUMED.
    if(!mGraphRef.empty() && !mGraphLoaded && mNavController){
        mGraphLoaded = true;
        mNavController->setGraph(mGraphRef);
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
