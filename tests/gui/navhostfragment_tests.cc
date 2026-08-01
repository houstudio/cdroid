/*********************************************************************************
 * NavHostFragment tests — port of androidx NavHostFragmentTest (setGraph in onCreate,
 * findNavController). A NavHostFragment hosting nav_host_test.xml is added to a
 * FragmentActivity; the deferred-commit + container-late-resolve path (setGraph now runs in
 * onCreate) must still land the start destination and publish the NavController.
 *********************************************************************************/
#include <gtest/gtest.h>
#include <fragment/fragmentactivity.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenttransaction.h>
#include <fragment/fragment.h>
#include <fragment/fragmentfactory.h>
#include <view/view.h>
#include <core/windowmanager.h>
#include <navigation/navhostfragment.h>
#include <navigation/navcontroller.h>
#include <navigation/navigation.h>
#include "guienvironment.h"

using namespace cdroid;

// Backs the <fragment> destinations in nav_host_test.xml. At global scope (with using namespace
// cdroid) so REGISTER_FRAGMENT can reference it by unqualified name.
class NavTestFragment : public fragment::Fragment {
public:
    View* onCreateView(LayoutInflater*, ViewGroup*, Bundle*) override {
        return new View(10, 10);
    }
};
REGISTER_FRAGMENT(NavTestFragment);

namespace {

class TestFragmentActivity : public fragment::FragmentActivity {
public:
    TestFragmentActivity() : FragmentActivity(0, 0, -1, -1) {}
};

void cleanup(Window* w) {
    WindowManager::getInstance().removeWindow(w);
    pumpFor(20);
}

} // namespace

TEST(NavHostFragment, SetGraphInOnCreateLoadsStart) {
    auto* activity = new TestFragmentActivity();
    pumpFor(100); // drive the activity's posted onCreate..onResume

    NavHostFragment* host = new NavHostFragment("@navigation/nav_host_test");
    activity->getSupportFragmentManager()
        ->beginTransaction()->replace(activity->getFragmentContainerId(), host).commit();
    activity->getSupportFragmentManager()->executePendingTransactions();
    pumpFor(100); // host onCreate (setGraph, deferred) + onCreateView converge

    NavController* nc = host->getNavController();
    ASSERT_NE(nc, nullptr);
    // setGraph in onCreate auto-navigated to the start destination (resolved by route).
    ASSERT_NE(nc->getCurrentDestination(), nullptr);
    EXPECT_EQ(nc->getCurrentDestination()->getRoute(), "start_fragment");
    cleanup(activity);
}

TEST(NavHostFragment, FindNavControllerFromHostView) {
    auto* activity = new TestFragmentActivity();
    pumpFor(100);

    NavHostFragment* host = new NavHostFragment("@navigation/nav_host_test");
    activity->getSupportFragmentManager()
        ->beginTransaction()->replace(activity->getFragmentContainerId(), host).commit();
    activity->getSupportFragmentManager()->executePendingTransactions();
    pumpFor(100);

    // onViewCreated publishes the NavController on the host View; findNavController resolves it.
    View* hostView = host->getView();
    ASSERT_NE(hostView, nullptr);
    NavController* found = Navigation::findNavController(hostView);
    EXPECT_NE(found, nullptr);
    cleanup(activity);
}
