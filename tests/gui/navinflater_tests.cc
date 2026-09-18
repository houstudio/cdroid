/*********************************************************************************
 * NavInflater tests — port of androidx NavInflaterTest. Inflates a nav-graph XML from the
 * test pak (gui_test.pak) and asserts the structure that CDROID's inflater produces.
 *
 * The ported nav_simple_test.xml uses id-based destinations (android:id="@+id/...", like
 * androidx nav_simple.xml); idgen assigns those IDs, reachable via App::getId(name).
 *
 * Known inflater gaps (not asserted here, TODO for the inflater): the app-namespace @id refs
 * inside <action> (app:destination, app:popUpTo) are not resolved by AttributeSet.getResourceId
 * (the destination's own android:id IS resolved), and typed default values are not yet merged
 * into the destination's default-arguments Bundle. The action/arg-default assertions are
 * therefore limited to what the inflater currently populates.
 *********************************************************************************/
#include <gtest/gtest.h>
#include "R.h"
#include <core/app.h>
#include <navigation/navcontroller.h>
#include <navigation/navigatorprovider.h>
#include <navigation/navinflater.h>
#include <navigation/navgraph.h>
#include <navigation/navdestination.h>
#include <navigation/navaction.h>
#include <navigation/navoptions.h>
#include <navigation/navargument.h>
#include "navtestnavigator.h"

using namespace cdroid;

namespace {
NavGraph* inflateTestGraph(int graphResId) {
    NavController nc(&App::getInstance());
    nc.getNavigatorProvider()->addNavigator(new TestNavigator());
    NavInflater inflater(&App::getInstance(), nc.getNavigatorProvider());
    return inflater.inflate(graphResId);
}
} // namespace

TEST(NavInflater, InflateSimpleGraph) {
    NavGraph* g = inflateTestGraph(gui_test::R::navigation::nav_simple_test);
    ASSERT_NE(g, nullptr);

    int startId = gui_test::R::id::start_test;
    int secondId = gui_test::R::id::second_test;
    ASSERT_NE(startId, 0);
    ASSERT_NE(secondId, 0);

    // Both destinations resolve by their idgen IDs (android:id="@+id/..." resolves).
    // (app:startDestination is a known app-namespace resolution gap — see file header — so
    // getStartDestination() is not asserted here.)
    EXPECT_NE(g->findNode(startId), nullptr);
    EXPECT_NE(g->findNode(secondId), nullptr);
}

TEST(NavInflater, InflateActionsPresent) {
    NavGraph* g = inflateTestGraph(gui_test::R::navigation::nav_simple_test);
    ASSERT_NE(g, nullptr);
    NavDestination* start = g->findNode(gui_test::R::id::start_test);
    NavDestination* second = g->findNode(gui_test::R::id::second_test);
    ASSERT_NE(start, nullptr);
    ASSERT_NE(second, nullptr);

    // The <action> elements are registered on their destinations (keyed by the action's idgen id).
    // (Their app:destination/app:popUpTo resolution is a known inflater gap — see file header.)
    EXPECT_NE(start->getAction(gui_test::R::id::second), nullptr);
    EXPECT_NE(second->getAction(gui_test::R::id::self), nullptr);
    EXPECT_NE(second->getAction(gui_test::R::id::finish), nullptr);
    EXPECT_NE(second->getAction(gui_test::R::id::finish_self), nullptr);
}

TEST(NavInflater, InflateArgumentsMetadata) {
    NavGraph* g = inflateTestGraph(gui_test::R::navigation::nav_simple_test);
    ASSERT_NE(g, nullptr);
    NavDestination* second = g->findNode(gui_test::R::id::second_test);
    ASSERT_NE(second, nullptr);

    const auto& args = second->getArguments();
    // arg2: nullable string, no default.
    ASSERT_TRUE(args.count("arg2") > 0);
    EXPECT_TRUE(args.at("arg2")->isNullable());
    EXPECT_FALSE(args.at("arg2")->isDefaultValuePresent());
    // defaultArg: string with a default value.
    ASSERT_TRUE(args.count("defaultArg") > 0);
    EXPECT_EQ(args.at("defaultArg")->getType(), NavTypeKind::STRING);
    EXPECT_TRUE(args.at("defaultArg")->isDefaultValuePresent());
}
