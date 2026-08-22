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
#ifndef __FRAGMENT_PAGER_ADAPTER_H__
#define __FRAGMENT_PAGER_ADAPTER_H__

#include <widget/adapter.h>
#include <fragment/fragment.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenttransaction.h>

/* CODE AND Resource Referenced:
 * https://github.com/androidx/androidx/blob/androidx-main/fragment/fragment/src/main/java/androidx/fragment/app/FragmentPagerAdapter.java
 * */
namespace cdroid{
namespace fragment{

/**
 * Implementation of {@link PagerAdapter} that represents each page as a {@link Fragment}
 * that is kept in the fragment manager as long as the user can return to the page.
 *
 * <p>This version of the pager is best for use when there are a handful of
 * typically more static fragments to be paged through, such as a set of tabs.
 * The fragment of each page the user visits will be kept in memory, though its
 * view hierarchy may be destroyed when not visible.  This can result in using
 * a significant amount of memory since fragment instances can hold on to an
 * arbitrary amount of state.  For larger sets of pages, consider
 * {@link FragmentStatePagerAdapter}.
 *
 * @deprecated Use {@link viewpager2.widget.ViewPager2} instead.
 */
class FragmentPagerAdapter:public PagerAdapter{
public:
    /** @deprecated Use {@link #BEHAVIOR_RESUME_ONLY_CURRENT_FRAGMENT} instead. */
    static constexpr int BEHAVIOR_SET_USER_VISIBLE_HINT = 0;
    static constexpr int BEHAVIOR_RESUME_ONLY_CURRENT_FRAGMENT = 1;
private:
    FragmentManager* mFragmentManager;
    int mBehavior;
    FragmentTransaction* mCurTransaction;
    Fragment* mCurrentPrimaryItem;
    bool mExecutingFinishUpdate;
public:
    /** @deprecated use {@link #FragmentPagerAdapter(FragmentManager, int)} instead. */
    FragmentPagerAdapter(FragmentManager* fm);
    FragmentPagerAdapter(FragmentManager* fm, int behavior);
    ~FragmentPagerAdapter()override;

    virtual Fragment* getItem(int position) = 0;

    void startUpdate(ViewGroup* container)override;
    void* instantiateItem(ViewGroup* container, int position)override;
    void destroyItem(ViewGroup* container, int position, void* object)override;
    void setPrimaryItem(ViewGroup* container, int position, void* object)override;
    void finishUpdate(ViewGroup* container)override;
    bool isViewFromObject(View* view, void* object)override;
    Parcelable* saveState()override;
    void restoreState(Parcelable* state)override;

    virtual long getItemId(int position);
    static std::string makeFragmentName(int viewId, long id);
};

}/*endof namespace fragment*/
}/*endof namespace cdroid*/
#endif/*__FRAGMENT_PAGER_ADAPTER_H__*/
