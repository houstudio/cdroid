#ifndef CDROID_ESPRESSO_ESPRESSO_H
#define CDROID_ESPRESSO_ESPRESSO_H

/*
 * android.support.test.espresso.Espresso — the entry point to the Espresso
 * framework. Use it to create ViewInteractions, register idling resources,
 * and set a custom failure handler.
 *
 * AOSP wires the graph (UiController/RootViewPicker/ViewFinder/FailureHandler)
 * through Dagger at instrumentation startup; the port assembles the same
 * objects lazily on first use, on the main thread (the in-process "test
 * runner" is posted there — see espressotests.h).
 */

#include <memory>
#include <vector>

#include <app/espresso/failurehandler.h>
#include <app/espresso/hamcrest.h>
#include <app/espresso/idlingresource.h>

namespace cdroid {
class View;

namespace espresso {

class DataInteraction;
class ViewInteraction;

class Espresso {
public:
    /**
     * Creates a ViewInteraction for a given view. Note: the view has to be
     * part of the view hierarchy. This may not be the case if it is rendered
     * as part of an Adapter (e.g. ListView). If this is the case, use
     * Espresso.onData instead.
     */
    static ViewInteraction onView(MatcherPtr<View> viewMatcher);

    /**
     * Creates a DataInteraction for a given data matcher. This should be
     * used in preference to a ViewInteraction when the underlying data is
     * rendered into an AdapterView (e.g. a ListView). The data model is
     * void* (Adapter::getItem), so the matcher matches against that.
     */
    static DataInteraction onData(MatcherPtr<void*> dataMatcher);

    /**
     * Sets the failure handler for Espresso (AOSP: setFailureHandler).
     * Null resets to the DefaultFailureHandler.
     */
    static void setFailureHandler(FailureHandler* failureHandler);

    static void registerIdlingResources(const std::vector<IdlingResourcePtr>& resources);
    static bool unregisterIdlingResources(const std::vector<IdlingResourcePtr>& resources);
    static std::vector<IdlingResourcePtr> getIdlingResources();

    /** AOSP registerLooperAsIdlingResource — LooperIdlingResource not yet ported. */

    /** Closes soft keyboard. */
    static void closeSoftKeyboard();

    /** Presses the back button. */
    static void pressBack();

    Espresso() = delete;
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_ESPRESSO_H*/
