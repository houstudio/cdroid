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
#include <fragment/fragmentpageradapter.h>
#include <fragment/fragment.h>
#include <lifecycle/lifecycle.h>
#include <porting/cdlog.h>

namespace cdroid{
namespace fragment{

FragmentPagerAdapter::FragmentPagerAdapter(FragmentManager* fm)
    : FragmentPagerAdapter(fm, BEHAVIOR_SET_USER_VISIBLE_HINT){
}

FragmentPagerAdapter::FragmentPagerAdapter(FragmentManager* fm, int behavior){
    mFragmentManager = fm;
    mBehavior = behavior;
    mCurTransaction = nullptr;
    mCurrentPrimaryItem = nullptr;
    mExecutingFinishUpdate = false;
}

FragmentPagerAdapter::~FragmentPagerAdapter(){
    // FragmentManager owns committed transactions; a never-committed one is ours.
    delete mCurTransaction;
}

void FragmentPagerAdapter::startUpdate(ViewGroup* container){
    if (container->getId() == View::NO_ID) {
        throw std::runtime_error("ViewPager with adapter requires a view id");
    }
}

void* FragmentPagerAdapter::instantiateItem(ViewGroup* container, int position){
    if (mCurTransaction == nullptr) {
        mCurTransaction = mFragmentManager->beginTransaction();
    }

    const long itemId = getItemId(position);
    const std::string name = makeFragmentName(container->getId(), itemId);

    Fragment* fragment = mFragmentManager->findFragmentByTag(name);
    if (fragment != nullptr) {
        LOGV("Attaching item #%ld: f=%p",itemId,fragment);
        mCurTransaction->attach(fragment);
    } else {
        fragment = getItem(position);
        LOGV("Adding item #%ld: f=%p",itemId,fragment);
        mCurTransaction->add(container->getId(), fragment,
                makeFragmentName(container->getId(), itemId));
    }

    if (fragment != mCurrentPrimaryItem) {
        fragment->setMenuVisibility(false);
        if (mBehavior == BEHAVIOR_RESUME_ONLY_CURRENT_FRAGMENT) {
            mCurTransaction->setMaxLifecycle(fragment, lifecycle::Lifecycle::State::STARTED);
        } else {
            // DEPRECATED BEHAVIOR_SET_USER_VISIBLE_HINT: Fragment.setUserVisibleHint is
            // not ported (superseded by setMaxLifecycle in androidx); the flag-only
            // setter on Fragment covers the call.
            fragment->setUserVisibleHint(false);
        }
    }

    return fragment;
}

void FragmentPagerAdapter::destroyItem(ViewGroup* container, int position, void* object){
    Fragment* fragment = (Fragment*) object;
    if (mCurTransaction == nullptr) {
        mCurTransaction = mFragmentManager->beginTransaction();
    }
    LOGV("Detaching item #%ld: f=%p", getItemId(position), fragment);
    mCurTransaction->detach(fragment);
    if (fragment == mCurrentPrimaryItem) {
        mCurrentPrimaryItem = nullptr;
    }
}

void FragmentPagerAdapter::setPrimaryItem(ViewGroup* container, int position, void* object){
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

void FragmentPagerAdapter::finishUpdate(ViewGroup* container){
    if (mCurTransaction != nullptr) {
        if (!mExecutingFinishUpdate) {
            mExecutingFinishUpdate = true;
            mCurTransaction->commitNowAllowingStateLoss();
            mExecutingFinishUpdate = false;
        }
        mCurTransaction = nullptr;
    }
}

bool FragmentPagerAdapter::isViewFromObject(View* view, void* object){
    return ((Fragment*) object)->getView() == view;
}

Parcelable* FragmentPagerAdapter::saveState(){
    return nullptr;
}

void FragmentPagerAdapter::restoreState(Parcelable* state){
}

long FragmentPagerAdapter::getItemId(int position){
    return position;
}

std::string FragmentPagerAdapter::makeFragmentName(int viewId, long id){
    return "android:switcher:" + std::to_string(viewId) + ":" + std::to_string(id);
}

}/*endof namespace fragment*/
}/*endof namespace cdroid*/
