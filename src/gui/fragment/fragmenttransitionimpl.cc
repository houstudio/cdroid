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
#include <fragment/fragmenttransitionimpl.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <transition/fade.h>
#include <transition/changebounds.h>
#include <transition/transitionset.h>

namespace cdroid{
namespace fragment{

Transition* FragmentTransitionImpl::makeEnterTransition(const SharedElementMapping& sharedElements){
    if(!sharedElements.empty()){
        // Shared-element transition: animate shared views' bounds/transform from source
        // (exiting fragment) to target (entering fragment).
        TransitionSet* set = new TransitionSet();
        ChangeBounds* changeBounds = new ChangeBounds();
        for(const auto& kv : sharedElements){
            if(kv.second) changeBounds->addTarget(kv.second);
        }
        set->addTransition(changeBounds);
        set->addTransition(new Fade());
        return set;
    }
    return new Fade();
}

Transition* FragmentTransitionImpl::makeExitTransition(const SharedElementMapping& sharedElements){
    (void)sharedElements;
    // Exit usually uses a Fade (shared elements animate in the enter side).
    return new Fade();
}

}//namespace fragment
}//namespace cdroid
