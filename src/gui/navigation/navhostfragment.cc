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
#include <navigation/navigation.h>
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
    // Apply the graph captured at construction, auto-navigating to its startDestination (androidx
    // reads app:navGraph in onInflate and calls setGraph in onCreate). setGraph -> navigate ->
    // FragmentNavigator.navigate -> child-FM commit is now DEFERRED: it posts to the host Handler
    // and runs on the next main-loop iteration, by which time onCreateView has built the child
    // container (the parent drives this fragment CREATED -> VIEW_CREATED -> ... synchronously
    // inside its moveToExpectedState, before the looper runs the posted commit).
    if(!mGraphRef.empty() && !mGraphLoaded && mNavController){
        mGraphLoaded = true;
        mNavController->setGraph(mGraphRef);
    }
}

void NavHostFragment::onResume(){
    fragment::Fragment::onResume();
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

void NavHostFragment::onViewCreated(View* view, Bundle* savedInstanceState){
    fragment::Fragment::onViewCreated(view, savedInstanceState);
    // Publish this host's NavController on the host View so any descendant — e.g. a button
    // inside a destination Fragment swapped in by FragmentNavigator — resolves it through
    // Navigation::findNavController(view) (androidx NavHostFragment.onViewCreated).
    Navigation::setViewNavController(view, mNavController);
    // Also publish it on any ancestor whose id equals this NavHost's id. When the host sits in
    // a container tagged with the fragment's id (the XML/FragmentContainerView and programmatic
    // add cases), this lets callers outside the host's own subtree — siblings of the NavHost —
    // find the same controller. The view is already attached to its container by this point
    // (FragmentStateManager addView runs before performViewCreated), so the parent chain is live.
    ViewGroup* parent = view->getParent();
    while(parent != nullptr){
        if(parent->getId() == getId()){
            Navigation::setViewNavController(parent, mNavController);
        }
        parent = parent->getParent();
    }
}

void NavHostFragment::onDestroyView(){
    // Tear down the tags published in onViewCreated: clear the controller from any id-matching
    // ancestor, but only when that ancestor still carries *this* controller (a newer host may
    // have superseded it). The host View's own tag is dropped together with the View. Note the
    // CDROID state machine detaches the view (removeView) before performDestroyView, so by here
    // getView()->getParent() is usually null and this walk is a harmless no-op — matching the
    // androidx shape rather than its exact firing point.
    View* view = getView();
    if(view != nullptr){
        ViewGroup* parent = view->getParent();
        while(parent != nullptr){
            if(parent->getId() == getId()
                    && Navigation::getViewNavController(parent) == mNavController){
                Navigation::setViewNavController(parent, nullptr);
            }
            parent = parent->getParent();
        }
    }
    fragment::Fragment::onDestroyView();
}

void NavHostFragment::setGraph(NavGraph* graph){
    if(mNavController && graph) mNavController->setGraph(graph);
}

}//namespace cdroid
