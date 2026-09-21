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
*/
#ifndef __WEAR_SIMPLE_ANIMATOR_LISTENER_H__
#define __WEAR_SIMPLE_ANIMATOR_LISTENER_H__
#include <animation/animator.h>
namespace cdroid{

/** Convenience class for listening for Animator events that implements the AnimatorListener
 *  interface and allows extending only methods that are necessary.
 *  androidx.wear.widget.SimpleAnimatorListener.java (lines 29-67).
 *
 *  CDROID's Animator::AnimatorListener is a value-semantics EventSet (CallbackBase
 *  hooks), not a Java-style interface — the constructor pre-wires those hooks to
 *  dispatch to the virtual methods below, so subclasses override just what they
 *  need, exactly like the upstream adapter. The inherited hooks are public and may
 *  be reassigned directly (lambda style); that replaces the forwarding for that
 *  one event, so prefer subclassing when the cancel/complete bookkeeping matters. */
class SimpleAnimatorListener: public Animator::AnimatorListener {
private:
    bool mWasCanceled;
public:
    SimpleAnimatorListener();

    // These virtuals shadow the identically named EventSet data members of the
    // base (CDROID's AnimatorListener has functions-as-hooks, not interface
    // methods), so they are plain virtuals, not overrides.
    virtual void onAnimationCancel(Animator& animator);
    virtual void onAnimationEnd(Animator& animator);
    virtual void onAnimationRepeat(Animator& animator);
    virtual void onAnimationStart(Animator& animator);

    /** Called when the animation finishes. Not called if the animation was canceled. */
    virtual void onAnimationComplete(Animator& animator);

    /** Provides information if the animation was cancelled.
        @return True if animation was cancelled. */
    bool wasCanceled() const;
};

}/*endof namespace*/
#endif/*__WEAR_SIMPLE_ANIMATOR_LISTENER_H__*/
