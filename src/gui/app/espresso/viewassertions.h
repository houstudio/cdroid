#ifndef CDROID_ESPRESSO_VIEWASSERTIONS_H
#define CDROID_ESPRESSO_VIEWASSERTIONS_H

/*
 * android.support.test.espresso.ViewAssertions — a collection of common
 * ViewAssertions.
 */

#include <app/espresso/hamcrest.h>
#include <app/espresso/viewassertion.h>

namespace cdroid {
class View;

namespace espresso {

class ViewAssertions {
public:
    /** Asserts that a view does not exist in the view hierarchy. */
    static ViewAssertionPtr doesNotExist();

    /**
     * Asserts that the selected view matches the given matcher.
     * (AOSP: ViewAssertions.matches.)
     */
    static ViewAssertionPtr matches(MatcherPtr<View> viewMatcher);

    /**
     * Returns a generic ViewAssertion which asserts that the selected view's
     * descendants matching the selector also match the matcher.
     */
    static ViewAssertionPtr selectedDescendantsMatch(MatcherPtr<View> selector,
            MatcherPtr<View> matcher);
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_VIEWASSERTIONS_H*/
