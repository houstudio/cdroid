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
// Completes synchronously (the transition runs async; the FSM may proceed).
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
