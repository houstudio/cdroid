#ifndef __DESKCLOCK_TIMERPAGERADAPTER_H__
#define __DESKCLOCK_TIMERPAGERADAPTER_H__
/*********************************************************************************
 * Port of com.android.deskclock.timer.TimerPagerAdapter — produces one
 * TimerItemFragment per timer, transaction-managed like FragmentTabPagerAdapter.
 *********************************************************************************/
#include <map>

#include <fragment/fragmentmanager.h>
#include <fragment/fragmenttransaction.h>
#include <widget/adapter.h>

#include <datalisteners.h>
#include <timer.h>
#include <timeritemfragment.h>

namespace cdroid {
namespace deskclock {
namespace timer {

class TimerPagerAdapter : public PagerAdapter {
private:
    /** The manager into which fragments are added. */
    FragmentManager* mFragmentManager;

    /** Maps each timer id to the TimerItemFragment that draws it. */
    std::map<int, TimerItemFragment*> mFragments;

    /** The current fragment transaction in play or null. */
    FragmentTransaction* mCurrentTransaction = nullptr;

    /** The TimerItemFragment that is currently visible on screen. */
    Fragment* mCurrentPrimaryItem = nullptr;

public:
    /** DataModel timer notifications (registered by TimerFragment, upstream: the adapter
     *  itself is a TimerListener). */
    data::TimerListener mTimerListener;

    explicit TimerPagerAdapter(FragmentManager* fragmentManager);
    ~TimerPagerAdapter() override;

    int getCount() override;
    bool isViewFromObject(View* view, void* object) override;
    int getItemPosition(void* object) override;
    void* instantiateItem(ViewGroup* container, int position) override;
    void destroyItem(ViewGroup* container, int position, void* object) override;
    void setPrimaryItem(ViewGroup* container, int position, void* object) override;
    void finishUpdate(ViewGroup* container) override;

    /**
     * @return true if at least one timer is in a state requiring continuous updates
     */
    bool updateTime();

    data::Timer getTimer(int index);

    const std::vector<data::Timer>& getTimers() const;

private:
    static void setItemVisible(Fragment* item, bool visible);
};

} // namespace timer
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_TIMERPAGERADAPTER_H__
