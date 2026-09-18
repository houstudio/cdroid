#ifndef CDROID_ESPRESSO_VIEWFINDER_H
#define CDROID_ESPRESSO_VIEWFINDER_H

/*
 * android.support.test.espresso.ViewFinder (+ base.ViewFinderImpl) — uses
 * matchers to locate particular views within the view hierarchy.
 */

#include <app/espresso/hamcrest.h>

namespace cdroid {
class View;

namespace espresso {

class RootViewPicker;

class ViewFinder {
public:
    virtual ~ViewFinder() = default;

    /**
     * Immediately locates a single view within the provided view hierarchy.
     *
     * @return A singular view which matches the matcher we were constructed
     *         with (never null).
     * @throws AmbiguousViewMatcherException when multiple views match.
     * @throws NoMatchingViewException when no views match.
     */
    virtual View* getView() = 0;
};

class ViewFinderImpl : public ViewFinder {
public:
    ViewFinderImpl(MatcherPtr<View> viewMatcher, RootViewPicker& rootViewPicker);

    View* getView() override;

private:
    MatcherPtr<View> mViewMatcher;
    RootViewPicker& mRootViewPicker;
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_VIEWFINDER_H*/
