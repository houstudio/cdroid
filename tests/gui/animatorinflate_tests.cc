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

    View*v=new View(&app);
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
    // Value level: alpha must actually animate to valueTo=1.0 (the old
    // string-probing getPVH lost every typed value and the set ran empty).
    v->setAlpha(0.f);
    pumpFor(600);
    ASSERT_NEAR(v->getAlpha(),1.f,0.01f) << "alpha did not animate to valueTo=1.0";

    v->setActivated(false); // back to base state -> 300ms animators
    running=v->getStateListAnimator()->getRunningAnimator();
    ASSERT_NE(running,(void*)nullptr);
    set=dynamic_cast<AnimatorSet*>(running);
    ASSERT_NE(set,(AnimatorSet*)nullptr);
    ASSERT_EQ(set->getChildAnimations().size(),(size_t)4);
    ASSERT_EQ(set->getTotalDuration(),(int64_t)300);
    pumpFor(400);
    ASSERT_NEAR(v->getAlpha(),0.7f,0.01f) << "alpha did not animate to valueTo=0.7";
    delete v; // deletes the SLA it owns
}

TEST_F(ANIMATORINFLATOR,fade_in){
    App&app=App::getInstance();
    Animator*sl=AnimatorInflater::loadAnimator(&app,gui_test::R::animator::fade_in);
    ASSERT_NE(sl,(void*)nullptr);
    pumpFor(300);
}

// AOSP AnimatorInflater.loadAnimator(Resources, Theme, id): the
// ConfigurationBoundResourceCache stores ConstantStates and every load returns
// newInstance() — two loads of the same id are DISTINCT pristine animators
// (never the cached source), and mutating one must not leak into the other.
TEST_F(ANIMATORINFLATOR,animator_cache_double_load){
    App&app=App::getInstance();
    Animator*a=AnimatorInflater::loadAnimator(&app,gui_test::R::animator::fade_in);
    Animator*b=AnimatorInflater::loadAnimator(&app,gui_test::R::animator::fade_in);
    ASSERT_NE(a,(void*)nullptr);
    ASSERT_NE(b,(void*)nullptr);
    EXPECT_NE(a,b) << "loads must be distinct instances (newInstance, not the cached source)";
    EXPECT_EQ(a->getDuration(),b->getDuration());
    EXPECT_FALSE(a->isStarted());
    EXPECT_FALSE(b->isStarted());
    const int64_t xmlDuration=a->getDuration();
    a->setDuration(12345);
    EXPECT_EQ(b->getDuration(),xmlDuration) << "mutating one clone must not affect the other";
    ObjectAnimator*oa=dynamic_cast<ObjectAnimator*>(a);
    ObjectAnimator*ob=dynamic_cast<ObjectAnimator*>(b);
    ASSERT_NE(oa,(ObjectAnimator*)nullptr);
    ASSERT_NE(ob,(ObjectAnimator*)nullptr);
    EXPECT_EQ(oa->getPropertyName(),ob->getPropertyName());
    delete a;
    delete b;
}

// Same for StateListAnimator: cached by int id, every load a deep clone — the
// clone's tuple animators are freshly cloned (no stale listeners bound to the
// cached source), and destroying one clone leaves the others usable.
TEST_F(ANIMATORINFLATOR,sla_cache_double_load){
    App&app=App::getInstance();
    StateListAnimator*a=AnimatorInflater::loadStateListAnimator(&app,
            gui_test::R::animator::statelist_test);
    StateListAnimator*b=AnimatorInflater::loadStateListAnimator(&app,
            gui_test::R::animator::statelist_test);
    ASSERT_NE(a,(void*)nullptr);
    ASSERT_NE(b,(void*)nullptr);
    EXPECT_NE(a,b);
    delete a;  // the cache's own source animator must survive this
    // A third load goes through the cache HIT path (put by the first load).
    StateListAnimator*c=AnimatorInflater::loadStateListAnimator(&app,
            gui_test::R::animator::statelist_test);
    ASSERT_NE(c,(void*)nullptr);
    EXPECT_NE(c,b);
    // Attach to a real view (setState on a targetless SLA would deref a null
    // target inside the tuple animator — same as AOSP's NPE) and drive it.
    // setActivated, not setPressed: the pressed dispatch is gated on
    // mAttachInfo (detached test view) while activated is not. A detached view
    // is not "aggregated visible", so drawableStateChanged JUMPS the just-
    // started animator to its end value (AOSP "skip any animated changes") —
    // which exercises the clone's full PHV pipeline: alpha must land on the
    // default item's valueTo (0.5).
    View*v=new View(&app);
    v->setStateListAnimator(b);  // takes ownership, sets target
    v->setActivated(true);
    EXPECT_NEAR(v->getAlpha(),0.5f,0.01f) << "clone must apply the tuple animator's end value";
    pumpFor(50);
    delete c;
    delete v;  // deletes b (its SLA)
}

// ConstantState contract: createConstantState() must not throw (it used to
// call shared_from_this() inside the constructor -> bad_weak_ptr), adopts the
// source animator, and newInstance() yields independent clones. The source is
// owned by the constant state — do NOT delete it here.
TEST_F(ANIMATORINFLATOR,constantstate_newinstance_independent){
    ValueAnimator*va=ValueAnimator::ofFloat({0.f,1.f});
    va->setDuration(777);
    // auto: the nested constant-state type is private to Animator.
    const auto cs=va->createConstantState();
    ASSERT_NE(cs,nullptr);
    Animator*c1=cs->newInstance();
    Animator*c2=cs->newInstance();
    ASSERT_NE(c1,(void*)nullptr);
    ASSERT_NE(c2,(void*)nullptr);
    EXPECT_NE(c1,c2);
    EXPECT_NE(c1,va);
    EXPECT_EQ(c1->getDuration(),(int64_t)777);
    EXPECT_EQ(c2->getDuration(),(int64_t)777);
    c1->setDuration(999);
    EXPECT_EQ(c2->getDuration(),(int64_t)777) << "clones must not share state";
    EXPECT_FALSE(c1->isStarted());
    delete c1;
    delete c2;
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


