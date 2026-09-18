#include <dropshadowcontroller.h>

#include <view/view.h>
#include <widgetEx/recyclerview/recyclerview.h>

#include <datamodel.h>
#include <utils.h>

namespace cdroid {
namespace deskclock {

DropShadowController::DropShadowController(View& dropShadowView,
                                           uidata::UiDataModel& uiDataModel,
                                           View& hairlineView)
    : mDropShadowView(dropShadowView), mUiDataModel(&uiDataModel) {
    mDropShadowAnimator = AnimatorUtils::getAlphaAnimator(&dropShadowView, {0.0f, 1.0f});
    mDropShadowAnimator->setDuration(
            uidata::UiDataModel::getUiDataModel().getShortAnimationDuration());

    mScrollChangeWatcher = [this](int /*selectedTab*/, bool scrolledToTop) {
        // TabScrollListener.selectedTabScrollToTopChanged
        updateDropShadow(!scrolledToTop);
    };
    mUiDataModel->addTabScrollListener(mScrollChangeWatcher);
    mHairlineView = &hairlineView;
    updateDropShadow(!uiDataModel.isSelectedTabScrolledToTop());
}

DropShadowController::DropShadowController(View& dropShadowView, RecyclerView& recyclerView)
    : mDropShadowView(dropShadowView) {
    mDropShadowAnimator = AnimatorUtils::getAlphaAnimator(&dropShadowView, {0.0f, 1.0f});
    mDropShadowAnimator->setDuration(
            uidata::UiDataModel::getUiDataModel().getShortAnimationDuration());

    mSourceRecyclerView = &recyclerView;
    mScrollListener.onScrolled = [this](RecyclerView& rv, int, int) {
        // ScrollChangeWatcher: shadow shows while the list can scroll up.
        updateDropShadow(rv.canScrollVertically(-1));
    };
    recyclerView.addOnScrollListener(mScrollListener);
    updateDropShadow(recyclerView.canScrollVertically(-1));
}

DropShadowController::~DropShadowController() {
    delete mDropShadowAnimator;
}

void DropShadowController::stop() {
    if (mUiDataModel != nullptr) {
        mUiDataModel->removeTabScrollListener(mScrollChangeWatcher);
        mUiDataModel = nullptr;
    }
    if (mSourceRecyclerView != nullptr) {
        mSourceRecyclerView->removeOnScrollListener(mScrollListener);
        mSourceRecyclerView = nullptr;
    }
}

void DropShadowController::updateDropShadow(bool shouldShowDropShadow) {
    if (!shouldShowDropShadow && mDropShadowView.getAlpha() != 0.0f) {
        if (data::DataModel::getDataModel().isApplicationInForeground()) {
            AnimatorUtils::reverse({mDropShadowAnimator});
        } else {
            mDropShadowView.setAlpha(0.0f);
        }
        if (mHairlineView) mHairlineView->setVisibility(View::VISIBLE);
    }
    if (shouldShowDropShadow && mDropShadowView.getAlpha() != 1.0f) {
        if (data::DataModel::getDataModel().isApplicationInForeground()) {
            mDropShadowAnimator->start();
        } else {
            mDropShadowView.setAlpha(1.0f);
        }
        if (mHairlineView) mHairlineView->setVisibility(View::INVISIBLE);
    }
}

} // namespace deskclock
} // namespace cdroid
