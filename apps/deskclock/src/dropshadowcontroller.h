#ifndef __DESKCLOCK_DROPSHADOWCONTROLLER_H__
#define __DESKCLOCK_DROPSHADOWCONTROLLER_H__
/*********************************************************************************
 * Port of com.android.deskclock.DropShadowController — fades in/out the drop
 * shadow under the tab bar as the selected tab's scroll state changes
 * (UiDataModel-driven source; the ListView/RecyclerView sources come with the
 * screens that own those lists).
 *********************************************************************************/
#include <animation/valueanimator.h>

#include <animatorutils.h>
#include <uidata.h>

#include <widgetEx/recyclerview/recyclerview.h>

namespace cdroid {

class View;

namespace deskclock {

class DropShadowController {
public:
    class ScrollChangeWatcher;

private:
    /** The component that displays a drop shadow. */
    View& mDropShadowView;

    /** Updates mDropShadowView in response to changes in the backing scroll model. */
    uidata::TabScrollListener mScrollChangeWatcher;

    /** Fades the drop shadow in/out as scroll state changes. */
    ValueAnimator* mDropShadowAnimator;

    /** Tab bar's hairline, hidden whenever the drop shadow is displayed. */
    View* mHairlineView = nullptr;

    uidata::UiDataModel* mUiDataModel = nullptr;

    /** The RecyclerView whose scroll state drives the shadow (RV variant). */
    RecyclerView* mSourceRecyclerView = nullptr;
    RecyclerView::OnScrollListener mScrollListener;

public:
    DropShadowController(View& dropShadowView, uidata::UiDataModel& uiDataModel,
                         View& hairlineView);
    /** The RecyclerView source variant (upstream's second constructor). */
    DropShadowController(View& dropShadowView, RecyclerView& recyclerView);
    ~DropShadowController();

    void stop();

private:
    void updateDropShadow(bool shouldShowDropShadow);
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_DROPSHADOWCONTROLLER_H__
