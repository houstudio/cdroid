/*********************************************************************************
 * NavigatorProvider tests — port of androidx.navigation navigation-common
 * NavigatorProviderTest, against CDROID's SimpleNavigatorProvider (the concrete
 * NavigatorProvider impl; there is no reflection so "annotation name" == the
 * Navigator's ctor-assigned mName).
 *
 * Ported: explicit-name add/get, name-from-navigator add/get, re-adding the
 * same instance is a no-op, adding a different navigator under the same name
 * replaces it (HashMap.put semantics).
 *
 * Skipped: the IllegalStateException assertions (CDROID's FATAL_IF aborts
 * rather than throwing), getNavigator-by-class (no reflection) and the
 * onAttach/isAttached sub-assertions of addExistingNavigatorDoesntReplace
 * (NavigatorState construction requires a live NavController); the Kotlin
 * operator-sugar set()/plusAssign() variants (same code paths).
 *********************************************************************************/
#include <gtest/gtest.h>
#include <navigation/simplenavigatorprovider.h>
#include <navigation/navigator.h>

using namespace cdroid;

namespace {
// androidx.navigation test double EmptyNavigator; NAME mimics @Navigator.Name.
class EmptyNavigator : public Navigator{
public:
    static const char* NAME() { return "empty"; }
    EmptyNavigator(const std::string& name = NAME()){ mName = name; }
    NavDestination* createDestination() override { return nullptr; }
};
class EmptyNavigator2 : public EmptyNavigator{
public:
    EmptyNavigator2() : EmptyNavigator(NAME()) {}
};
} // namespace

TEST(NavigatorProvider, AddWithExplicitNameGetWithExplicitName) {
    SimpleNavigatorProvider provider;
    Navigator* navigator = new EmptyNavigator("explicit");
    provider.addNavigator("name", navigator);
    EXPECT_EQ(provider.getNavigator("name"), navigator);
}

TEST(NavigatorProvider, AddWithAnnotationNameGetWithExplicitName) {
    SimpleNavigatorProvider provider;
    Navigator* navigator = new EmptyNavigator();
    provider.addNavigator(navigator);
    EXPECT_EQ(provider.getNavigator(EmptyNavigator::NAME()), navigator);
}

TEST(NavigatorProvider, AddExistingNavigatorDoesntReplace) {
    SimpleNavigatorProvider provider;
    Navigator* navigator = new EmptyNavigator();
    provider.addNavigator(navigator);
    EXPECT_EQ(provider.getNavigator(EmptyNavigator::NAME()), navigator);
    // Re-adding the same instance is a no-op (must not delete/replace it).
    provider.addNavigator(navigator);
    EXPECT_EQ(provider.getNavigator(EmptyNavigator::NAME()), navigator);
    EXPECT_EQ(provider.getNavigators().size(), 1u);
}

TEST(NavigatorProvider, AddWithSameNameButUnequalNavigatorDoesReplace) {
    SimpleNavigatorProvider provider;
    Navigator* navigatorA = new EmptyNavigator();
    Navigator* navigatorB = new EmptyNavigator2();
    provider.addNavigator(navigatorA);
    EXPECT_EQ(provider.getNavigator(EmptyNavigator::NAME()), navigatorA);
    provider.addNavigator(navigatorB);
    EXPECT_EQ(provider.getNavigator(EmptyNavigator::NAME()), navigatorB);
}
