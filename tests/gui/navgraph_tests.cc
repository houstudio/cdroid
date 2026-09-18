/*********************************************************************************
 * NavGraph tests — port of androidx.navigation navigation-common NavGraphTest
 * (androidHostTest), the subset CDROID implements.
 *
 * Ported: addDestination (parent wiring + findNode), addDestinationWithoutId,
 * addDestinationWithSameId (child id == graph id), setStartDestinationWithSameId,
 * addDestinationsAsCollection, addReplacementDestination (same id replaces and
 * detaches the old child), addDestinationWithExistingParent, addAll (moves the
 * nodes), remove, clear, and iteration order via NavGraph::begin()/end().
 *
 * Skipped: the Kotlin java.util.Iterator contract tests (iteratorNoSuchElement,
 * iteratorRemove/doubleRemove — CDROID exposes a C++ range, not a mutable Java
 * iterator), equals/hashCode of graphs, and NavGraphNavigatorTest navigate()
 * (covered by NavController tests; the back stack lives on the controller).
 *
 * Like navcontroller_tests: everything is heap-allocated and leaked on purpose
 * (the test process exits); NavGraph owns its destinations.
 *********************************************************************************/
#include <gtest/gtest.h>
#include <navigation/navgraph.h>
#include <navigation/navdestination.h>
#include <navigation/simplenavigatorprovider.h>
#include "navtestnavigator.h"

using namespace cdroid;

namespace {
constexpr int FIRST_DESTINATION_ID = 1;
constexpr int SECOND_DESTINATION_ID = 2;

NavDestination* createFirstDestination(TestNavigator* nav) {
    NavDestination* d = nav->createDestination();
    d->setId(FIRST_DESTINATION_ID);
    return d;
}
NavDestination* createSecondDestination(TestNavigator* nav) {
    NavDestination* d = nav->createDestination();
    d->setId(SECOND_DESTINATION_ID);
    return d;
}
} // namespace

TEST(NavGraph, AddDestinationWithoutId) {
    TestNavigator nav;
    NavGraph graph(new SimpleNavigatorProvider());
    NavDestination* destination = nav.createDestination(); // no id, no route
    EXPECT_THROW(graph.addDestination(destination), std::runtime_error);
}

TEST(NavGraph, AddDestination) {
    TestNavigator nav;
    NavGraph* graph = new NavGraph(new SimpleNavigatorProvider());
    NavDestination* destination = createFirstDestination(&nav);
    graph->addDestination(destination);
    EXPECT_EQ(destination->getParent(), graph);
    EXPECT_EQ(graph->findNode(FIRST_DESTINATION_ID), destination);
}

TEST(NavGraph, AddDestinationWithSameId) {
    TestNavigator nav;
    NavGraph* graph = new NavGraph(new SimpleNavigatorProvider());
    graph->setId(FIRST_DESTINATION_ID);
    NavDestination* destination = createFirstDestination(&nav);
    // Adding a destination with the same id as its parent graph should fail.
    EXPECT_THROW(graph->addDestination(destination), std::runtime_error);
}

TEST(NavGraph, SetStartDestinationWithSameId) {
    TestNavigator nav;
    NavGraph* graph = new NavGraph(new SimpleNavigatorProvider());
    graph->setId(FIRST_DESTINATION_ID);
    // Setting a start destination with the same id as its parent graph should fail.
    EXPECT_THROW(graph->setStartDestination(FIRST_DESTINATION_ID), std::runtime_error);
}

TEST(NavGraph, AddDestinationsAsCollection) {
    TestNavigator nav;
    NavGraph* graph = new NavGraph(new SimpleNavigatorProvider());
    NavDestination* destination = createFirstDestination(&nav);
    NavDestination* secondDestination = createSecondDestination(&nav);
    graph->addDestinations({destination, secondDestination});

    EXPECT_EQ(destination->getParent(), graph);
    EXPECT_EQ(graph->findNode(FIRST_DESTINATION_ID), destination);
    EXPECT_EQ(secondDestination->getParent(), graph);
    EXPECT_EQ(graph->findNode(SECOND_DESTINATION_ID), secondDestination);
}

TEST(NavGraph, AddReplacementDestination) {
    TestNavigator nav;
    NavGraph* graph = new NavGraph(new SimpleNavigatorProvider());
    NavDestination* destination = createFirstDestination(&nav);
    graph->addDestination(destination);

    NavDestination* replacementDestination = nav.createDestination();
    replacementDestination->setId(FIRST_DESTINATION_ID);
    graph->addDestination(replacementDestination);

    EXPECT_EQ(destination->getParent(), nullptr);
    EXPECT_EQ(replacementDestination->getParent(), graph);
    EXPECT_EQ(graph->findNode(FIRST_DESTINATION_ID), replacementDestination);
}

TEST(NavGraph, AddDestinationWithExistingParent) {
    TestNavigator nav;
    NavGraph* graph = new NavGraph(new SimpleNavigatorProvider());
    NavDestination* destination = createFirstDestination(&nav);
    graph->addDestination(destination);

    NavGraph* other = new NavGraph(new SimpleNavigatorProvider());
    EXPECT_THROW(other->addDestination(destination), std::runtime_error);
}

TEST(NavGraph, AddAll) {
    TestNavigator nav;
    NavGraph* other = new NavGraph(new SimpleNavigatorProvider());
    NavDestination* destination = createFirstDestination(&nav);
    other->addDestination(destination);

    NavGraph* graph = new NavGraph(new SimpleNavigatorProvider());
    graph->addAll(other);

    EXPECT_EQ(destination->getParent(), graph);
    EXPECT_EQ(graph->findNode(FIRST_DESTINATION_ID), destination);
    EXPECT_EQ(other->findNode(FIRST_DESTINATION_ID), nullptr);
}

TEST(NavGraph, RemoveDestination) {
    TestNavigator nav;
    NavGraph* graph = new NavGraph(new SimpleNavigatorProvider());
    NavDestination* destination = createFirstDestination(&nav);
    graph->addDestination(destination);

    graph->remove(destination);

    EXPECT_EQ(destination->getParent(), nullptr);
    EXPECT_EQ(graph->findNode(FIRST_DESTINATION_ID), nullptr);
}

TEST(NavGraph, Clear) {
    TestNavigator nav;
    NavGraph* graph = new NavGraph(new SimpleNavigatorProvider());
    NavDestination* destination = createFirstDestination(&nav);
    NavDestination* secondDestination = createSecondDestination(&nav);
    graph->addDestination(destination);
    graph->addDestination(secondDestination);

    graph->clear();
    EXPECT_EQ(destination->getParent(), nullptr);
    EXPECT_EQ(graph->findNode(FIRST_DESTINATION_ID), nullptr);
    EXPECT_EQ(secondDestination->getParent(), nullptr);
    EXPECT_EQ(graph->findNode(SECOND_DESTINATION_ID), nullptr);
}

TEST(NavGraph, IterateDestinationsInOrder) {
    TestNavigator nav;
    NavGraph* graph = new NavGraph(new SimpleNavigatorProvider());
    NavDestination* destination = createFirstDestination(&nav);
    NavDestination* secondDestination = createSecondDestination(&nav);
    graph->addDestination(destination);
    graph->addDestination(secondDestination);

    std::vector<NavDestination*> visited;
    for(auto it = graph->begin(); it != graph->end(); ++it)
        visited.push_back((*it).second);
    ASSERT_EQ(visited.size(), 2u);
    // SparseArray keeps ascending id order: FIRST then SECOND.
    EXPECT_EQ(visited[0], destination);
    EXPECT_EQ(visited[1], secondDestination);
}
