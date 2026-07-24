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
#ifndef __CDROID_TRANSITION_TRANSITIONVALUESMAPS_H__
#define __CDROID_TRANSITION_TRANSITIONVALUESMAPS_H__

#include <string>

#include <core/sparsearray.h> // SparseArray, LongSparseArray
#include <view/view.h>

#include <transition/arraymap.h>
#include <transition/transitionvalues.h>

namespace cdroid{

/**
 * Holds captured start/end values indexed four ways. Ported (package-private
 * data class) from android-36 android.transition.TransitionValuesMaps.
 *
 *  - viewValues   : by View identity      (android: ArrayMap<View, TransitionValues>)
 *  - idValues     : by View#getId()       (android: SparseArray<View>)
 *  - itemIdValues : by adapter item id    (android: LongSparseArray<View>)
 *  - nameValues   : by View#getTransitionName() (android: ArrayMap<String, View>)
 *
 * viewValues stores shared_ptr<TransitionValues> so the same object is shared
 * with Transition's mStartValuesList/mEndValuesList (java reference identity).
 */
struct TransitionValuesMaps{
    ArrayMap<View*, TransitionValuesPtr> viewValues;
    SparseArray<View*>    idValues;
    LongSparseArray<View*> itemIdValues;
    ArrayMap<std::string, View*> nameValues;
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_TRANSITIONVALUESMAPS_H__
