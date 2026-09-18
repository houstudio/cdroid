#include <timerpageradapter.h>

#include <datamodel.h>

namespace cdroid {
namespace deskclock {
namespace timer {

TimerPagerAdapter::TimerPagerAdapter(FragmentManager* fragmentManager)
    : mFragmentManager(fragmentManager) {
    mTimerListener.timerAdded = [this](const data::Timer&) {
        notifyDataSetChanged();
    };
    mTimerListener.timerRemoved = [this](const data::Timer&) {
        notifyDataSetChanged();
    };
    mTimerListener.timerUpdated = [this](const data::Timer&, const data::Timer& after) {
        TimerItemFragment* fragment = mFragments[after.id];
        if (fragment != nullptr) fragment->updateTime();
    };
}

TimerPagerAdapter::~TimerPagerAdapter() {
    delete mCurrentTransaction;
}

int TimerPagerAdapter::getCount() {
    return (int) getTimers().size();
}

const std::vector<data::Timer>& TimerPagerAdapter::getTimers() const {
    return data::DataModel::getDataModel().getTimers();
}

bool TimerPagerAdapter::isViewFromObject(View* view, void* object) {
    return ((Fragment*) object)->getView() == view;
}

int TimerPagerAdapter::getItemPosition(void* object) {
    TimerItemFragment* fragment = (TimerItemFragment*) object;
    const data::Timer timer = fragment->getTimer();

    const std::vector<data::Timer>& timers = getTimers();
    for (size_t i = 0; i < timers.size(); i++) {
        if (timers[i].id == timer.id) return (int) i;
    }
    return POSITION_NONE;
}

void* TimerPagerAdapter::instantiateItem(ViewGroup* container, int position) {
    if (mCurrentTransaction == nullptr) {
        mCurrentTransaction = mFragmentManager->beginTransaction();
    }

    const data::Timer timer = getTimer(position);

    // Search for the existing fragment by tag.
    const std::string tag = "TimerItemFragment" + std::to_string(timer.id);
    TimerItemFragment* fragment =
            (TimerItemFragment*) mFragmentManager->findFragmentByTag(tag);

    if (fragment != nullptr) {
        // Reattach the existing fragment.
        mCurrentTransaction->attach(fragment);
    } else {
        // Create and add a new fragment.
        fragment = TimerItemFragment::newInstance(timer);
        mCurrentTransaction->add(container->getId(), fragment, tag);
    }

    if (fragment != mCurrentPrimaryItem) {
        setItemVisible(fragment, false);
    }

    mFragments[timer.id] = fragment;

    return fragment;
}

void TimerPagerAdapter::destroyItem(ViewGroup* /*container*/, int /*position*/, void* object) {
    TimerItemFragment* fragment = (TimerItemFragment*) object;

    if (mCurrentTransaction == nullptr) {
        mCurrentTransaction = mFragmentManager->beginTransaction();
    }

    mFragments.erase(fragment->getTimerId());
    mCurrentTransaction->remove(fragment);
}

void TimerPagerAdapter::setPrimaryItem(ViewGroup* /*container*/, int /*position*/, void* object) {
    Fragment* fragment = (Fragment*) object;
    if (fragment != mCurrentPrimaryItem) {
        if (mCurrentPrimaryItem != nullptr) {
            setItemVisible(mCurrentPrimaryItem, false);
        }

        mCurrentPrimaryItem = fragment;

        if (mCurrentPrimaryItem != nullptr) {
            setItemVisible(mCurrentPrimaryItem, true);
        }
    }
}

void TimerPagerAdapter::finishUpdate(ViewGroup* /*container*/) {
    if (mCurrentTransaction != nullptr) {
        mCurrentTransaction->commitAllowingStateLoss();
        mCurrentTransaction = nullptr;

        // Upstream guards with !isDestroyed; cdroid FragmentManager does not expose
        // that state (see FragmentTabPagerAdapter::finishUpdate).
        mFragmentManager->executePendingTransactions();
    }
}

bool TimerPagerAdapter::updateTime() {
    bool continuousUpdates = false;
    for (const auto& entry : mFragments) {
        TimerItemFragment* fragment = entry.second;
        if (fragment != nullptr) {
            continuousUpdates = fragment->updateTime() || continuousUpdates;
        }
    }
    return continuousUpdates;
}

data::Timer TimerPagerAdapter::getTimer(int index) {
    return getTimers()[index];
}

void TimerPagerAdapter::setItemVisible(Fragment* item, bool visible) {
    item->setMenuVisibility(visible);
    item->setUserVisibleHint(visible);
}

} // namespace timer
} // namespace deskclock
} // namespace cdroid
