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
#ifndef __CDROID_TRANSITION_TRANSITIONLISTENERADAPTER_H__
#define __CDROID_TRANSITION_TRANSITIONLISTENERADAPTER_H__

#include <transition/transition.h>

namespace cdroid {

/**
 * Transition::TransitionListener is now an EventSet value type whose callback
 * members (onTransitionStart/End/...) default to no-ops, so there is nothing to
 * adapt — a listener is filled in directly. This alias is kept for source
 * compatibility with any code still naming TransitionListenerAdapter.
 *
 * Ported from android-36 android.transition.TransitionListenerAdapter.
 */
using TransitionListenerAdapter = Transition::TransitionListener;

} // namespace cdroid
#endif // __CDROID_TRANSITION_TRANSITIONLISTENERADAPTER_H__
