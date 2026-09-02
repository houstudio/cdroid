#ifndef CDROID_ESPRESSO_VIEWINTERACTION_H
#define CDROID_ESPRESSO_VIEWINTERACTION_H

/*
 * android.support.test.espresso.ViewInteraction — provides the primary
 * interface for test authors to perform actions or asserts on views.
 *
 * Each interaction is associated with a view identified by a view matcher.
 * All view actions and asserts are performed on the UI thread (thus ensuring
 * sequential execution). The same goes for retrieval of views (this is done
 * to ensure that view state is "fresh" prior to execution of each operation).
 *
 * Threading adaptation: AOSP marshals a FutureTask to the main-thread
 * executor and blocks the instrumentation thread; CDROID's Espresso runs ON
 * the main thread (the runner is posted there), so runSynchronouslyOnUiThread
 * executes the task inline. The failure-handling contract (everything routed
 * through FailureHandler.handle) is unchanged.
 */

#include <memory>
#include <vector>

#include <app/espresso/failurehandler.h>
#include <app/espresso/hamcrest.h>
#include <app/espresso/rootmatchers.h>

namespace cdroid {
class View;

namespace espresso {

class UiController;
class ViewAction;
class ViewAssertion;
class ViewFinder;

class ViewInteraction {
public:
    ViewInteraction(UiController& uiController, std::shared_ptr<ViewFinder> viewFinder,
            FailureHandler& failureHandler, MatcherPtr<View> viewMatcher,
            std::shared_ptr<RootMatcherPtr> rootMatcherRef);

    /**
     * Performs the given action(s) on the view selected by the current view
     * matcher. If more than one action is provided, actions are executed in
     * the order provided with precondition checks running prior to each action.
     */
    ViewInteraction& perform(const std::vector<std::shared_ptr<ViewAction>>& viewActions);

    /** Replaces the default failure handler for this particular interaction. */
    ViewInteraction& withFailureHandler(FailureHandler* failureHandler);

    /** Makes this ViewInteraction scoped to the root selected by the given root matcher. */
    ViewInteraction& inRoot(RootMatcherPtr rootMatcher);

    /** Checks the given ViewAssertion on the view selected by the current view matcher. */
    ViewInteraction& check(std::shared_ptr<ViewAssertion> viewAssert);

private:
    void doPerform(std::shared_ptr<ViewAction> viewAction);
    void runSynchronouslyOnUiThread(const std::function<void()>& action);

    UiController& mUiController;
    std::shared_ptr<ViewFinder> mViewFinder;  // per-interaction (Java GC)
    FailureHandler& mDefaultFailureHandler;
    FailureHandler* mFailureHandler;   // volatile in AOSP; single-threaded here
    MatcherPtr<View> mViewMatcher;
    std::shared_ptr<RootMatcherPtr> mRootMatcherRef;  // AtomicReference<Matcher<Root>>
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_VIEWINTERACTION_H*/
