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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *********************************************************************************/
#ifndef __CDROID_TRANSITION_AUTOTRANSITION_H__
#define __CDROID_TRANSITION_AUTOTRANSITION_H__

#include <core/attributeset.h>
#include <core/context.h>

#include <transition/changebounds.h>
#include <transition/fade.h>
#include <transition/transitionset.h>

namespace cdroid{

/**
 * Utility class for a default transition that automatically fades, moves, and resizes
 * views during a scene change. Ported from android-36 android.transition.AutoTransition.
 *
 * A TransitionSet that plays sequentially: Fade(OUT) → ChangeBounds → Fade(IN).
 * clone() is inherited from TransitionSet (AutoTransition adds no fields; the sliced copy
 * preserves the children + ordering, behaving identically).
 */
class AutoTransition: public TransitionSet{
public:
    AutoTransition(){ init(); }
    AutoTransition(Context* context, AttributeSet* attrs): TransitionSet(context, attrs){ init(); }

private:
    void init(){
        setOrdering(ORDERING_SEQUENTIAL);
        addTransition(new Fade(Fade::OUT));
        addTransition(new ChangeBounds());
        addTransition(new Fade(Fade::IN));
    }
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_AUTOTRANSITION_H__
