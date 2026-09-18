#ifndef CDROID_ESPRESSO_VIEWASSERTION_H
#define CDROID_ESPRESSO_VIEWASSERTION_H

/*
 * android.support.test.espresso.ViewAssertion — responsible for performing
 * assertions on the selected view. Called on the main thread after the view
 * has been located (both arguments are nullable: null view + non-null
 * NoMatchingViewException when nothing matched).
 */

namespace cdroid {
class View;

namespace espresso {

class NoMatchingViewException;

class ViewAssertion {
public:
    virtual ~ViewAssertion() = default;

    /**
     * Checks the state of the given view (if it exists — null when the
     * matcher found nothing, in which case noViewFoundException describes why).
     */
    virtual void check(View* view, NoMatchingViewException* noViewFoundException) = 0;
};

using ViewAssertionPtr = std::shared_ptr<ViewAssertion>;

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_VIEWASSERTION_H*/
