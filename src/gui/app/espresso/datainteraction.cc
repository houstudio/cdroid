#include <app/espresso/datainteraction.h>

#include <app/espresso/adapterdataloaderaction.h>
#include <app/espresso/adapterviewprotocols.h>
#include <app/espresso/espresso.h>
#include <app/espresso/viewassertion.h>
#include <app/espresso/viewinteraction.h>
#include <app/espresso/viewmatchers.h>
#include <widget/adapterview.h>

namespace cdroid {
namespace espresso {

/* ------------------------------------------------------------------ */
/* DataInteraction.DisplayDataMatcher                                 */
/* ------------------------------------------------------------------ */

class DataInteraction::DisplayDataMatcher : public TypeSafeMatcher<View> {
public:
    /**
     * Returns an instance of DisplayDataMatcher (internal — do not call
     * from test code).
     * @param adapterMatcher matcher that matches an AdapterView
     * @param dataMatcher the data matcher for matching a View by its adapter data
     * @param rootMatcher matcher for the view's root
     * @param hasAtPosition/atPosition optional zero-based position of the data to be matched
     * @param adapterViewProtocol the AdapterViewProtocol used for this data interaction
     */
    static std::shared_ptr<DisplayDataMatcher> displayDataMatcher(MatcherPtr<View> adapterMatcher,
            MatcherPtr<void*> dataMatcher, RootMatcherPtr rootMatcher,
            bool hasAtPosition, int atPosition,
            std::shared_ptr<AdapterViewProtocol> adapterViewProtocol) {
        std::shared_ptr<AdapterDataLoaderAction> loaderAction =
                std::make_shared<AdapterDataLoaderAction>(dataMatcher, hasAtPosition,
                        atPosition, adapterViewProtocol);
        // new (not make_shared): the constructor is private, and only this
        // member function may call it.
        return std::shared_ptr<DisplayDataMatcher>(new DisplayDataMatcher(
                std::move(adapterMatcher), std::move(dataMatcher), std::move(rootMatcher),
                std::move(adapterViewProtocol), std::move(loaderAction)));
    }

    void describeTo(Description& description) const override {
        description.appendText(" displaying data matching: ");
        mDataMatcher->describeTo(description);
        description.appendText(" within adapter view matching: ");
        mAdapterMatcher->describeTo(description);
    }

    bool matchesSafely(const View& view) const override {
        View* parent = view.getParent();
        while (parent != nullptr && dynamic_cast<AdapterView*>(parent) == nullptr) {
            parent = parent->getParent();
        }
        if (parent != nullptr && mAdapterMatcher->matches(*parent)) {
            AdapterView& adapterView = static_cast<AdapterView&>(*parent);
            std::shared_ptr<AdaptedData> data =
                    mAdapterViewProtocol->getDataRenderedByView2(adapterView,
                            const_cast<View&>(view));
            if (data != nullptr) {
                return data->opaqueToken
                        == mAdapterDataLoaderAction->getAdaptedData().opaqueToken;
            }
        }
        return false;
    }

private:
    DisplayDataMatcher(MatcherPtr<View> adapterMatcher, MatcherPtr<void*> dataMatcher,
            RootMatcherPtr rootMatcher, std::shared_ptr<AdapterViewProtocol> adapterViewProtocol,
            std::shared_ptr<AdapterDataLoaderAction> adapterDataLoaderAction)
        : mAdapterMatcher(std::move(adapterMatcher)),
          mDataMatcher(std::move(dataMatcher)),
          mAdapterViewProtocol(std::move(adapterViewProtocol)),
          mAdapterDataLoaderAction(std::move(adapterDataLoaderAction)) {
        // AOSP: the constructor chain ends by invoking loadDataFunction —
        // onView(adapterMatcher).inRoot(rootMatcher).perform(
        //     adapterDataLoaderAction) — so the data load (scroll into view)
        // happens while the MATCHER is being built; matching itself only
        // compares opaque tokens afterwards.
        Espresso::onView(mAdapterMatcher).inRoot(rootMatcher)
                .perform({mAdapterDataLoaderAction});
    }

    MatcherPtr<View> mAdapterMatcher;
    MatcherPtr<void*> mDataMatcher;
    std::shared_ptr<AdapterViewProtocol> mAdapterViewProtocol;
    std::shared_ptr<AdapterDataLoaderAction> mAdapterDataLoaderAction;
};

/* ------------------------------------------------------------------ */
/* DataInteraction                                                     */
/* ------------------------------------------------------------------ */

DataInteraction::DataInteraction(MatcherPtr<void*> dataMatcher)
    : mDataMatcher(std::move(dataMatcher)),
      mAdapterMatcher(ViewMatchers::isAssignableFrom<AdapterView>()),
      mAdapterViewProtocol(AdapterViewProtocols::standardProtocol()),
      mRootMatcher(RootMatchers::DEFAULT()) {
}

DataInteraction& DataInteraction::onChildView(MatcherPtr<View> childMatcher) {
    mChildViewMatcher = std::move(childMatcher);
    return *this;
}

DataInteraction& DataInteraction::inRoot(RootMatcherPtr rootMatcher) {
    mRootMatcher = std::move(rootMatcher);
    return *this;
}

DataInteraction& DataInteraction::inAdapterView(MatcherPtr<View> adapterMatcher) {
    mAdapterMatcher = std::move(adapterMatcher);
    return *this;
}

DataInteraction& DataInteraction::atPosition(int atPosition) {
    mHasAtPosition = true;
    mAtPosition = atPosition;
    return *this;
}

DataInteraction& DataInteraction::usingAdapterViewProtocol(
        std::shared_ptr<AdapterViewProtocol> adapterViewProtocol) {
    mAdapterViewProtocol = std::move(adapterViewProtocol);
    return *this;
}

ViewInteraction DataInteraction::perform(const std::vector<ViewActionPtr>& actions) {
    return Espresso::onView(makeTargetMatcher()).inRoot(mRootMatcher).perform(actions);
}

ViewInteraction DataInteraction::check(std::shared_ptr<ViewAssertion> assertion) {
    return Espresso::onView(makeTargetMatcher()).inRoot(mRootMatcher).check(assertion);
}

MatcherPtr<View> DataInteraction::makeTargetMatcher() {
    MatcherPtr<View> targetView = DisplayDataMatcher::displayDataMatcher(
            mAdapterMatcher, mDataMatcher, mRootMatcher, mHasAtPosition, mAtPosition,
            mAdapterViewProtocol);
    if (mChildViewMatcher != nullptr) {
        targetView = allOf(mChildViewMatcher, ViewMatchers::isDescendantOfA(targetView));
    }
    return targetView;
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
