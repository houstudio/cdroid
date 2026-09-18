#include <deskclock.h>

#include <R.h>

#include <animation/objectanimator.h>
#include <core/activityfactory.h>
#include <core/intent.h>
#include <widget/internal_R.h>
#include <menu/menuitem.h>
#include <view/layoutinflater.h>
#include <view/view.h>
#include <widget/cdwindow.h>
#include <widget/viewpager.h>

#include <animatorutils.h>
#include <datamodel.h>
#include <snackbarmanager.h>
#include <uidata.h>
#include <utils.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {

using uidata::UiDataModel;
using data::DataModel;

DeskClock::DeskClock() : FragmentActivity(0, 0, -1, -1) {
   // [window-bg-experiment] manual opaque backdrop disabled — the theme already
   // carries android:windowBackground=@color/default_background (assets/values/
   // themes.xml) and the Window paints it itself now:
   // setBackgroundColor(0xFF000000);
}

void DeskClock::onNewIntent(const Intent& newIntent) {
    FragmentActivity::onNewIntent(newIntent);

    // Fragments may query the latest intent for information, so update the intent.
    setIntent(newIntent);
}

void DeskClock::onCreate(Bundle* savedInstanceState) {
    FragmentActivity::onCreate(savedInstanceState);

    // Window has no setContentView on cdroid: inflate and attach directly.
    View* content = LayoutInflater::from(getContext())->inflate(R::layout::desk_clock, nullptr, false);
    addView(content);
    mSnackbarAnchor = findViewById(R::id::content);

    // Configure the toolbar.
    Toolbar* toolbar = (Toolbar*) findViewById(R::id::toolbar);
    setActionBar(toolbar);

    ActionBar* actionBar = getActionBar();
    if (actionBar != nullptr) {
        actionBar->setDisplayShowTitleEnabled(false);
    }

    // Configure the menu item controllers to add behavior to the toolbar.
    std::vector<actionbarmenu::MenuItemController*> controllers;
    controllers.push_back(new actionbarmenu::NightModeMenuItemController(*getContext()));
    controllers.push_back(new actionbarmenu::SettingsMenuItemController(static_cast<cdroid::Window*>(this)));
    mOptionsMenuManager.addMenuItemController(controllers);
    mOptionsMenuManager.addMenuItemController(
            actionbarmenu::MenuItemControllerFactory::buildMenuItemControllers(getContext()));

    // Inflate the menu during creation to avoid a double layout pass.
    onCreateOptionsMenu(*toolbar->getMenu());

    // Create the tabs that make up the user interface.
    mTabLayout = (TabLayout*) findViewById(R::id::tabs);
    const int tabCount = UiDataModel::getUiDataModel().getTabCount();
    const bool showTabLabel = getContext()->getResources().getBoolean(R::boolean::showTabLabel);
    const bool showTabHorizontally = getContext()->getResources().getBoolean(R::boolean::showTabHorizontally);
    for (int i = 0; i < tabCount; i++) {
        const uidata::Tab tabModel = UiDataModel::getUiDataModel().getTab(i);
        const int labelResId = tabModel.labelResId;

        TabLayout::Tab* tab = mTabLayout->newTab();
        tab->setTag((void*) (intptr_t) tabModel.value);
        tab->setIcon(tabModel.iconResId);
        tab->setContentDescription(labelResId);

        if (showTabLabel) {
            tab->setText(labelResId);
            tab->setCustomView(R::layout::tab_item);

            TextView* text = (TextView*) tab->getCustomView()->findViewById(internal::R::id::text1);
            if (text) text->setTextColor(mTabLayout->getTabTextColors());

            // Bind the icon to the TextView.
            Drawable* icon = tab->getIcon();
            if (showTabHorizontally) {
                // Remove the icon so it doesn't affect the minimum TabLayout height.
                tab->setIcon(nullptr);
                text->setCompoundDrawablesWithIntrinsicBounds(icon, nullptr, nullptr, nullptr);
            } else {
                text->setCompoundDrawablesWithIntrinsicBounds(nullptr, icon, nullptr, nullptr);
            }
        }

        mTabLayout->addTab(tab);
    }

    // Configure the buttons shared by the tabs.
    mFab = (ImageView*) findViewById(R::id::fab);
    mLeftButton = (Button*) findViewById(R::id::left_button);
    mRightButton = (Button*) findViewById(R::id::right_button);

    mFab->setOnClickListener([this](View&) {
        getSelectedDeskClockFragment().onFabClick(*mFab);
    });
    mLeftButton->setOnClickListener([this](View&) {
        getSelectedDeskClockFragment().onLeftButtonClick(*mLeftButton);
    });
    mRightButton->setOnClickListener([this](View&) {
        getSelectedDeskClockFragment().onRightButtonClick(*mRightButton);
    });

    const int64_t duration = UiDataModel::getUiDataModel().getShortAnimationDuration();

    ValueAnimator* hideFabAnimation = AnimatorUtils::getScaleAnimator(mFab, {1.0f, 0.0f});
    ValueAnimator* showFabAnimation = AnimatorUtils::getScaleAnimator(mFab, {0.0f, 1.0f});

    ValueAnimator* leftHideAnimation = AnimatorUtils::getScaleAnimator(mLeftButton, {1.0f, 0.0f});
    ValueAnimator* rightHideAnimation = AnimatorUtils::getScaleAnimator(mRightButton, {1.0f, 0.0f});
    ValueAnimator* leftShowAnimation = AnimatorUtils::getScaleAnimator(mLeftButton, {0.0f, 1.0f});
    ValueAnimator* rightShowAnimation = AnimatorUtils::getScaleAnimator(mRightButton, {0.0f, 1.0f});

    {
        Animator::AnimatorListener l;
        l.onAnimationEnd = [this](Animator&, bool) {
            getSelectedDeskClockFragment().onUpdateFab(*mFab);
        };
        hideFabAnimation->addListener(l);
    }
    {
        Animator::AnimatorListener l;
        l.onAnimationEnd = [this](Animator&, bool) {
            getSelectedDeskClockFragment().onUpdateFabButtons(*mLeftButton, *mRightButton);
        };
        leftHideAnimation->addListener(l);
    }

    // Build the reusable animations that hide and show the fab and left/right buttons.
    delete mHideAnimation;
    mHideAnimation = new AnimatorSet();
    mHideAnimation->setDuration(duration);
    mHideAnimation->play(hideFabAnimation)->with(leftHideAnimation).with(rightHideAnimation);

    delete mShowAnimation;
    mShowAnimation = new AnimatorSet();
    mShowAnimation->setDuration(duration);
    mShowAnimation->play(showFabAnimation)->with(leftShowAnimation).with(rightShowAnimation);

    // Build the reusable animation that hides and shows only the fab.
    delete mUpdateFabOnlyAnimation;
    mUpdateFabOnlyAnimation = new AnimatorSet();
    mUpdateFabOnlyAnimation->setDuration(duration);
    mUpdateFabOnlyAnimation->play(showFabAnimation)->after(hideFabAnimation);

    // Build the reusable animation that hides and shows only the buttons.
    delete mUpdateButtonsOnlyAnimation;
    mUpdateButtonsOnlyAnimation = new AnimatorSet();
    mUpdateButtonsOnlyAnimation->setDuration(duration);
    mUpdateButtonsOnlyAnimation->play(leftShowAnimation)->with(rightShowAnimation)
            .after(leftHideAnimation).after(rightHideAnimation);

    // Customize the view pager.
    mFragmentTabPagerAdapter = new FragmentTabPagerAdapter(*this);
    mFragmentTabPager = (ViewPager*) findViewById(R::id::desk_clock_pager);
    // Keep all four tabs to minimize jank.
    mFragmentTabPager->setOffscreenPageLimit(3);
    // Mirror changes made to the selected page of the view pager into UiDataModel.
    mPageChangeWatcher.onPageScrolled = [this](int, float, int positionOffsetPixels) {
        // Only hide the fab when a non-zero drag distance is detected. This prevents
        // over-scrolling from needlessly hiding the fab.
        if (mFabState == FabState::HIDE_ARMED && positionOffsetPixels != 0) {
            mFabState = FabState::HIDING;
            mHideAnimation->start();
        }
    };
    mPageChangeWatcher.onPageScrollStateChanged = [this](int state) {
        if (mPriorScrollState == ViewPager::SCROLL_STATE_IDLE
                && state == ViewPager::SCROLL_STATE_SETTLING) {
            // The user has tapped a tab button; play the hide and show animations linearly.
            mHideAnimation->addListener(mAutoStartShowListener);
            mHideAnimation->start();
            mFabState = FabState::HIDING;
        } else if (mPriorScrollState == ViewPager::SCROLL_STATE_SETTLING
                && state == ViewPager::SCROLL_STATE_DRAGGING) {
            // The user has interrupted settling on a tab and the fab button must be re-hidden.
            if (mShowAnimation->isStarted()) {
                mShowAnimation->cancel();
            }
            if (mHideAnimation->isStarted()) {
                // Let the hide animation finish naturally; don't auto show when it ends.
                mHideAnimation->removeListener(mAutoStartShowListener);
            } else {
                // Start and immediately end the hide animation to jump to the hidden state.
                mHideAnimation->start();
                mHideAnimation->end();
            }
            mFabState = FabState::HIDING;
        } else if (state != ViewPager::SCROLL_STATE_DRAGGING && mFabState == FabState::HIDING) {
            // The user has lifted their finger; show the buttons now or after hide ends.
            if (mHideAnimation->isStarted()) {
                // Finish the hide animation and then start the show animation.
                mHideAnimation->addListener(mAutoStartShowListener);
            } else {
                updateFab(FabContainer::FAB_AND_BUTTONS_IMMEDIATE);
                mShowAnimation->start();

                // The animation to show the fab has begun; update the state to showing.
                mFabState = FabState::SHOWING;
            }
        } else if (state == ViewPager::SCROLL_STATE_DRAGGING) {
            // The user has started a drag so arm the hide animation.
            mFabState = FabState::HIDE_ARMED;
        }

        // Update the last known state.
        mPriorScrollState = state;
    };
    mPageChangeWatcher.onPageSelected = [this](int position) {
        mFragmentTabPagerAdapter->getDeskClockFragment(position)->selectTab();
    };
    mFragmentTabPager->addOnPageChangeListener(mPageChangeWatcher);
    mFragmentTabPager->setAdapter(mFragmentTabPagerAdapter);

    // Mirror changes made to the selected tab into UiDataModel.
    TabLayout::OnTabSelectedListener tabWatcher;
    tabWatcher.onTabSelected = [this](TabLayout::Tab& tab) {
        UiDataModel::getUiDataModel().setSelectedTab((int) (intptr_t) tab.getTag());
    };
    mTabLayout->addOnTabSelectedListener(tabWatcher);

    // If the hide animation has already completed, the buttons must be updated now when the
    // new tab is known. Otherwise they are updated at the end of the hide animation.
    mAutoStartShowListener.onAnimationEnd = [this](Animator& animator, bool) {
        // Prepare the hide animation for its next use; by default do not auto-show after hide.
        mHideAnimation->removeListener(mAutoStartShowListener);

        // Update the buttons now that they are no longer visible.
        updateFab(FabContainer::FAB_AND_BUTTONS_IMMEDIATE);

        // Automatically start the grow animation now that shrinking is complete.
        mShowAnimation->start();

        // The animation to show the fab has begun; update the state to showing.
        mFabState = FabState::SHOWING;
    };

    // Honor changes to the selected tab from outside entities.
    mTabChangeWatcher = [this](int /*oldSelectedTab*/, int /*newSelectedTab*/) {
        // TabChangeWatcher.selectedTabChanged
        // Update the view pager and tab layout to agree with the model.
        updateCurrentTab();

        // If the hide animation has already completed, the buttons must be updated now when the
        // new tab is known. Otherwise they are updated at the end of the hide animation.
        if (!mHideAnimation->isStarted()) {
            updateFab(FabContainer::FAB_AND_BUTTONS_IMMEDIATE);
        }
    };
    UiDataModel::getUiDataModel().addTabListener(mTabChangeWatcher);

    // Silent-setting snackbar watcher (notifications cut; the listener never fires).
    mSilentSettingChangeWatcher.onSilentSettingsChange = [this](void*, void*) {
        SnackbarManager::dismiss();
    };
}

void DeskClock::onStart() {
    FragmentActivity::onStart();
    DataModel::getDataModel().addSilentSettingsListener(mSilentSettingChangeWatcher);
    DataModel::getDataModel().setApplicationInForeground(true);
}

void DeskClock::onResume() {
    FragmentActivity::onResume();

    View* dropShadow = findViewById(R::id::drop_shadow);
    View* hairline = mSnackbarAnchor ? mSnackbarAnchor->findViewById(R::id::tab_hairline) : nullptr;
    if (hairline != nullptr) {
        mDropShadowController = new DropShadowController(*dropShadow,
                UiDataModel::getUiDataModel(), *hairline);
    }

    // ViewPager does not save state; this honors the selected tab in the user interface.
    updateCurrentTab();

    // Upstream performs this in onPostResume (not part of the cdroid Window lifecycle).
    if (mRecreateActivity) {
        mRecreateActivity = false;

        // A runnable must be posted here or the new DeskClock activity will be recreated in a
        // paused state, even though it is the foreground activity.
        Runnable r = [this]() { recreate(); };
        mFragmentTabPager->post(r);
    }
}

void DeskClock::onPause() {
    if (mDropShadowController != nullptr) {
        mDropShadowController->stop();
        delete mDropShadowController;
        mDropShadowController = nullptr;
    }

    FragmentActivity::onPause();
}

void DeskClock::onStop() {
    // (No isChangingConfigurations on cdroid; recreate() re-enters through onStart.)
    DataModel::getDataModel().setApplicationInForeground(false);

    FragmentActivity::onStop();
}

void DeskClock::onDestroy() {
    UiDataModel::getUiDataModel().removeTabListener(mTabChangeWatcher);
    FragmentActivity::onDestroy();
}

bool DeskClock::onCreateOptionsMenu(Menu& menu) {
    mOptionsMenuManager.onCreateOptionsMenu(menu);
    return true;
}

bool DeskClock::onPrepareOptionsMenu(Menu& menu) {
    FragmentActivity::onPrepareOptionsMenu(menu);
    mOptionsMenuManager.onPrepareOptionsMenu(menu);
    return true;
}

bool DeskClock::onOptionsItemSelected(MenuItem& item) {
    return mOptionsMenuManager.onOptionsItemSelected(item)
            || FragmentActivity::onOptionsItemSelected(item);
}

bool DeskClock::onKeyDown(int keyCode, KeyEvent& event) {
    return getSelectedDeskClockFragment().onKeyDown(keyCode, event)
            || FragmentActivity::onKeyDown(keyCode, event);
}

void DeskClock::updateFab(int updateType) {
    DeskClockFragment& f = getSelectedDeskClockFragment();

    switch (updateType & FabContainer::FAB_ANIMATION_MASK) {
        case FabContainer::FAB_SHRINK_AND_EXPAND: mUpdateFabOnlyAnimation->start(); break;
        case FabContainer::FAB_IMMEDIATE: f.onUpdateFab(*mFab); break;
        case FabContainer::FAB_MORPH: f.onMorphFab(*mFab); break;
        default: break;
    }
    if ((updateType & FabContainer::FAB_REQUEST_FOCUS_MASK) == FabContainer::FAB_REQUEST_FOCUS) {
        mFab->requestFocus();
    }
    switch (updateType & FabContainer::BUTTONS_ANIMATION_MASK) {
        case FabContainer::BUTTONS_IMMEDIATE:
            f.onUpdateFabButtons(*mLeftButton, *mRightButton);
            break;
        case FabContainer::BUTTONS_SHRINK_AND_EXPAND:
            mUpdateButtonsOnlyAnimation->start();
            break;
        default: break;
    }
    if ((updateType & FabContainer::BUTTONS_DISABLE_MASK) == FabContainer::BUTTONS_DISABLE) {
        mLeftButton->setClickable(false);
        mRightButton->setClickable(false);
    }
    switch (updateType & FabContainer::FAB_AND_BUTTONS_SHRINK_EXPAND_MASK) {
        case FabContainer::FAB_AND_BUTTONS_SHRINK: mHideAnimation->start(); break;
        case FabContainer::FAB_AND_BUTTONS_EXPAND: mShowAnimation->start(); break;
        default: break;
    }
}

void DeskClock::onActivityResult(int requestCode, int resultCode, Intent* /*data*/) {
    // Recreate the activity if any settings have been changed
    if (requestCode == actionbarmenu::SettingsMenuItemController::REQUEST_CHANGE_SETTINGS
            && resultCode == RESULT_OK) {
        mRecreateActivity = true;
    }
}

void DeskClock::updateCurrentTab() {
    // Fetch the selected tab from the source of truth: UiDataModel.
    const int selectedTab = UiDataModel::getUiDataModel().getSelectedTab();

    // Update the selected tab in the tablayout if it does not agree with UiDataModel.
    for (int i = 0; i < mTabLayout->getTabCount(); i++) {
        TabLayout::Tab* tab = mTabLayout->getTabAt(i);
        if (tab != nullptr && (int) (intptr_t) tab->getTag() == selectedTab && !tab->isSelected()) {
            tab->select();
            break;
        }
    }

    // Update the selected fragment in the viewpager if it does not agree with UiDataModel.
    for (int i = 0; i < mFragmentTabPagerAdapter->getCount(); i++) {
        DeskClockFragment* fragment = mFragmentTabPagerAdapter->getDeskClockFragment(i);
        if (fragment->isTabSelected() && mFragmentTabPager->getCurrentItem() != i) {
            mFragmentTabPager->setCurrentItem(i);
            break;
        }
    }
}

DeskClockFragment& DeskClock::getSelectedDeskClockFragment() {
    for (int i = 0; i < mFragmentTabPagerAdapter->getCount(); i++) {
        DeskClockFragment* fragment = mFragmentTabPagerAdapter->getDeskClockFragment(i);
        if (fragment->isTabSelected()) {
            return *fragment;
        }
    }
    const int selectedTab = UiDataModel::getUiDataModel().getSelectedTab();
    throw std::logic_error("Unable to locate selected fragment (" + std::to_string(selectedTab) + ")");
}

REGISTER_ACTIVITY(DeskClock);

} // namespace deskclock
} // namespace cdroid
