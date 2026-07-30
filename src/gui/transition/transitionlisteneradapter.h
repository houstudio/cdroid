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
#ifndef __CDROID_TRANSITION_TRANSITIONLISTENERADAPTER_H__
#define __CDROID_TRANSITION_TRANSITIONLISTENERADAPTER_H__

#include <transition/transition.h>

namespace cdroid {

/**
 * This adapter class provides empty implementations of the methods from
 * Transition::TransitionListener. A custom listener that cares only about a subset
 * of the methods can subclass this adapter instead of implementing the interface.
 *
 * Ported from android-36 android.transition.TransitionListenerAdapter.
 */
class TransitionListenerAdapter: public Transition::TransitionListener {
  public:
    void onTransitionStart(Transition& transition) override {
        (void)transition;
    }
    void onTransitionEnd(Transition& transition) override {
        (void)transition;
    }
    void onTransitionCancel(Transition& transition) override {
        (void)transition;
    }
    void onTransitionPause(Transition& transition) override {
        (void)transition;
    }
    void onTransitionResume(Transition& transition) override {
        (void)transition;
    }
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_TRANSITIONLISTENERADAPTER_H__
