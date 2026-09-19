#ifndef CDROID_ESPRESSO_FAILUREHANDLER_H
#define CDROID_ESPRESSO_FAILUREHANDLER_H

/*
 * android.support.test.espresso.FailureHandler — used by Espresso to handle
 * failures. Replaces itself via Espresso.setFailureHandler.
 */

#include <app/espresso/hamcrest.h>

namespace cdroid {
class View;

namespace espresso {

class FailureHandler {
public:
    virtual ~FailureHandler() = default;

    /**
     * Handles the given error in the appropriate manner (Java Throwable →
     * std::exception_ptr; rethrowable with std::rethrow_exception).
     *
     * @param error the error (if any) that caused the failure.
     * @param viewMatcher the view matcher that was used to select the view.
     */
    virtual void handle(std::exception_ptr error, MatcherPtr<View> viewMatcher) = 0;
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_FAILUREHANDLER_H*/
