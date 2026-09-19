#include <app/espresso/defaultfailurehandler.h>

#include <app/espresso/espressoexception.h>

namespace cdroid {
namespace espresso {

DefaultFailureHandler::DefaultFailureHandler(Context* appContext) : mAppContext(appContext) {}

void DefaultFailureHandler::handle(std::exception_ptr error, MatcherPtr<View> viewMatcher) {
    try {
        std::rethrow_exception(error);
    } catch (EspressoException&) {
        // assertions and espresso errors propagate through the friendly path
        getUserFriendlyError(error, std::move(viewMatcher));
    } catch (std::exception&) {
        // AOSP: propagate(error)
        std::rethrow_exception(error);
    }
}

void DefaultFailureHandler::getUserFriendlyError(std::exception_ptr error,
        MatcherPtr<View> viewMatcher) {
    try {
        std::rethrow_exception(error);
    } catch (PerformException& performException) {
        // Re-throw the exception with the viewMatcher (used to locate the view)
        // as the view description (makes the error more readable). The reason we
        // do this here: not all creators of PerformException have access to the
        // viewMatcher.
        throw PerformException::Builder()
                .from(performException)
                .withViewDescription(StringDescription::toString(*viewMatcher))
                .build();
    } catch (AssertionFailedError&) {
        // junit hides the cause constructor — AssertionFailedError carries it.
        throw;
    } catch (...) {
        std::rethrow_exception(error);
    }
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
