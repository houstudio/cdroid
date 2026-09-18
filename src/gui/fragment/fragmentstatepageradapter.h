/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __FRAGMENT_STATE_PAGER_ADAPTER_H__
#define __FRAGMENT_STATE_PAGER_ADAPTER_H__

#include <widget/adapter.h>
#include <fragment/fragment.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenttransaction.h>
#include <vector>

/* CODE AND Resource Referenced:
 * https://github.com/androidx/androidx/blob/androidx-main/fragment/fragment/src/main/java/androidx/fragment/app/FragmentStatePagerAdapter.java
 * */
namespace cdroid{

/**
 * Implementation of {@link PagerAdapter} that uses a {@link Fragment} to manage each page.
 * This class also handles saving and restoring of fragment's state.
 *
 * <p>This version of the pager is more useful when there are a large number
 * of pages, working more like a list view.  When pages are not visible to
 * the user, their entire fragment may be destroyed, only keeping the saved
 * state of that page.  This differs from the {@link FragmentPagerAdapter},
 * which keeps all fragments in memory.
 *
 * @deprecated Use {@link viewpager2.widget.ViewPager2} instead.
 */
class FragmentStatePagerAdapter:public PagerAdapter{
public:
    /** @deprecated Use {@link #BEHAVIOR_RESUME_ONLY_CURRENT_FRAGMENT} instead. */
    static constexpr int BEHAVIOR_SET_USER_VISIBLE_HINT = 0;
    static constexpr int BEHAVIOR_RESUME_ONLY_CURRENT_FRAGMENT = 1;
private:
    FragmentManager* mFragmentManager;
    int mBehavior;
    FragmentTransaction* mCurTransaction;
    std::vector<Fragment::SavedState*> mSavedState;
    std::vector<Fragment*> mFragments;
    Fragment* mCurrentPrimaryItem;
    bool mExecutingFinishUpdate;
public:
    /** @deprecated use {@link #FragmentStatePagerAdapter(FragmentManager, int)} instead. */
    FragmentStatePagerAdapter(FragmentManager* fm);
    FragmentStatePagerAdapter(FragmentManager* fm, int behavior);
    ~FragmentStatePagerAdapter()override;

    virtual Fragment* getItem(int position) = 0;

    void startUpdate(ViewGroup* container)override;
    void* instantiateItem(ViewGroup* container, int position)override;
    void destroyItem(ViewGroup* container, int position, void* object)override;
    void setPrimaryItem(ViewGroup* container, int position, void* object)override;
    void finishUpdate(ViewGroup* container)override;
    bool isViewFromObject(View* view, void* object)override;
    Parcelable* saveState()override;
    void restoreState(Parcelable* state)override;
};

}/*endof namespace cdroid*/
#endif/*__FRAGMENT_STATE_PAGER_ADAPTER_H__*/
