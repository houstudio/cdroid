#ifndef __DESKCLOCK_DESKCLOCKFRAGMENT_H__
#define __DESKCLOCK_DESKCLOCKFRAGMENT_H__
/*********************************************************************************
 * Port of com.android.deskclock.DeskClockFragment — base class of the four tab
 * fragments: tab identity, fab-container ferry-through, key dispatch.
 *********************************************************************************/
#include <fragment/fragment.h>

#include <fabcontainer.h>
#include <uidata.h>

namespace cdroid {
namespace deskclock {

class DeskClockFragment : public Fragment, public FabContainer, public FabController {
private:
    /** The tab associated with this fragment. */
    const int mTab;

    /** The container that houses the fab and its left and right buttons. */
    FabContainer* mFabContainer = nullptr;

public:
    explicit DeskClockFragment(int tab) : Fragment(), mTab(tab) {}

    void onResume() override;

    virtual bool onKeyDown(int keyCode, KeyEvent& event);

    void onLeftButtonClick(Button& left) override {}
    void onRightButtonClick(Button& right) override {}
    void onMorphFab(ImageView& fab) override {}

    // FabController members left abstract for the tab fragments (as upstream).
    virtual void onUpdateFab(ImageView& fab) = 0;
    virtual void onFabClick(ImageView& fab) = 0;
    virtual void onUpdateFabButtons(Button& left, Button& right) = 0;

    /** @param fabContainer the container that houses the fab and its buttons. */
    void setFabContainer(FabContainer* fabContainer) { mFabContainer = fabContainer; }

    /** Requests that the parent activity update the fab and buttons. */
    void updateFab(int updateTypes) override {
        if (mFabContainer != nullptr) mFabContainer->updateFab(updateTypes);
    }

    /** @return true iff the currently selected tab displays this fragment. */
    bool isTabSelected() const {
        return uidata::UiDataModel::getUiDataModel().getSelectedTab() == mTab;
    }

    /** Select the tab that displays this fragment. */
    void selectTab() { uidata::UiDataModel::getUiDataModel().setSelectedTab(mTab); }

    /** Updates the scrolling state in the UiDataModel for this tab. */
    void setTabScrolledToTop(bool scrolledToTop) {
        uidata::UiDataModel::getUiDataModel().setTabScrolledToTop(mTab, scrolledToTop);
    }
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_DESKCLOCKFRAGMENT_H__
