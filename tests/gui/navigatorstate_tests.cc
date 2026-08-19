/*********************************************************************************
 * NavigatorState tests — port of androidx.navigation navigation-testing
 * TestNavigatorStateTest.testLifecycle, adapted to CDROID's NavigatorState.
 *
 * CDROID has no standalone TestNavigatorState (its NavigatorState is constructed
 * with a live NavController and routes push/pop through controller handlers that
 * are installed only during navigate()/popBackStack()), so the standalone
 * navigator.navigate(listOf(entry)) flow is replaced by the same transition
 * sequence driven through NavController::navigate()/popBackStack().
 *
 * Ported: getOrCreateNavigatorState attaches the navigator (stable instance),
 * createBackStackEntry starts INITIALIZED, and the back-stack lifecycle chain
 * top RESUMED / covered entry CREATED / re-exposed RESUMED after pop.
 *
 * Skipped: the popped entry's DESTROYED assertion (CDROID deletes the entry
 * synchronously in popEntryFromBackStack — reading it after pop is a UAF),
 * FloatingWindow/SupportingPane/transition variants (not ported).
 *********************************************************************************/
#include <gtest/gtest.h>
#include <core/app.h>
#include <lifecycle/lifecycle.h>
#include <navigation/navcontroller.h>
#include <navigation/navigatorprovider.h>
#include <navigation/navgraph.h>
#include <navigation/navdestination.h>
#include <navigation/navigatorstate.h>
#include "navtestnavigator.h"

using namespace cdroid;

namespace {
using State = lifecycle::Lifecycle::State;

NavController* makeController(TestNavigator** outNav = nullptr) {
    NavController* nc = new NavController(&App::getInstance());
    TestNavigator* nav = new TestNavigator();
    if(outNav) *outNav = nav;
    nc->getNavigatorProvider()->addNavigator(nav);
    NavGraph* graph = new NavGraph(nc->getNavigatorProvider());
    auto addDest = [&](const std::string& route){
        NavDestination* d = nav->createDestination();
        d->setRoute(route);
        graph->addDestination(d);
    };
    addDest("start");
    addDest("b");
    graph->setStartDestination("start");
    nc->setGraph(graph);
    return nc;
}

// Current lifecycle state of the back-stack entry for `route`, or State::DESTROYED
// when no such entry is on the stack.
State stateOf(NavController* nc, const std::string& route) {
    for(NavBackStackEntry* e : nc->getBackStack())
        if(e->getDestination() && e->getDestination()->getRoute() == route)
            return e->getLifecycle().getCurrentState();
    return State::DESTROYED; // sentinel: not on the stack
}
} // namespace

TEST(NavigatorState, AttachOnSetGraph) {
    TestNavigator* nav = nullptr;
    makeController(&nav); // setGraph attaches every navigator
    EXPECT_TRUE(nav->isAttached());
    EXPECT_NE(nav->getState(), nullptr);
}

TEST(NavigatorState, CreateBackStackEntryStartsInitialized) {
    TestNavigator* nav = nullptr;
    makeController(&nav);
    NavigatorState* state = nav->getState();
    ASSERT_NE(state, nullptr);
    NavBackStackEntry* entry = state->createBackStackEntry(nav->createDestination(), nullptr);
    EXPECT_EQ(entry->getLifecycle().getCurrentState(), State::INITIALIZED);
}

TEST(NavigatorState, LifecycleThroughNavigation) {
    NavController* nc = makeController(); // setGraph -> start

    EXPECT_EQ(stateOf(nc, "start"), State::RESUMED);

    nc->navigate("b");
    EXPECT_EQ(stateOf(nc, "b"),     State::RESUMED);
    EXPECT_EQ(stateOf(nc, "start"), State::CREATED);

    nc->popBackStack();
    EXPECT_EQ(stateOf(nc, "start"), State::RESUMED);
    EXPECT_EQ(stateOf(nc, "b"),     State::DESTROYED); // popped: no longer on the stack
}
