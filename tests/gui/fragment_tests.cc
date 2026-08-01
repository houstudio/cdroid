/*********************************************************************************
 * FragmentManager / FragmentTransaction tests — port of androidx FragmentLifecycleTest +
 * FragmentTransactionTest (the subset CDROID implements: add/remove lifecycle, back stack,
 * commit() deferred vs commitNow() synchronous, setMaxLifecycle).
 *
 * Each test builds a FragmentActivity (a Window that self-registers with the WindowManager
 * and has its lifecycle posted to the main looper), drives it + deferred commits with
 * pumpFor()/executePendingTransactions(), and removes the window at the end.
 *********************************************************************************/
#include <gtest/gtest.h>
#include <algorithm>
#include <vector>
#include <string>
#include <fragment/fragmentactivity.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragment.h>
#include <fragment/fragmenttransaction.h>
#include <view/view.h>
#include <core/windowmanager.h>
#include <lifecycle/lifecycle.h>
#include "guienvironment.h"

using namespace cdroid;

namespace {

// Fragment that records its lifecycle callback sequence.
class CountingFragment : public fragment::Fragment {
public:
    std::vector<std::string> calls;
    bool saw(const std::string& s) const {
        return std::find(calls.begin(), calls.end(), s) != calls.end();
    }
    void onAttach(Context*) override { calls.push_back("onAttach"); }
    void onCreate(Bundle*) override { calls.push_back("onCreate"); }
    View* onCreateView(LayoutInflater*, ViewGroup*, Bundle*) override {
        calls.push_back("onCreateView");
        return new View(10, 10);
    }
    void onViewCreated(View*, Bundle*) override { calls.push_back("onViewCreated"); }
    void onResume() override { calls.push_back("onResume"); }
    void onPause() override { calls.push_back("onPause"); }
    void onStop() override { calls.push_back("onStop"); }
    void onDestroyView() override { calls.push_back("onDestroyView"); }
    void onDestroy() override { calls.push_back("onDestroy"); }
    void onDetach() override { calls.push_back("onDetach"); }
};

class TestFragmentActivity : public fragment::FragmentActivity {
public:
    TestFragmentActivity() : FragmentActivity(0, 0, -1, -1) {}
};

void cleanup(Window* w) {
    WindowManager::getInstance().removeWindow(w);
    pumpFor(20);
}

} // namespace

TEST(Fragment, BasicLifecycle) {
    auto* activity = new TestFragmentActivity();
    pumpFor(100); // drive the posted onCreate..onResume

    CountingFragment* f = new CountingFragment();
    activity->getSupportFragmentManager()
        ->beginTransaction()->add(activity->getFragmentContainerId(), f).commit();
    activity->getSupportFragmentManager()->executePendingTransactions();
    pumpFor(50);

    ASSERT_FALSE(f->calls.empty());
    EXPECT_EQ(f->calls.front(), "onAttach");
    EXPECT_TRUE(f->saw("onCreate"));
    EXPECT_TRUE(f->saw("onResume"));

    activity->getSupportFragmentManager()
        ->beginTransaction()->remove(f).commit();
    activity->getSupportFragmentManager()->executePendingTransactions();
    pumpFor(50);
    EXPECT_TRUE(f->saw("onDestroy"));

    cleanup(activity);
}

TEST(Fragment, CommitIsDeferred) {
    auto* activity = new TestFragmentActivity();
    pumpFor(100);
    CountingFragment* f = new CountingFragment();
    // commit() enqueues; the fragment is not attached until the pending action is drained.
    activity->getSupportFragmentManager()
        ->beginTransaction()->add(activity->getFragmentContainerId(), f).commit();
    EXPECT_TRUE(f->calls.empty());

    activity->getSupportFragmentManager()->executePendingTransactions();
    EXPECT_FALSE(f->calls.empty());
    EXPECT_EQ(f->calls.front(), "onAttach");
    cleanup(activity);
}

TEST(Fragment, CommitNowIsSynchronous) {
    auto* activity = new TestFragmentActivity();
    pumpFor(100);
    CountingFragment* f = new CountingFragment();
    // commitNow() runs synchronously: the fragment is attached immediately, no drain needed.
    activity->getSupportFragmentManager()
        ->beginTransaction()->add(activity->getFragmentContainerId(), f).commitNow();
    EXPECT_FALSE(f->calls.empty());
    EXPECT_EQ(f->calls.front(), "onAttach");
    cleanup(activity);
}

TEST(Fragment, BackStack) {
    auto* activity = new TestFragmentActivity();
    pumpFor(100);
    auto* fm = activity->getSupportFragmentManager();

    CountingFragment* f1 = new CountingFragment();
    fm->beginTransaction()->add(activity->getFragmentContainerId(), f1).commit();
    fm->executePendingTransactions();
    pumpFor(50);

    CountingFragment* f2 = new CountingFragment();
    fm->beginTransaction()
        ->replace(activity->getFragmentContainerId(), f2).addToBackStack("stack1").commit();
    fm->executePendingTransactions();
    pumpFor(50);

    // Popping reverses the back-stack record.
    EXPECT_TRUE(fm->popBackStackImmediate());
    pumpFor(50);
    cleanup(activity);
}
