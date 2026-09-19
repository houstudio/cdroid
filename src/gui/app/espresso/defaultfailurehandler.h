#ifndef CDROID_ESPRESSO_DEFAULTFAILUREHANDLER_H
#define CDROID_ESPRESSO_DEFAULTFAILUREHANDLER_H

/*
 * android.support.test.espresso.base.DefaultFailureHandler — Espresso's
 * default FailureHandler. If this does not fit your needs, provide your own
 * via Espresso.setFailureHandler.
 *
 * C++ note: Java swaps in the current thread's stack trace; there is no such
 * artifact here — AssertionFailedError keeps its cause chain instead.
 */

#include <app/espresso/failurehandler.h>

namespace cdroid {
class Context;

namespace espresso {

class DefaultFailureHandler : public FailureHandler {
public:
    explicit DefaultFailureHandler(Context* appContext);

    void handle(std::exception_ptr error, MatcherPtr<View> viewMatcher) override;

private:
    /**
     * When the error is coming from espresso, it is more user friendly to:
     * 1. propagate assertions as assertions
     * 2. swap PerformException's view description to the viewMatcher used to
     *    locate the view (makes the error more readable).
     */
    void getUserFriendlyError(std::exception_ptr error, MatcherPtr<View> viewMatcher);

    Context* mAppContext;
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_DEFAULTFAILUREHANDLER_H*/
