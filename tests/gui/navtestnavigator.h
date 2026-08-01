/*********************************************************************************
 * Test-only helpers for the navigation test suite (port of androidx TestNavigator).
 *
 * TestNavigator: a minimal Navigator named "test", matching the <test> destination tag
 * in the ported nav_simple test graph. CDROID's NavController maintains the back stack
 * itself (NavController::addEntryToBackStack), so this navigator only needs to supply
 * destinations; the inherited legacy navigate(NavDestination*,Bundle*,NavOptions*) no-op
 * is sufficient — the controller pushes the NavBackStackEntry.
 *********************************************************************************/
#ifndef CDROID_TESTS_GUI_NAVTESTNAVIGATOR_H
#define CDROID_TESTS_GUI_NAVTESTNAVIGATOR_H
#include <navigation/navigator.h>
#include <navigation/navdestination.h>

namespace cdroid {

class TestNavigator : public Navigator {
public:
    TestNavigator() { mName = "test"; }

    NavDestination* createDestination() override {
        NavDestination* destination = new NavDestination((Navigator*)this);
        destination->setNavigatorName("test");
        return destination;
    }
};

} // namespace cdroid

#endif // CDROID_TESTS_GUI_NAVTESTNAVIGATOR_H
