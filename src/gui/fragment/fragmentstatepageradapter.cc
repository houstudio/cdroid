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
#include <fragment/fragmentstatepageradapter.h>
#include <fragment/fragment.h>
#include <lifecycle/lifecycle.h>
#include <porting/cdlog.h>

namespace cdroid{
namespace fragment{

FragmentStatePagerAdapter::FragmentStatePagerAdapter(FragmentManager* fm)
    : FragmentStatePagerAdapter(fm, BEHAVIOR_SET_USER_VISIBLE_HINT){
}

FragmentStatePagerAdapter::FragmentStatePagerAdapter(FragmentManager* fm, int behavior){
    mFragmentManager = fm;
    mBehavior = behavior;
    mCurTransaction = nullptr;
    mCurrentPrimaryItem = nullptr;
    mExecutingFinishUpdate = false;
}

FragmentStatePagerAdapter::~FragmentStatePagerAdapter(){
    // FragmentManager owns committed transactions; a never-committed one is ours.
    delete mCurTransaction;
    for (auto ss : mSavedState) delete ss;
}

void FragmentStatePagerAdapter::startUpdate(ViewGroup* container){
    if (container->getId() == View::NO_ID) {
        throw std::runtime_error("ViewPager with adapter requires a view id");
    }
}

void* FragmentStatePagerAdapter::instantiateItem(ViewGroup* container, int position){
    if (position < (int) mFragments.size()) {
        Fragment* f = mFragments[position];
        if (f != nullptr) {
            return f;
        }
    }

    if (mCurTransaction == nullptr) {
        mCurTransaction = mFragmentManager->beginTransaction();
    }

    Fragment* fragment = getItem(position);
    LOGV("Adding item #%d: f=%p",position,fragment);
    if ((int) mSavedState.size() > position) {
        Fragment::SavedState* fss = mSavedState[position];
        if (fss != nullptr) {
            fragment->setInitialSavedState(fss);
        }
    }
    while ((int) mFragments.size() <= position) {
        mFragments.push_back(nullptr);
    }
    fragment->setMenuVisibility(false);
    if (mBehavior == BEHAVIOR_SET_USER_VISIBLE_HINT) {
        // DEPRECATED: flag-only port of Fragment.setUserVisibleHint.
        fragment->setUserVisibleHint(false);
    }
    mFragments[position] = fragment;
    mCurTransaction->add(container->getId(), fragment);
    if (mBehavior == BEHAVIOR_RESUME_ONLY_CURRENT_FRAGMENT) {
        mCurTransaction->setMaxLifecycle(fragment, lifecycle::Lifecycle::State::STARTED);
    }
    return fragment;
}

void FragmentStatePagerAdapter::destroyItem(ViewGroup* container, int position, void* object){
    Fragment* fragment = (Fragment*) object;
    if (mCurTransaction == nullptr) {
        mCurTransaction = mFragmentManager->beginTransaction();
    }
    LOGV("Removing item #%d: f=%p",position,fragment);
    while ((int) mSavedState.size() <= position) {
        mSavedState.push_back(nullptr);
    }
    delete mSavedState[position];
    mSavedState[position] = fragment->isAdded()
            ? mFragmentManager->saveFragmentInstanceState(fragment) : nullptr;
    mFragments[position] = nullptr;
    mCurTransaction->remove(fragment);
    if (fragment == mCurrentPrimaryItem) {
        mCurrentPrimaryItem = nullptr;
    }
}

void FragmentStatePagerAdapter::setPrimaryItem(ViewGroup* container, int position, void* object){
    Fragment* fragment = (Fragment*) object;
    if (fragment != mCurrentPrimaryItem) {
        if (mCurrentPrimaryItem != nullptr) {
            mCurrentPrimaryItem->setMenuVisibility(false);
            if (mBehavior == BEHAVIOR_RESUME_ONLY_CURRENT_FRAGMENT) {
                if (mCurTransaction == nullptr) {
                    mCurTransaction = mFragmentManager->beginTransaction();
                }
                mCurTransaction->setMaxLifecycle(mCurrentPrimaryItem,
                        lifecycle::Lifecycle::State::STARTED);
            } else {
                mCurrentPrimaryItem->setUserVisibleHint(false);
            }
        }
        fragment->setMenuVisibility(true);
        if (mBehavior == BEHAVIOR_RESUME_ONLY_CURRENT_FRAGMENT) {
            if (mCurTransaction == nullptr) {
                mCurTransaction = mFragmentManager->beginTransaction();
            }
            mCurTransaction->setMaxLifecycle(fragment, lifecycle::Lifecycle::State::RESUMED);
        } else {
            fragment->setUserVisibleHint(true);
        }
        mCurrentPrimaryItem = fragment;
    }
}

void FragmentStatePagerAdapter::finishUpdate(ViewGroup* container){
    if (mCurTransaction != nullptr) {
        // We drop any transactions that attempt to be committed
        // from a re-entrant call to finishUpdate(). We need to
        // do this as a workaround for Robolectric running measure/layout
        // calls inline rather than allowing them to be posted
        // as they would on a real device.
        if (!mExecutingFinishUpdate) {
            mExecutingFinishUpdate = true;
            mCurTransaction->commitNowAllowingStateLoss();
            mExecutingFinishUpdate = false;
        }
        mCurTransaction = nullptr;
    }
}

bool FragmentStatePagerAdapter::isViewFromObject(View* view, void* object){
    return ((Fragment*) object)->getView() == view;
}

Parcelable* FragmentStatePagerAdapter::saveState(){
    // androidx serializes mSavedState + added fragments into a Bundle (putParcelableArray
    // + putFragment). CDROID's Bundle has no parcel-array round-trip; the in-session
    // destroy/recreate path keeps mSavedState alive in memory, and cross-recreation
    // persistence goes through the FragmentManager's own state chain instead.
    return nullptr;
}

void FragmentStatePagerAdapter::restoreState(Parcelable* state){
}

}/*endof namespace fragment*/
}/*endof namespace cdroid*/
