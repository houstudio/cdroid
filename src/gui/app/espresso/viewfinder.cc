#include <app/espresso/viewfinder.h>

#include <app/espresso/espressoexception.h>
#include <app/espresso/humanreadables.h>
#include <app/espresso/rootviewpicker.h>
#include <app/espresso/treeiterables.h>

#include <core/looper.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <widget/adapterview.h>

namespace cdroid {
namespace espresso {

ViewFinderImpl::ViewFinderImpl(MatcherPtr<View> viewMatcher, RootViewPicker& rootViewPicker)
    : mViewMatcher(std::move(viewMatcher)), mRootViewPicker(rootViewPicker) {}

View* ViewFinderImpl::getView() {
    // AOSP ViewFinderImpl.checkMainThread
    if (Looper::getMainLooper() != Looper::myLooper()) {
        throw std::runtime_error("Method invocation not expected from the main thread!");
    }

    View* rootView = mRootViewPicker.get();
    View* matchedView = nullptr;

    for (View* view : breadthFirstViewTraversal(rootView)) {
        if (mViewMatcher->matches(*view)) {
            if (matchedView != nullptr) {
                throw AmbiguousViewMatcherException::Builder()
                        .withViewMatcher(mViewMatcher)
                        .withView1(matchedView)
                        .withView2(view)
                        .withRootView(rootView)
                        .build();
            }
            matchedView = view;
        }
    }

    if (matchedView == nullptr) {
        // AOSP collects the AdapterViews in the hierarchy for the error
        // message (they hint the user towards onData).
        std::vector<View*> adapterViews;
        for (View* view : breadthFirstViewTraversal(rootView)) {
            if (dynamic_cast<AdapterView*>(view) != nullptr) {
                adapterViews.push_back(view);
            }
        }
        throw NoMatchingViewException::Builder()
                .withViewMatcher(mViewMatcher)
                .withViewHierarchy(HumanReadables::getViewHierarchyErrorMessage(rootView,
                        adapterViews,
                        "No views in hierarchy found matching: "
                        + StringDescription::toString(*mViewMatcher),
                        "**********this is the view we tried to match**********"))
                .build();
    }
    return matchedView;
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
