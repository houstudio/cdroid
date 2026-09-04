#ifndef __DESKCLOCK_FRAGMENTTABPAGERADAPTER_H__
#define __DESKCLOCK_FRAGMENTTABPAGERADAPTER_H__
/*********************************************************************************
 * Port of com.android.deskclock.FragmentTabPagerAdapter — produces the
 * DeskClockFragments that are the tab content; fragments are registered with
 * the manager using position-independent tags (per-tab name).
 *********************************************************************************/
#include <map>
#include <string>

#include <fragment/fragmentmanager.h>
#include <fragment/fragmenttransaction.h>
#include <widget/adapter.h>

#include <deskclockfragment.h>
#include <uidata.h>

namespace cdroid {
namespace deskclock {

class DeskClock;

class FragmentTabPagerAdapter : public PagerAdapter {
private:
    DeskClock& mDeskClock;

    /** The manager into which fragments are added. */
    FragmentManager* mFragmentManager;

    /** A fragment cache that can be accessed before instantiateItem is called. */
    std::map<int /*tab*/, DeskClockFragment*> mFragmentCache;

    /** The active fragment transaction if one exists. */
    FragmentTransaction* mCurrentTransaction = nullptr;

    /** The current fragment displayed to the user. */
    Fragment* mCurrentPrimaryItem = nullptr;

public:
    explicit FragmentTabPagerAdapter(DeskClock& deskClock);
    ~FragmentTabPagerAdapter() override;

    int getCount() override;

    /** @return the fragment displayed at the given left-to-right position. */
    DeskClockFragment* getDeskClockFragment(int position);

    void startUpdate(ViewGroup* container) override;
    void* instantiateItem(ViewGroup* container, int position) override;
    void destroyItem(ViewGroup* container, int position, void* object) override;
    void setPrimaryItem(ViewGroup* container, int position, void* object) override;
    void finishUpdate(ViewGroup* container) override;
    bool isViewFromObject(View* view, void* object) override;
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_FRAGMENTTABPAGERADAPTER_H__
