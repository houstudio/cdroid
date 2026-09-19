#include <app/espresso/viewinteraction.h>

#include <functional>

#include <app/espresso/espressoexception.h>
#include <app/espresso/humanreadables.h>
#include <app/espresso/uicontroller.h>
#include <app/espresso/viewaction.h>
#include <app/espresso/viewassertion.h>
#include <app/espresso/viewfinder.h>
#include <app/espresso/viewmatchers.h>

#include <porting/cdlog.h>
#include <view/view.h>
#include <widget/adapterview.h>
#include <app/espresso/viewactions.h>

namespace cdroid {
namespace espresso {

static const char* TAG = "ViewInteraction";

ViewInteraction::ViewInteraction(UiController& uiController,
        std::shared_ptr<ViewFinder> viewFinder, FailureHandler& failureHandler,
        MatcherPtr<View> viewMatcher, std::shared_ptr<RootMatcherPtr> rootMatcherRef)
    : mUiController(uiController),
      mViewFinder(viewFinder),
      mDefaultFailureHandler(failureHandler),
      mFailureHandler(&failureHandler),
      mViewMatcher(std::move(viewMatcher)),
      mRootMatcherRef(std::move(rootMatcherRef)) {}

ViewInteraction& ViewInteraction::perform(
        const std::vector<std::shared_ptr<ViewAction>>& viewActions) {
    for (const std::shared_ptr<ViewAction>& action : viewActions) {
        doPerform(action);
    }
    return *this;
}

ViewInteraction& ViewInteraction::withFailureHandler(FailureHandler* failureHandler) {
    mFailureHandler = failureHandler;
    return *this;
}

ViewInteraction& ViewInteraction::inRoot(RootMatcherPtr rootMatcher) {
    *mRootMatcherRef = std::move(rootMatcher);
    return *this;
}

void ViewInteraction::doPerform(std::shared_ptr<ViewAction> viewAction) {
    if (!viewAction) {
        throw std::invalid_argument("viewAction must not be null");
    }
    MatcherPtr<View> constraints = viewAction->getConstraints();
    if (!constraints) {
        throw std::invalid_argument("Action constraints must not be null");
    }
    runSynchronouslyOnUiThread([this, viewAction, constraints]() {
        mUiController.loopMainThreadUntilIdle();
        View* targetView = mViewFinder->getView();
        LOGI("Performing '%s' action on view %s", viewAction->getDescription().c_str(),
             StringDescription::toString(*mViewMatcher).c_str());
        if (!constraints->matches(*targetView)) {
            // TODO(user): update this to describeMismatch once hamcrest is updated to new
            StringDescription stringDescription;
            stringDescription.appendText(
                    "Action will not be performed because the target view "
                    "does not match one or more of the following constraints:\n");
            constraints->describeTo(stringDescription);
            stringDescription.appendText("\nTarget view: ")
                    .appendValue(HumanReadables::describe(targetView));

            if (std::dynamic_pointer_cast<ScrollToAction>(viewAction) != nullptr
                    && ViewMatchers::isDescendantOfA(
                            ViewMatchers::isAssignableFrom<AdapterView>())
                               ->matches(*targetView)) {
                stringDescription.appendText(
                        "\nFurther Info: ScrollToAction on a view inside an AdapterView will not "
                        "work. Use Espresso.onData to load the view.");
            }
            throw PerformException::Builder()
                    .withActionDescription(viewAction->getDescription())
                    .withViewDescription(StringDescription::toString(*mViewMatcher))
                    .withCause(std::make_exception_ptr(
                            std::runtime_error(stringDescription.str())))
                    .build();
        } else {
            viewAction->perform(mUiController, *targetView);
        }
    });
}

ViewInteraction& ViewInteraction::check(std::shared_ptr<ViewAssertion> viewAssert) {
    if (!viewAssert) {
        throw std::invalid_argument("viewAssert must not be null");
    }
    runSynchronouslyOnUiThread([this, viewAssert]() {
        mUiController.loopMainThreadUntilIdle();

        View* targetView = nullptr;
        // Java keeps the caught exception alive for the assertion (GC); C++
        // destroys it at the end of the catch clause, so a copy is held.
        std::shared_ptr<NoMatchingViewException> missingViewException;
        try {
            targetView = mViewFinder->getView();
        } catch (NoMatchingViewException& noMatchingViewException) {
            missingViewException = std::make_shared<NoMatchingViewException>(
                    noMatchingViewException);
        }
        viewAssert->check(targetView, missingViewException.get());
    });
    return *this;
}

void ViewInteraction::runSynchronouslyOnUiThread(const std::function<void()>& action) {
    // AOSP: FutureTask → mainThreadExecutor.execute(uiTask) → uiTask.get();
    // ExecutionException → failureHandler.handle(ee.getCause(), viewMatcher).
    // Every failure inside the task surfaces through handle() — the handler
    // itself decides what propagates (see DefaultFailureHandler). The
    // single-thread port runs the task inline (see file header).
    try {
        action();
    } catch (...) {
        mFailureHandler->handle(std::current_exception(), mViewMatcher);
    }
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
