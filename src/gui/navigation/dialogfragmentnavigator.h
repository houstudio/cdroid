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
#ifndef __DIALOGFRAGMENTNAVIGATOR_H__
#define __DIALOGFRAGMENTNAVIGATOR_H__
/*********************************************************************************
 * Port of androidx.navigation.fragment.DialogFragmentNavigator. A Navigator that
 * shows DialogFragments — each destination's className is a DialogFragment shown via
 * DialogFragment.show(fragmentManager, entry.id) on navigate, dismissed on popBackStack.
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
namespace fragment{ class FragmentManager; class DialogFragment; }
class Context;

class DialogFragmentNavigator : public Navigator{
public:
    class Destination : public NavDestination{
    public:
        explicit Destination(DialogFragmentNavigator* owner) : NavDestination((Navigator*)owner){}
        void onInflate(cdroid::Context* context, const AttributeSet& attrs) override;
        void setClassName(const std::string& cls){ mClassName = cls; }
        const std::string& getClassName() const { return mClassName; }
    private:
        std::string mClassName;
    };

    DialogFragmentNavigator(Context* context, fragment::FragmentManager* fm);
    NavDestination* createDestination() override;
    // androidx DialogFragmentNavigator.navigate(entries, navOptions, navigatorExtras).
    void navigate(std::vector<NavBackStackEntry*>& entries, NavOptions* navOptions, Extras* navigatorExtras) override;
    // androidx DialogFragmentNavigator.popBackStack(popUpTo, savedState): dismiss the dialog
    // by finding the fragment by entry id, then pop the entry off this navigator's state.
    void popBackStack(NavBackStackEntry* popUpTo, bool savedState) override;
private:
    Context* mContext;
    fragment::FragmentManager* mFragmentManager;
    // androidx DialogFragmentNavigator.navigate(entry) — per-entry core.
    void navigate(NavBackStackEntry* entry, NavOptions* navOptions);
};

}//namespace cdroid
#endif
