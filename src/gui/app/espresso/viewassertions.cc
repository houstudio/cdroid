#include <app/espresso/viewassertions.h>

#include <app/espresso/espressoexception.h>
#include <app/espresso/humanreadables.h>
#include <app/espresso/treeiterables.h>
#include <app/espresso/viewmatchers.h>

#include <stdexcept>

namespace cdroid {
namespace espresso {

ViewAssertionPtr ViewAssertions::doesNotExist() {
    class DoesNotExist : public ViewAssertion {
    public:
        void check(View* view, NoMatchingViewException* /*noViewFoundException*/) override {
            if (view != nullptr) {
                throw AssertionFailedError("View is present in the hierarchy: "
                        + HumanReadables::describe(view));
            }
        }
    };
    return std::make_shared<DoesNotExist>();
}

ViewAssertionPtr ViewAssertions::matches(MatcherPtr<View> viewMatcher) {
    class Matches : public ViewAssertion {
    public:
        explicit Matches(MatcherPtr<View> matcher) : mMatcher(std::move(matcher)) {}
        void check(View* view, NoMatchingViewException* noViewFoundException) override {
            if (noViewFoundException != nullptr) {
                throw *noViewFoundException;
            }
            ViewMatchers::assertThat(view, *mMatcher,
                    "'" + StringDescription::toString(*mMatcher)
                    + "' doesn't match the selected view.");
        }
    private:
        MatcherPtr<View> mMatcher;
    };
    return std::make_shared<Matches>(std::move(viewMatcher));
}

ViewAssertionPtr ViewAssertions::selectedDescendantsMatch(MatcherPtr<View> selector,
        MatcherPtr<View> matcher) {
    class SelectedDescendantsMatch : public ViewAssertion {
    public:
        SelectedDescendantsMatch(MatcherPtr<View> selector, MatcherPtr<View> matcher)
            : mSelector(std::move(selector)), mMatcher(std::move(matcher)) {}
        void check(View* view, NoMatchingViewException* noViewFoundException) override {
            if (noViewFoundException != nullptr) {
                throw *noViewFoundException;
            }
            if (view == nullptr) {
                throw std::invalid_argument("view must not be null");
            }
            std::vector<View*> selectedViews;
            for (View* descendant : breadthFirstViewTraversal(view)) {
                if (mSelector->matches(*descendant)) {
                    if (!mMatcher->matches(*descendant)) {
                        selectedViews.push_back(descendant);
                    }
                }
            }
            if (!selectedViews.empty()) {
                const std::string errorMessage = HumanReadables::getViewHierarchyErrorMessage(view,
                        selectedViews,
                        "The following selected descendant views did not match "
                        + StringDescription::toString(*mMatcher) + ".",
                        "*************");
                throw AssertionFailedError(errorMessage);
            }
        }
    private:
        MatcherPtr<View> mSelector;
        MatcherPtr<View> mMatcher;
    };
    return std::make_shared<SelectedDescendantsMatch>(std::move(selector), std::move(matcher));
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
