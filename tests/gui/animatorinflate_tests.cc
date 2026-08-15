#include <gtest/gtest.h>
#include <core/app.h>
#include <porting/cdlog.h>
#include <animation/objectanimator.h>
#include <animation/animatorinflater.h>
#include <animation/animationutils.h>
#include <animation/animatorset.h>
#include <animation/statelistanimator.h>
#include <view/view.h>
#include <guienvironment.h>
#include "R.h"
using namespace cdroid;
#include <guienvironment.h>
class ANIMATORINFLATOR:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

// The old test1/test2/"test2.xml" cases loaded non-existent resources (the
// assertions were vacuous). Replaced by a real state-list asset in this pak.
TEST_F(ANIMATORINFLATOR,statelistanimator){
    App&app=App::getInstance();
    StateListAnimator*sl=AnimatorInflater::loadStateListAnimator(&app,
            gui_test::R::animator::statelist_test);
    ASSERT_NE(sl,(void*)nullptr);
    pumpFor(300);
}

// Deep replication of kaidu_ms7 home_tab_bg_anim.xml: selector -> item(state) ->
// set -> multiple objectAnimators (integer valueFrom, pivotY, floats). Asserts the
// full chain loads under binary AXML: SLA parses, the state matches, an AnimatorSet
// with 4 children actually starts on a real View's state change.
TEST_F(ANIMATORINFLATOR,statelistanimator_scale_runs){
    App&app=App::getInstance();
    StateListAnimator*sl=AnimatorInflater::loadStateListAnimator(&app,
            gui_test::R::animator::statelist_scale);
    ASSERT_NE(sl,(void*)nullptr);

    View*v=new View(100,100);
    v->setStateListAnimator(sl); // takes ownership, sets target
    ASSERT_EQ(v->getStateListAnimator(),(StateListAnimator*)sl);
    // AOSP drawableStateChanged: !isAggregatedVisible() jumpDrawablesToCurrent-
    // State() ends (and clears) the running animator, so the view must be
    // attached/visible for the state change to leave an animator running.
    GUIEnvironment::content()->addView(v);

    // Base state (item without specs) should already have matched on attach-less
    // setState via refreshDrawableState once we poke a state change.
    v->setActivated(true);
    // setState/start run synchronously inside setActivated — grab the running
    // animator before pumping (a pumped frame can block long enough for a
    // short animator to end and clear mRunningAnimator).
    Animator*running=v->getStateListAnimator()->getRunningAnimator();
    ASSERT_NE(running,(void*)nullptr) << "no animator started on state_activated";
    auto*set=dynamic_cast<AnimatorSet*>(running);
    ASSERT_NE(set,(AnimatorSet*)nullptr) << "expected <set> payload";
    const auto children=set->getChildAnimations();
    ASSERT_EQ(children.size(),(size_t)4) << "expected 4 objectAnimators";
    ASSERT_TRUE(set->isRunning()) << "animator set not running";
    // Durations come through the binary AXML typed values (500 for activated item).
    ASSERT_EQ(set->getTotalDuration(),(int64_t)500) << "duration not read from AXML";
    pumpFor(600);

    v->setActivated(false); // back to base state -> 300ms animators
    running=v->getStateListAnimator()->getRunningAnimator();
    ASSERT_NE(running,(void*)nullptr);
    set=dynamic_cast<AnimatorSet*>(running);
    ASSERT_NE(set,(AnimatorSet*)nullptr);
    ASSERT_EQ(set->getChildAnimations().size(),(size_t)4);
    ASSERT_EQ(set->getTotalDuration(),(int64_t)300);
    pumpFor(400);
    delete v; // deletes the SLA it owns
}

TEST_F(ANIMATORINFLATOR,fade_in){
    App&app=App::getInstance();
    Animator*sl=AnimatorInflater::loadAnimator(&app,gui_test::R::animator::fade_in);
    ASSERT_NE(sl,(void*)nullptr);
    pumpFor(300);
}

// Legacy tween <translate> animations live in anim/ and load through
// AnimationUtils (Animation), not AnimatorInflater — on Android the latter
// throws "Unknown animator name: translate" for this resource.
TEST_F(ANIMATORINFLATOR,slide_in_left){
    App&app=App::getInstance();
    Animation*sl=AnimationUtils::loadAnimation(&app,gui_test::R::anim::slide_in_left);
    ASSERT_NE(sl,(void*)nullptr);
    pumpFor(300);
}


