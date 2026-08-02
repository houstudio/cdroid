/*********************************************************************************
 * NavController tests — port of androidx NavControllerTest core cluster (the subset
 * CDROID implements: route navigate/pop, singleTop, popUpTo, destination-changed listener,
 * back-stack size). Uses a programmatic NavGraph + TestNavigator (no XML/Activity needed).
 *
 * androidx invariant mirrored here: the back stack lives on the NavController (not the
 * navigator's NavigatorState, which is dead code in CDROID), so sizes are read via
 * NavController::getBackStack().
 *********************************************************************************/
#include <gtest/gtest.h>
#include <core/app.h>
#include <navigation/navcontroller.h>
#include <navigation/navigatorprovider.h>
#include <navigation/navgraph.h>
#include <navigation/navdestination.h>
#include <navigation/navoptions.h>
#include <navigation/navbackstackentry.h>
#include <lifecycle/lifecycle.h>
#include "navtestnavigator.h"

using namespace cdroid;

namespace {

// Build a NavController whose provider has a "test" navigator, with a graph of routes
// {start, b, c} and startDestination "start", then setGraph (auto-navigates to start).
// Heap-allocated; leaked on purpose (test process exits).
NavController* makeController() {
    NavController* nc = new NavController(&App::getInstance());
    TestNavigator* nav = new TestNavigator();
    nc->getNavigatorProvider()->addNavigator(nav);
    NavGraph* graph = new NavGraph(nc->getNavigatorProvider());
    auto addDest = [&](const std::string& route){
        NavDestination* d = nav->createDestination();
        d->setRoute(route);
        graph->addDestination(d);
    };
    addDest("start");
    addDest("b");
    addDest("c");
    graph->setStartDestination("start");
    nc->setGraph(graph);
    return nc;
}

// Records routes visited via an OnDestinationChangedListener callback.
struct DestRecorder {
    std::vector<std::string> routes;
};

} // namespace

TEST(NavController, Navigate) {
    NavController* nc = makeController();
    ASSERT_NE(nc->getCurrentDestination(), nullptr);
    EXPECT_EQ(nc->getCurrentDestination()->getRoute(), "start");
    EXPECT_GE(nc->getBackStack().size(), 1u);   // start entry pushed on setGraph

    nc->navigate("b");
    EXPECT_EQ(nc->getCurrentDestination()->getRoute(), "b");
    // navigating pushes an entry: back stack grew (start -> start,b leaf entries, plus graph).
    auto sizeAfterB = nc->getBackStack().size();
    EXPECT_GT(sizeAfterB, 1u);
}

TEST(NavController, PopRoot) {
    NavController* nc = makeController();
    ASSERT_NE(nc->getCurrentDestination(), nullptr);
    EXPECT_EQ(nc->getCurrentDestination()->getRoute(), "start");
    // Pop everything off the stack (CDROID V2 keeps a graph entry, so loop until pop returns
    // false rather than assuming a single pop empties to null).
    while(nc->popBackStack()) { /* drain */ }
    // At the root / empty stack, further pops return false (androidx testPopOnEmptyStack).
    EXPECT_FALSE(nc->popBackStack());
    EXPECT_FALSE(nc->popBackStack());
}

TEST(NavController, NavigateThenPop) {
    NavController* nc = makeController();
    nc->navigate("b");
    EXPECT_EQ(nc->getCurrentDestination()->getRoute(), "b");
    EXPECT_TRUE(nc->popBackStack());
    EXPECT_EQ(nc->getCurrentDestination()->getRoute(), "start");
}

TEST(NavController, PopToUnknownDestination) {
    NavController* nc = makeController();
    nc->navigate("b");
    size_t before = nc->getBackStack().size();
    EXPECT_FALSE(nc->popBackStack(std::string("nonexistent"), false, false));
    EXPECT_EQ(nc->getBackStack().size(), before); // unchanged
    EXPECT_EQ(nc->getCurrentDestination()->getRoute(), "b");
}

TEST(NavController, NavigateWithPopUpTo) {
    NavController* nc = makeController();
    nc->navigate("b");
    ASSERT_EQ(nc->getCurrentDestination()->getRoute(), "b");
    // popUpTo(start){inclusive=true} collapses the stack, then navigate to c -> single c on top.
    NavOptions* opts = NavOptions::Builder()
        .setPopUpTo(std::string("start"), true).build();
    nc->navigate("c", opts);
    delete opts;
    EXPECT_EQ(nc->getCurrentDestination()->getRoute(), "c");
    // The start/b entries were popped; popping c should leave an empty/stack-without-leaf.
    EXPECT_FALSE(nc->popBackStack(std::string("start"), false, false));
}

TEST(NavController, SingleTop) {
    NavController* nc = makeController();
    nc->navigate("b");
    size_t sizeBefore = nc->getBackStack().size();
    // singleTop navigating to the current destination must not grow the stack.
    NavOptions* opts = NavOptions::Builder().setLaunchSingleTop(true).build();
    nc->navigate("b", opts);
    delete opts;
    EXPECT_EQ(nc->getCurrentDestination()->getRoute(), "b");
    EXPECT_EQ(nc->getBackStack().size(), sizeBefore);
}

TEST(NavController, DestinationChangedListener) {
    NavController* nc = makeController();
    DestRecorder rec;
    NavController::OnDestinationChangedListener listener =
        [&rec](NavController*, NavDestination* destination, Bundle*) {
            if(destination) rec.routes.push_back(destination->getRoute());
        };
    nc->addOnDestinationChangedListener(listener);
    // Registering dispatches the current destination (androidx V7: immediate dispatch).
    nc->navigate("b");
    nc->removeOnDestinationChangedListener(listener);
    // Should have seen at least start (on register) and b (on navigate), in order.
    ASSERT_GE(rec.routes.size(), 2u);
    EXPECT_EQ(rec.routes.back(), "b");
    bool sawStart = false, sawB = false;
    for(const std::string& r : rec.routes){ if(r=="start") sawStart=true; if(r=="b") sawB=true; }
    EXPECT_TRUE(sawStart);
    EXPECT_TRUE(sawB);
}

TEST(NavController, TopEntryIsResumed) {
    NavController* nc = makeController();
    nc->navigate("b");
    ASSERT_FALSE(nc->getBackStack().empty());
    NavBackStackEntry* top = nc->getCurrentBackStackEntry();
    ASSERT_NE(top, nullptr);
    // androidx NavBackStackEntryLifecycleTest: only the top destination is RESUMED.
    EXPECT_EQ(top->getLifecycle().getCurrentState(), lifecycle::Lifecycle::State::RESUMED);
}
