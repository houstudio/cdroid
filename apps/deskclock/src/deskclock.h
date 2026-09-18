#ifndef __DESKCLOCK_DESKCLOCK_H__
#define __DESKCLOCK_DESKCLOCK_H__
/*********************************************************************************
 * Port of com.android.deskclock.DeskClock — the main singleTask activity that
 * hosts the alarm/clock/timer/stopwatch tabs: TabLayout + ViewPager + the
 * shared fab/left/right button bar with hide/update/show animation chains.
 *********************************************************************************/
#include <animation/animatorset.h>

#include <widget/button.h>
#include <widget/imageview.h>
#include <widget/toolbar.h>
#include <widgetEx/tablayout/tablayout.h>

#include <fragment/fragmentactivity.h>

#include <actionbarmenu.h>
#include <deskclockfragment.h>
#include <dropshadowcontroller.h>
#include <datalisteners.h>
#include <fabcontainer.h>
#include <fragmenttabpageradapter.h>

namespace cdroid {
namespace deskclock {

// TODO(label-dialog): add the AlarmLabelDialogHandler interface when the
// LabelDialogFragment port lands with the alarm module.
class DeskClock : public FragmentActivity, public FabContainer {
private:
    /** Models the interesting display state of the fab button. */
    enum class FabState { SHOWING, HIDE_ARMED, HIDING };

    /** Coordinates handling of context menu items. */
    actionbarmenu::OptionsMenuManager mOptionsMenuManager;

    /** Shrinks the fab and left/right buttons to nothing. */
    AnimatorSet* mHideAnimation = nullptr;
    /** Grows the fab and left/right buttons to natural sizes. */
    AnimatorSet* mShowAnimation = nullptr;
    /** Hides, updates, and shows only the fab; the buttons are untouched. */
    AnimatorSet* mUpdateFabOnlyAnimation = nullptr;
    /** Hides, updates, and shows only the left and right buttons. */
    AnimatorSet* mUpdateButtonsOnlyAnimation = nullptr;

    /** Automatically starts the show animation after the hide animation ends. */
    Animator::AnimatorListener mAutoStartShowListener;

    /** Updates the UI to reflect the selected tab from the backing model. */
    uidata::TabListener mTabChangeWatcher;

    /** Displays a snackbar explaining why alarms may not fire (stubbed out). */
    data::OnSilentSettingsListener mSilentSettingChangeWatcher;

    /** The view to which snackbar items are anchored. */
    View* mSnackbarAnchor = nullptr;

    /** The current display state of the fab. */
    FabState mFabState = FabState::SHOWING;

    /** The single floating-action button shared across all tabs. */
    ImageView* mFab = nullptr;
    /** The button left of the fab shared across all tabs. */
    Button* mLeftButton = nullptr;
    /** The button right of the fab shared across all tabs. */
    Button* mRightButton = nullptr;

    /** Shows the drop shadow when content is not scrolled to the top. */
    DropShadowController* mDropShadowController = nullptr;

    /** The ViewPager that pages through the fragments representing the tabs. */
    ViewPager* mFragmentTabPager = nullptr;

    /** Generates the fragments displayed by the pager. */
    FragmentTabPagerAdapter* mFragmentTabPagerAdapter = nullptr;

    /** The container that stores the tab headers. */
    TabLayout* mTabLayout = nullptr;

    /** Mirrors pager page changes into the UiDataModel. */
    ViewPager::OnPageChangeListener mPageChangeWatcher;

    /** true when a settings change necessitates recreating this activity. */
    bool mRecreateActivity = false;

    /** Prior scroll state for exotic state-change detection. */
    int mPriorScrollState = ViewPager::SCROLL_STATE_IDLE;

public:
    // android.app.Activity.RESULT_OK (not surfaced by the cdroid Window port).
    static constexpr int RESULT_OK = -1;

    DeskClock();

    void onNewIntent(const Intent& intent) override;
    void onStart() override;
    void onCreate(Bundle* savedInstanceState) override;
    void onResume() override;
    void onPause() override;
    void onStop() override;
    void onDestroy() override;

    bool onCreateOptionsMenu(Menu& menu) override;
    bool onPrepareOptionsMenu(Menu& menu) override;
    bool onOptionsItemSelected(MenuItem& item) override;

    bool onKeyDown(int keyCode, KeyEvent& event) override;

    void updateFab(int updateTypes) override;

    void onActivityResult(int requestCode, int resultCode, Intent* data) override;

protected:
    /** Configures the pager and tab layout to display the model's selected tab. */
    void updateCurrentTab();

    /** @return the DeskClockFragment currently selected per the UiDataModel. */
    DeskClockFragment& getSelectedDeskClockFragment();
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_DESKCLOCK_H__
