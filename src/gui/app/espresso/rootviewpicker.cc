#include <app/espresso/rootviewpicker.h>

#include <stdexcept>

#include <app/espresso/espressoexception.h>
#include <app/espresso/hamcrest.h>
#include <app/espresso/humanreadables.h>
#include <app/espresso/uicontroller.h>

#include <core/systemclock.h>
#include <porting/cdlog.h>
#include <view/view.h>

namespace cdroid {
namespace espresso {

RootViewPicker::RootViewPicker(RootMatcherSupplier rootMatcherSupplier,
        UiController& uiController, Looper* mainLooper)
    : mRootMatcherSupplier(std::move(rootMatcherSupplier)),
      mUiController(uiController),
      mMainLooper(mainLooper) {}

View* RootViewPicker::get() {
    Root root = findRoot();
    // AOSP loops: <3 iterations loopMainThreadUntilIdle, <1001 iterations
    // loopMainThreadForAtLeast(10), then gives up ("waited for the root view
    // for 10 secs...").
    int loops = 0;
    while (!isReady(root)) {
        if (loops < 3) {
            mUiController.loopMainThreadUntilIdle();
        } else if (loops < 1001) {
            mUiController.loopMainThreadForAtLeast(10);
        } else {
            throw std::runtime_error(
                    "Waited for the root view to be ready for 10 secs — bailing out.");
        }
        // Re-resolve the root: focus may have moved to another window.
        root = findRoot();
        loops++;
    }
    return root.getDecorView();
}

Root RootViewPicker::findRoot() {
    waitForAtLeastOneWindowToAppear();

    const std::vector<Root> activeRoots = mRootsOracle.listActiveRoots();
    RootMatcherPtr rootMatcher = mRootMatcherSupplier();

    std::vector<Root> picked;
    for (const Root& root : activeRoots) {
        if (rootMatcher->matches(root)) {
            picked.push_back(root);
        }
    }
    if (picked.empty()) {
        std::string message = StringDescription::toString(*rootMatcher)
                + " matched no roots (among " + std::to_string(activeRoots.size())
                + " active roots)";
        for (const Root& root : activeRoots) {
            message += "\n" + root.toString();
        }
        throw NoMatchingRootException(message);
    }
    if (picked.size() == 1) {
        return picked[0];
    }
    return reduceRoots(picked);
}

void RootViewPicker::waitForAtLeastOneWindowToAppear() {
    // AOSP: waitForAtLeastOneActivityToBeResumed (ActivityLifecycleMonitor).
    const int64_t deadline = SystemClock::uptimeMillis() + 10000;
    while (mRootsOracle.listActiveRoots().empty()) {
        if (SystemClock::uptimeMillis() > deadline) {
            throw std::runtime_error("no window appeared within 10 seconds");
        }
        mUiController.loopMainThreadForAtLeast(50);
    }
}

// static
bool RootViewPicker::isReady(const Root& root) {
    View* decorView = root.getDecorView();
    if (decorView == nullptr) return false;
    // root.isLayoutRequested() && (hasWindowFocus || !isFocusable())
    return !decorView->isLayoutRequested()
            && (viewHasWindowFocus(decorView) || !RootMatchers::isFocusable()->matches(root));
}

Root RootViewPicker::reduceRoots(const std::vector<Root>& roots) {
    // AOSP: an Ordering that prefers isDialog matches, breaking ties by the
    // highest windowLayoutParams.type.
    RootMatcherPtr isDialog = RootMatchers::isDialog();
    Root best = roots[0];
    for (const Root& root : roots) {
        const bool bestDialog = isDialog->matches(best);
        const bool rootDialog = isDialog->matches(root);
        if (rootDialog && !bestDialog) {
            best = root;
            continue;
        }
        if (rootDialog == bestDialog) {
            const int rootType = root.getWindowLayoutParams() ? root.getWindowLayoutParams()->type : -1;
            const int bestType = best.getWindowLayoutParams() ? best.getWindowLayoutParams()->type : -1;
            if (rootType > bestType) best = root;
        }
    }
    return best;
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
