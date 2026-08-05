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
#include <navigation/dialogfragmentnavigator.h>
#include <navigation/navigatorstate.h>
#include <fragment/fragment.h>
#include <fragment/fragmentfactory.h>
#include <fragment/fragmentmanager.h>
#include <fragment/dialogfragment.h>

namespace cdroid{

void DialogFragmentNavigator::Destination::onInflate(cdroid::Context* context, const AttributeSet& attrs){
    NavDestination::onInflate(context, attrs);
    setClassName(attrs.getString("name"));
    LOGV("DialogFragmentNavigator.Destination.onInflate route='%s' className='%s'",
         getRoute().c_str(), getClassName().c_str());
}

DialogFragmentNavigator::DialogFragmentNavigator(Context* context, fragment::FragmentManager* fm)
    : mContext(context), mFragmentManager(fm){
    mName = "dialog";
}

NavDestination* DialogFragmentNavigator::createDestination(){
    return new Destination(this);
}

void DialogFragmentNavigator::navigate(std::vector<NavBackStackEntry*>& entries, NavOptions* navOptions, Extras* /*navigatorExtras*/){
    // androidx DialogFragmentNavigator.kt:157-169
    if(!mFragmentManager){ LOGD("DialogFragmentNavigator.navigate: bail (fm null)"); return; }
    for(NavBackStackEntry* entry : entries){
        if(entry) navigate(entry, navOptions);
    }
}

void DialogFragmentNavigator::navigate(NavBackStackEntry* entry, NavOptions* /*navOptions*/){
    // androidx DialogFragmentNavigator.kt:171-181 — createDialogFragment + show + state.push.
    Destination* d = dynamic_cast<Destination*>(entry->getDestination());
    if(!d || !mFragmentManager) return;
    fragment::FragmentFactory factory;
    fragment::Fragment* f = factory.instantiate(d->getClassName());
    if(!f) return;
    auto* dialogFragment = dynamic_cast<fragment::DialogFragment*>(f);
    if(!dialogFragment){ LOGE("DialogFragmentNavigator: %s is not a DialogFragment", d->getClassName().c_str()); delete f; return; }
    dialogFragment->setArguments(entry->getArguments() ? new Bundle(*entry->getArguments()) : nullptr);
    // androidx :173 — show(fm, entry.id) adds the dialog fragment tagged with the entry id.
    dialogFragment->show(mFragmentManager, entry->getId());
    // androidx :176 — push the entry onto this navigator's state (drives NavController back queue).
    if(getState()) getState()->push(entry);
}

void DialogFragmentNavigator::popBackStack(NavBackStackEntry* popUpTo, bool savedState){
    // androidx DialogFragmentNavigator.kt:115-135 — dismiss the dialog by finding the fragment
    // by entry id (tag), then pop the entry off this navigator's state.
    if(mFragmentManager){
        fragment::Fragment* f = mFragmentManager->findFragmentByTag(popUpTo->getId());
        if(f){
            auto* df = dynamic_cast<fragment::DialogFragment*>(f);
            if(df) df->dismiss();
        }
    }
    if(getState()) getState()->pop(popUpTo, savedState);
}

}//namespace cdroid
