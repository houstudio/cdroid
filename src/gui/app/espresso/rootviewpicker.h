#ifndef CDROID_ESPRESSO_ROOTVIEWPICKER_H
#define CDROID_ESPRESSO_ROOTVIEWPICKER_H

/*
 * android.support.test.espresso.base.RootViewPicker — provides the root view
 * of the next matching view hierarchy. Waits (up to 10s) for the root to be
 * ready: laid out and, if focusable, focused.
 *
 * CDROID adaptation: AOSP first waits for a RESUMED activity via the
 * ActivityLifecycleMonitor; with no Activity stack the picker waits for the
 * WindowManager to hold at least one window.
 */

#include <functional>

#include <app/espresso/rootmatchers.h>
#include <app/espresso/rootsoracle.h>

namespace cdroid {
class Looper;
class View;

namespace espresso {

class UiController;

class RootViewPicker {
public:
    /** AOSP injects an AtomicReference<Matcher<Root>> — the supplier reads it. */
    using RootMatcherSupplier = std::function<RootMatcherPtr()>;

    RootViewPicker(RootMatcherSupplier rootMatcherSupplier, UiController& uiController,
            Looper* mainLooper);

    /** The root view of the hierarchy to run view matchers against. */
    View* get();

private:
    Root findRoot();
    void waitForAtLeastOneWindowToAppear();
    static bool isReady(const Root& root);
    Root reduceRoots(const std::vector<Root>& roots);

    RootMatcherSupplier mRootMatcherSupplier;
    UiController& mUiController;
    Looper* mMainLooper;
    RootsOracle mRootsOracle;
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_ROOTVIEWPICKER_H*/
