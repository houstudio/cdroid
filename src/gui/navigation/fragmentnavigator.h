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
#ifndef __FRAGMENTNAVIGATOR_H__
#define __FRAGMENTNAVIGATOR_H__
/*********************************************************************************
 * Port of androidx.navigation.fragment.FragmentNavigator. A Navigator that swaps
 * Fragments via FragmentTransaction.replace for each destination's className.
 *********************************************************************************/
#include <string>
#include <vector>
#include <set>
#include <navigation/navigator.h>
#include <navigation/navdestination.h>
#include <navigation/navoptions.h>
#include <navigation/navbackstackentry.h>
#include <core/bundle.h>
#include <core/attributeset.h>
#include <porting/cdlog.h>
namespace cdroid{
namespace fragment{ class FragmentManager; }
class FragmentNavigator : public Navigator{
public:
    class Destination : public NavDestination{
    public:
        explicit Destination(FragmentNavigator* owner) : NavDestination((Navigator*)owner){}
        // Reads the Fragment class name from XML android:name.
        void onInflate(cdroid::Context* context, const AttributeSet& attrs) override;
        void setClassName(const std::string& cls){ mClassName = cls; }
        const std::string& getClassName() const { return mClassName; }
    private:
        std::string mClassName;
    };

    FragmentNavigator(fragment::FragmentManager* fm, int containerId);
    NavDestination* createDestination() override;
    // androidx FragmentNavigator.navigate(entries, navOptions, navigatorExtras): swap a Fragment
    // in for each entry via FragmentTransaction.replace, skipping addToBackStack on the initial
    // navigation (state.backStack was empty), then push each entry onto this navigator's state.
    void navigate(std::vector<NavBackStackEntry*>& entries, NavOptions* navOptions, Extras* navigatorExtras) override;
    // androidx FragmentNavigator.popBackStack(popUpTo, savedState): pop the FragmentManager back
    // stack to (inclusive) popUpTo, then pop the entry off this navigator's state.
    void popBackStack(NavBackStackEntry* popUpTo, bool savedState) override;
    // Legacy no-arg pop (kept until NavController switches fully to the entry-based pop): pop the
    // top of the FragmentManager back stack. Returns false at the start fragment (initial).
    bool popBackStack() override;
    // androidx FragmentNavigator.onSaveState/onRestoreState: persist the savedIds set (which entry
    // ids are eligible for restoreBackStack).
    Bundle onSaveState() override;
    void onRestoreState(const Bundle& savedState) override;
private:
    fragment::FragmentManager* mFragmentManager;
    int mContainerId;
    // androidx FragmentNavigator.savedIds: entry ids saved via saveBackStack and thus eligible for
    // restoreBackStack (gates the navigate restoreState path).
    std::set<std::string> mSavedIds;
    // androidx FragmentNavigator.kt:428-470 — per-entry navigate core.
    void navigate(NavBackStackEntry* entry, NavOptions* navOptions);
};
}//namespace cdroid
#endif
