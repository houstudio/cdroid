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
#ifndef __DEFAULTSPECIALEFFECTSCONTROLLER_H__
#define __DEFAULTSPECIALEFFECTSCONTROLLER_H__
/*********************************************************************************
 * Port of androidx DefaultSpecialEffectsController (simplified). Picks an Animation effect
 * (custom anim) or a Transition effect (default Fade/shared) per Operation, then the base
 * controller commits them. Drops Animator + seek.
 *********************************************************************************/
#include <fragment/specialeffectscontroller.h>
#include <animation/animation.h>
#include <transition/transition.h>
namespace cdroid{
namespace fragment{

class DefaultSpecialEffectsController : public SpecialEffectsController{
public:
    using SpecialEffectsController::SpecialEffectsController;
    void collectEffects(std::vector<Operation*>& operations, bool isPop) override;
};

// Runs a custom Animation via View.startAnimation. ADDING/show completes synchronously (no
// reliable end listener); REMOVED completes on onAnimationEnd (deferred via post so we don't
// delete the Animation while it's still in its own callback).
class AnimationEffect : public SpecialEffectsController::Effect{
public:
    AnimationEffect(SpecialEffectsController::Operation* op, Animation* animation) : Effect(op), mAnimation(animation){}
    ~AnimationEffect() override{ delete mAnimation; }
    void onCommit(ViewGroup* container) override;
private:
    Animation* mAnimation;
};

// Runs a Transition via TransitionManager.beginDelayedTransition (default Fade / shared ChangeBounds).
// Completes on transition end/cancel (the clone runs async; completeEffect is deferred via a cloned
// listener so the view isn't reclaimed while the transition's startValues still references it).
class TransitionEffect : public SpecialEffectsController::Effect{
public:
    TransitionEffect(SpecialEffectsController::Operation* op, Transition* transition) : Effect(op), mTransition(transition){}
    ~TransitionEffect() override{ delete mTransition; }
    void onCommit(ViewGroup* container) override;
private:
    Transition* mTransition;
};

}}//namespace fragment::cdroid
#endif/*__DEFAULTSPECIALEFFECTSCONTROLLER_H__*/
