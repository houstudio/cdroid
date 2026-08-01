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
#include <navigation/fragmentnavigator.h>
#include <navigation/navigatorstate.h>
#include <fragment/fragment.h>
#include <fragment/fragmentfactory.h>
#include <fragment/fragmenttransaction.h>
#include <fragment/fragmentmanager.h>

namespace cdroid{

FragmentNavigator::FragmentNavigator(fragment::FragmentManager* fm, int containerId)
    : mFragmentManager(fm), mContainerId(containerId){
    mName = "fragment";
}

NavDestination* FragmentNavigator::createDestination(){
    return new Destination(this);
}

void FragmentNavigator::Destination::onInflate(cdroid::Context* context, const AttributeSet& attrs){
    NavDestination::onInflate(context, attrs);
    setClassName(attrs.getString("name"));
    LOGV("FragmentNavigator.Destination.onInflate route='%s' className='%s'",
         getRoute().c_str(), getClassName().c_str());
}

void FragmentNavigator::navigate(std::vector<NavBackStackEntry*>& entries, NavOptions* navOptions, Extras* /*navigatorExtras*/){
    // androidx FragmentNavigator.kt:414-426
    if(!mFragmentManager){ LOGD("FragmentNavigator.navigate: bail (fm null)"); return; }
    for(NavBackStackEntry* entry : entries){
        if(entry) navigate(entry, navOptions);
    }
}

void FragmentNavigator::navigate(NavBackStackEntry* entry, NavOptions* navOptions){
    // androidx FragmentNavigator.kt:428-470
    Destination* d = dynamic_cast<Destination*>(entry->getDestination());
    LOGD("FragmentNavigator.navigate className='%s' initial=%d",
         d ? d->getClassName().c_str() : "(null)", getState() ? (int)getState()->getBackStack().empty() : -1);
    if(!d || !mFragmentManager) return;
    // androidx :433 — initial navigation is when this navigator's back stack is empty (the start
    // fragment). It is committed directly, NOT addToBackStack, so it can never be popped by the
    // FragmentManager to a blank screen; its NavBackStackEntry still leaves the logical back stack
    // through state.pop (its fragment lingers until the next navigate()'s replace evicts it).
    const bool initialNavigation = !isAttached() || getState()->getBackStack().empty();
    fragment::FragmentFactory factory;
    fragment::Fragment* fragment = factory.instantiate(d->getClassName());
    if(!fragment) return;
    fragment->setArguments(entry->getArguments() ? new Bundle(*entry->getArguments()) : nullptr);
    fragment::FragmentTransaction* t = mFragmentManager->beginTransaction();
    // Apply custom animations from NavOptions (androidx createFragmentTransaction :530-539).
    if(navOptions){
        std::string enter = navOptions->getEnterAnim();
        std::string exit = navOptions->getExitAnim();
        std::string popEnter = navOptions->getPopEnterAnim();
        std::string popExit = navOptions->getPopExitAnim();
        if(!enter.empty() || !exit.empty() || !popEnter.empty() || !popExit.empty()){
            t->setCustomAnimations(enter, exit, popEnter, popExit);
        }
    }
    t->replace(mContainerId, fragment, entry->getId());   // androidx :541 — tag = entry.id
    if(!initialNavigation){
        // androidx :447-457 — only non-initial navigations go on the FragmentManager back stack,
        // named with the entry id so popBackStack(entry.id, INCLUSIVE) can target them.
        t->addToBackStack(entry->getId());
    }
    t->commit();
    // androidx :469 — push the entry onto this navigator's state (drives the NavController back
    // queue via the push handler, then this state's own back stack).
    if(getState()) getState()->push(entry);
}

void FragmentNavigator::popBackStack(NavBackStackEntry* popUpTo, bool savedState){
    // androidx FragmentNavigator.kt:317-369. Best-effort FragmentManager pop to (inclusive) the
    // entry: the initial fragment was never addToBackStack, so popBackStackImmediate(name) is a
    // no-op for it (returns false) — that's correct; its fragment is evicted by the next replace.
    if(mFragmentManager){
        mFragmentManager->popBackStackImmediate(popUpTo->getId(), fragment::FragmentManager::POP_BACK_STACK_INCLUSIVE);
    }
    if(getState()) getState()->pop(popUpTo, savedState);
}

bool FragmentNavigator::popBackStack(){
    // Legacy no-arg pop: pop the top of the FragmentManager back stack (false at the initial).
    return mFragmentManager ? mFragmentManager->popBackStackImmediate() : false;
}

}//namespace cdroid
