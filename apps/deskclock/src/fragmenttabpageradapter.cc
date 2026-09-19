#include <fragmenttabpageradapter.h>

#include <view/view.h>

#include <deskclock.h>
#include <fragment/fragmentfactory.h>

namespace cdroid {
namespace deskclock {

namespace {

/** The position-independent tag for the tab's fragment. */
std::string tabName(int tab) {
    switch (tab) {
        case uidata::Tab::ALARMS:    return "ALARMS";
        case uidata::Tab::CLOCKS:    return "CLOCKS";
        case uidata::Tab::TIMERS:    return "TIMERS";
        case uidata::Tab::STOPWATCH: return "STOPWATCH";
    }
    return "CLOCKS";
}

} // namespace

FragmentTabPagerAdapter::FragmentTabPagerAdapter(DeskClock& deskClock)
    : mDeskClock(deskClock), mFragmentManager(deskClock.getSupportFragmentManager()) {
}

FragmentTabPagerAdapter::~FragmentTabPagerAdapter() {
    // Transactions are owned by the FragmentManager; nothing to free here.
}

int FragmentTabPagerAdapter::getCount() {
    return uidata::UiDataModel::getUiDataModel().getTabCount();
}

DeskClockFragment* FragmentTabPagerAdapter::getDeskClockFragment(int position) {
    // Fetch the tab the UiDataModel reports for the position.
    const int tab = (int) uidata::UiDataModel::getUiDataModel().getTabAt(position).value;

    // First check the local cache for the fragment.
    auto cached = mFragmentCache.find(tab);
    if (cached != mFragmentCache.end() && cached->second != nullptr) {
        return cached->second;
    }

    // Next check the fragment manager; relevant when the app is rebuilt because
    // this adapter is new and the cache is empty but the manager retains fragments.
    Fragment* existing = mFragmentManager->findFragmentByTag(tabName(tab));
    if (existing != nullptr) {
        DeskClockFragment* fragment = static_cast<DeskClockFragment*>(existing);
        fragment->setFabContainer(&mDeskClock);
        mFragmentCache[tab] = fragment;
        return fragment;
    }

    // Otherwise, build the fragment from scratch.
    const std::string fragmentClassName = uidata::UiDataModel::getUiDataModel().getTab(tab)
            .fragmentClassName;
    FragmentFactory factory;
    Fragment* instance = factory.instantiate(fragmentClassName);
    DeskClockFragment* fragment = static_cast<DeskClockFragment*>(instance);
    fragment->setFabContainer(&mDeskClock);
    mFragmentCache[tab] = fragment;
    return fragment;
}

void FragmentTabPagerAdapter::startUpdate(ViewGroup* /*container*/) {
    // (ViewPager container id check upstream; the container here is our own pager.)
}

void* FragmentTabPagerAdapter::instantiateItem(ViewGroup* container, int position) {
    if (mCurrentTransaction == nullptr) {
        mCurrentTransaction = mFragmentManager->beginTransaction();
    }

    // Use the fragment located in the fragment manager if one exists.
    const int tab = (int) uidata::UiDataModel::getUiDataModel().getTabAt(position).value;
    Fragment* fragment = mFragmentManager->findFragmentByTag(tabName(tab));
    if (fragment != nullptr) {
        mCurrentTransaction->attach(fragment);
    } else {
        fragment = getDeskClockFragment(position);
        mCurrentTransaction->add(container->getId(), fragment, tabName(tab));
    }

    if (fragment != mCurrentPrimaryItem) {
        fragment->setMenuVisibility(false);
        fragment->setUserVisibleHint(false);
    }

    return fragment;
}

void FragmentTabPagerAdapter::destroyItem(ViewGroup* /*container*/, int /*position*/,
                                          void* object) {
    if (mCurrentTransaction == nullptr) {
        mCurrentTransaction = mFragmentManager->beginTransaction();
    }
    DeskClockFragment* fragment = static_cast<DeskClockFragment*>(object);
    fragment->setFabContainer(nullptr);
    mCurrentTransaction->detach(fragment);
}

void FragmentTabPagerAdapter::setPrimaryItem(ViewGroup* /*container*/, int /*position*/,
                                             void* object) {
    Fragment* fragment = static_cast<Fragment*>(object);
    if (fragment != mCurrentPrimaryItem) {
        if (mCurrentPrimaryItem != nullptr) {
            mCurrentPrimaryItem->setMenuVisibility(false);
            mCurrentPrimaryItem->setUserVisibleHint(false);
        }
        fragment->setMenuVisibility(true);
        fragment->setUserVisibleHint(true);
        mCurrentPrimaryItem = fragment;
    }
}

void FragmentTabPagerAdapter::finishUpdate(ViewGroup* /*container*/) {
    if (mCurrentTransaction != nullptr) {
        mCurrentTransaction->commitAllowingStateLoss();
        mCurrentTransaction = nullptr;
        mFragmentManager->executePendingTransactions();
    }
}

bool FragmentTabPagerAdapter::isViewFromObject(View* view, void* object) {
    Fragment* fragment = static_cast<Fragment*>(object);
    return fragment->getView() == view;
}

} // namespace deskclock
} // namespace cdroid
