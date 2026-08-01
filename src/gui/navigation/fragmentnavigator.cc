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

void FragmentNavigator::navigate(NavDestination* destination, Bundle* args, NavOptions* navOptions){
    Destination* d = dynamic_cast<Destination*>(destination);
    LOGD("FragmentNavigator.navigate className='%s' fm=%p containerId=%d",
         d ? d->getClassName().c_str() : "(null)", mFragmentManager, mContainerId);
    if(!d || !mFragmentManager){ LOGD("FragmentNavigator.navigate: bail (d=%p fm=%p)", d, mFragmentManager); return; }
    fragment::FragmentFactory factory;
    fragment::Fragment* fragment = factory.instantiate(d->getClassName());
    if(!fragment) return;
    fragment->setArguments(args ? new Bundle(*args) : nullptr);
    fragment::FragmentTransaction* t = mFragmentManager->beginTransaction();
    // Apply custom animations from NavOptions (androidx FragmentNavigator.kt:534-539).
    if(navOptions){
        std::string enter = navOptions->getEnterAnim();
        std::string exit = navOptions->getExitAnim();
        std::string popEnter = navOptions->getPopEnterAnim();
        std::string popExit = navOptions->getPopExitAnim();
        if(!enter.empty() || !exit.empty() || !popEnter.empty() || !popExit.empty()){
            t->setCustomAnimations(enter, exit, popEnter, popExit);
        }
    }
    t->replace(mContainerId, fragment);
    // NOTE: androidx FragmentNavigator.kt:433,447-457 skips addToBackStack on the *initial*
    // navigation (initialNavigation = state.backStack.isEmpty()). CDROID cannot do that yet: its
    // popBackStack pops via FragmentManager.popBackStackImmediate() (an FM-driven model), NOT via
    // the navigator's own entry-state stack like AndroidX. If the start fragment is not on the FM
    // back stack, popUpTo(start, inclusive) cannot FM-pop it, the start fragment stays in the
    // container, and the following navigate()'s replace() then removes a just-restored fragment —
    // a restore→replace cycle that crashes the special-effects transition. So every navigated
    // fragment (including the start) is pushed onto the FM back stack. Porting initialNavigation
    // faithfully requires first porting AndroidX's navigator-state-based pop model.
    t->addToBackStack(d->getRoute());
    t->commit();
}

bool FragmentNavigator::popBackStack(){
    return mFragmentManager ? mFragmentManager->popBackStackImmediate() : false;
}

}//namespace cdroid
