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
#include <fragment/fragmentanim.h>
#include <fragment/fragment.h>
#include <animation/alphaanimation.h>
#include <animation/animationutils.h>
#include <core/context.h>
namespace cdroid{
namespace fragment{

Animation* FragmentAnim::loadEnterAnimation(Fragment* /*fragment*/){
    // Legacy default: fade in.
    return new AlphaAnimation(0.0f, 1.0f);
}

Animation* FragmentAnim::loadExitAnimation(Fragment* /*fragment*/){
    // Legacy default: fade out.
    return new AlphaAnimation(1.0f, 0.0f);
}

Animation* FragmentAnim::loadAnimation(Context* context, Fragment* fragment, bool enter, bool isPop){
    std::string anim = getNextAnim(fragment, enter, isPop);
    if(anim.empty()) return nullptr;
    return AnimationUtils::loadAnimation(context, anim);
}

std::string FragmentAnim::getNextAnim(Fragment* fragment, bool enter, bool isPop){
    if(!fragment) return std::string();
    // enter(=ADDING/VISIBLE) + forward -> mEnterAnim;  enter + pop -> mPopEnterAnim;
    // exit(=REMOVING)        + forward -> mExitAnim;   exit  + pop -> mPopExitAnim.
    if(isPop) return enter ? fragment->mPopEnterAnim : fragment->mPopExitAnim;
    return enter ? fragment->mEnterAnim : fragment->mExitAnim;
}

}//namespace fragment
}//namespace cdroid
