#include <app/espresso/espressoexception.h>

#include <app/espresso/humanreadables.h>

#include <view/viewgroup.h>

namespace cdroid {
namespace espresso {

// PerformException.build(): "Error performing '<action>' on view '<view>'."
// with the cause, if any, chained (Java chains via initCause).
PerformException PerformException::Builder::build() {
    std::string message = "Error performing '" + mActionDescription + "' on view '"
        + mViewDescription + "'.";
    PerformException exception(message);
    exception.mActionDescription = mActionDescription;
    exception.mViewDescription = mViewDescription;
    exception.mCause = mCause;
    return exception;
}

NoMatchingViewException NoMatchingViewException::Builder::build() {
    std::string message = "No views in hierarchy found matching: "
        + StringDescription::toString(*mViewMatcher);
    if (!mViewHierarchy.empty()) {
        message += "\nView Hierarchy:\n" + mViewHierarchy;
    }
    NoMatchingViewException exception(message);
    exception.mViewMatcher = mViewMatcher;
    exception.mViewHierarchy = mViewHierarchy;
    return exception;
}

AmbiguousViewMatcherException AmbiguousViewMatcherException::Builder::build() {
    std::string message = StringDescription::toString(*mViewMatcher)
        + " matched multiple views in the hierarchy!";
    if (mView1 != nullptr && mView2 != nullptr) {
        View* root = mRootView ? mRootView : mView1->getRootView();
        std::vector<View*> problemViews = {mView1, mView2};
        message = HumanReadables::getViewHierarchyErrorMessage(root, problemViews,
                "view matcher " + message, "<<======+");  // AOSP problemViewSuffix
    }
    AmbiguousViewMatcherException exception(message);
    return exception;
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
