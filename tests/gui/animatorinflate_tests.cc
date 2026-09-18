#include <gtest/gtest.h>
#include <core/app.h>
#include <porting/cdlog.h>
#include <animation/objectanimator.h>
#include <animation/animatorinflater.h>
#include <animation/animationutils.h>
#include <animation/animatorset.h>
#include <animation/statelistanimator.h>
#include <animation/keyframeset.h>
#include <animation/pathkeyframes.h>
#include <view/view.h>
#include <content/resources.h>   // Resources::newTheme()/_engineHandle (cache-key test)
#include <widget/internal_R.h>    // framework interpolator ids
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

// AnimationUtils' interpolator cache is keyed (id, theme engine): the same
// interpolator id under the context's theme, under a second independent
// theme (Resources::newTheme()), and under a null theme must resolve to
// three DISTINCT entries, while a repeated load under the same key hits the
// same process-resident instance. Cached interpolators are borrowed — never
// deleted here (OWNERSHIP note on loadInterpolator).
TEST_F(ANIMATORINFLATOR,interpolator_cache_theme_keyed){
    App&app=App::getInstance();
    Resources&res=app.getResources();
    const int id=internal::R::interpolator::decelerate_quad;

    // Context overload: cached under (id, context theme engine).
    Interpolator*a=AnimationUtils::loadInterpolator(&app,id);
    Interpolator*a2=AnimationUtils::loadInterpolator(&app,id);
    ASSERT_NE(a,(void*)nullptr);
    EXPECT_EQ(a,a2) << "same theme engine must hit the cache";

    // @hide overload with an independent engine (newTheme() owns its engine).
    Resources::Theme other=res.newTheme();
    ASSERT_NE(other._engineHandle(),app.getTheme()._engineHandle())
        << "newTheme() must hand out an independent engine";
    Interpolator*b=AnimationUtils::loadInterpolator(&res,&other,id);
    Interpolator*b2=AnimationUtils::loadInterpolator(&res,&other,id);
    ASSERT_NE(b,(void*)nullptr);
    EXPECT_NE(a,b) << "same id under a different theme engine must not share the entry";
    EXPECT_EQ(b,b2) << "the second engine must have its own cache hit";

    // Null theme styles themelessly (engine key nullptr) — a third entry.
    Interpolator*c=AnimationUtils::loadInterpolator(&res,nullptr,id);
    ASSERT_NE(c,(void*)nullptr);
    EXPECT_NE(c,a);
    EXPECT_NE(c,b);

    // All three parsed the same XML: decelerate factor defaults to 1.0, so
    // 1-(1-t)^2 = 0.75 at t=0.5 — each entry is a functional interpolator.
    EXPECT_NEAR(a->getInterpolation(0.5f),0.75f,1e-3f);
    EXPECT_NEAR(b->getInterpolation(0.5f),0.75f,1e-3f);
    EXPECT_NEAR(c->getInterpolation(0.5f),0.75f,1e-3f);
}



// --- Keyframes (android.animation.Keyframe/KeyframeSet port) ---------------

// Uniform fractions: ofFloat({0,100}) interpolates linearly across the range.
TEST_F(ANIMATORINFLATOR,keyframes_uniform_interpolation){
    KeyframeSet*ks = KeyframeSet::ofFloat({0.f,100.f});
    ASSERT_NE(ks,(void*)nullptr);
    FloatKeyframeSet*fks = dynamic_cast<FloatKeyframeSet*>(ks);
    ASSERT_NE(fks,(FloatKeyframeSet*)nullptr);
    EXPECT_NEAR(fks->getFloatValue(0.f),0.f,0.001f);
    EXPECT_NEAR(fks->getFloatValue(0.25f),25.f,0.001f);
    EXPECT_NEAR(fks->getFloatValue(0.5f),50.f,0.001f);
    EXPECT_NEAR(fks->getFloatValue(1.f),100.f,0.001f);
    // >2 values walk the right segment: {0,100,200} at 0.75 → 150.
    KeyframeSet*ks3 = KeyframeSet::ofFloat({0.f,100.f,200.f});
    FloatKeyframeSet*fks3 = dynamic_cast<FloatKeyframeSet*>(ks3);
    ASSERT_NE(fks3,(FloatKeyframeSet*)nullptr);
    EXPECT_NEAR(fks3->getFloatValue(0.75f),150.f,0.001f);
    delete ks; delete ks3;
}

// Non-uniform fractions via ofKeyframe(): segment lookup must use the
// keyframes' own fractions, not uniform spacing. Keyframes at 0/0.25/1:
// fraction 0.5 sits 1/3 into the second segment → 50 + (200-50)/3 = 100.
TEST_F(ANIMATORINFLATOR,keyframes_nonuniform_fractions){
    std::vector<Keyframe*> kfs = {
        Keyframe::ofFloat(0.f, 0.f),
        Keyframe::ofFloat(0.25f, 50.f),
        Keyframe::ofFloat(1.f, 200.f),
    };
    KeyframeSet*ks = KeyframeSet::ofKeyframe(kfs);
    FloatKeyframeSet*fks = dynamic_cast<FloatKeyframeSet*>(ks);
    ASSERT_NE(fks,(FloatKeyframeSet*)nullptr);
    EXPECT_NEAR(fks->getFloatValue(0.125f),25.f,0.001f);   // mid of first segment
    EXPECT_NEAR(fks->getFloatValue(0.5f),100.f,0.001f);    // 1/3 into second
    EXPECT_NEAR(fks->getFloatValue(1.f),200.f,0.001f);
    delete ks;
}

// Single value → [no-value@0, value@1]; the empty start keyframe reports
// hasValue()==false until filled (AOSP setupValue semantics).
TEST_F(ANIMATORINFLATOR,keyframes_single_value_hasvalue){
    KeyframeSet*ks = KeyframeSet::ofFloat({100.f});
    ASSERT_EQ(ks->getKeyframes().size(),(size_t)2);
    EXPECT_FALSE(ks->getKeyframes()[0]->hasValue());
    EXPECT_TRUE (ks->getKeyframes()[1]->hasValue());
    ks->getKeyframes()[0]->setValue(25.f);
    EXPECT_TRUE(ks->getKeyframes()[0]->hasValue());
    FloatKeyframeSet*fks = dynamic_cast<FloatKeyframeSet*>(ks);
    ASSERT_NE(fks,(FloatKeyframeSet*)nullptr);
    EXPECT_NEAR(fks->getFloatValue(0.5f),62.5f,0.001f);   // 25→100 midpoint
    delete ks;
}

// KeyframeSet::clone() deep-copies the keyframes.
TEST_F(ANIMATORINFLATOR,keyframes_clone_independent){
    std::vector<Keyframe*> kfs = {
        Keyframe::ofFloat(0.f, 0.f),
        Keyframe::ofFloat(1.f, 100.f),
    };
    KeyframeSet*orig = KeyframeSet::ofKeyframe(kfs);
    KeyframeSet*copy = orig->clone();
    ASSERT_NE(copy,(KeyframeSet*)nullptr);
    ASSERT_NE(copy,orig);
    ASSERT_EQ(copy->getKeyframes().size(),orig->getKeyframes().size());
    ASSERT_NE(copy->getKeyframes()[1],orig->getKeyframes()[1]);
    orig->getKeyframes()[1]->setValue(999.f);
    FloatKeyframeSet*fcopy = dynamic_cast<FloatKeyframeSet*>(copy);
    ASSERT_NE(fcopy,(FloatKeyframeSet*)nullptr);
    EXPECT_NEAR(fcopy->getFloatValue(1.f),100.f,0.001f);  // clone unaffected
    delete orig; delete copy;
}

// PHV over keyframes: load keyframes_test.xml (propertyValuesHolder with three
// <keyframe> elements at 0/0.5/1) through the int-resid entry.
TEST_F(ANIMATORINFLATOR,keyframes_xml){
    App&app=App::getInstance();
    Animator*anim=AnimatorInflater::loadAnimator(&app,gui_test::R::animator::keyframes_test);
    ASSERT_NE(anim,(void*)nullptr);
    ObjectAnimator*oa=dynamic_cast<ObjectAnimator*>(anim);
    ASSERT_NE(oa,(ObjectAnimator*)nullptr);
    ASSERT_EQ(oa->getValues().size(),(size_t)1);
    EXPECT_EQ(oa->getPropertyName(),"translationX");
    EXPECT_EQ(oa->getValues(0)->getValueType(),Property::FLOAT_TYPE);
    delete anim;
}

// PathKeyframes sampling: M0,0 L100,100 — arc-length fractions land on the
// diagonal, X/Y projections read the coordinates.
TEST_F(ANIMATORINFLATOR,pathkeyframes_sampling){
    auto path = PathParser::createPathFromPathData("M 0,0 L 100,100");
    auto pk = std::make_shared<PathKeyframes>(path, 0.5f);
    const PointF& mid = pk->pointForFraction(0.5f);
    EXPECT_NEAR(mid.x,50.f,0.6f);
    EXPECT_NEAR(mid.y,50.f,0.6f);
    FloatKeyframes* xf = pk->createXFloatKeyframes();
    FloatKeyframes* yf = pk->createYFloatKeyframes();
    EXPECT_NEAR(xf->getFloatValue(0.25f),25.f,0.6f);
    EXPECT_NEAR(yf->getFloatValue(0.75f),75.f,0.6f);
    IntKeyframes* xi = pk->createXIntKeyframes();
    EXPECT_EQ(xi->getIntValue(1.f),100);
    delete xf; delete yf; delete xi;
}

// setupObjectAnimator's path case (was #if 0): propertyXName/propertyYName +
// pathData must build two PHVs and actually animate a view along the path.
TEST_F(ANIMATORINFLATOR,pathxy_xml){
    App&app=App::getInstance();
    Animator*anim=AnimatorInflater::loadAnimator(&app,gui_test::R::animator::pathxy_test);
    ASSERT_NE(anim,(void*)nullptr);
    ObjectAnimator*oa=dynamic_cast<ObjectAnimator*>(anim);
    ASSERT_NE(oa,(ObjectAnimator*)nullptr);
    ASSERT_EQ(oa->getValues().size(),(size_t)2) << "expected X and Y holders";
    EXPECT_EQ(oa->getValues(0)->getPropertyName(),"translationX");
    EXPECT_EQ(oa->getValues(1)->getPropertyName(),"translationY");
    View*v=new View(&app);
    GUIEnvironment::content()->addView(v);
    oa->setTarget(v);
    oa->start();
    pumpFor(500);
    EXPECT_NEAR(v->getTranslationX(),100.f,1.f) << "X did not animate along the path";
    EXPECT_NEAR(v->getTranslationY(),100.f,1.f) << "Y did not animate along the path";
    delete oa;
    delete v;
}

// PathData animators (AVD path morphing): the keyframes' scratch value must
// be seeded with the PathData alternative before the first evaluate — the
// evaluator writes in place via GET_VARIANT(out, PathData), and an unseeded
// variant aborted widgetsDemo with bad_variant_access at start().
TEST_F(ANIMATORINFLATOR,pathdata_animator_start){
    PropertyValuesHolder*pvh = PropertyValuesHolder::ofObject("pathData",
            {PathParser::PathData("M 0,0 L 10,10"), PathParser::PathData("M 0,0 L 20,20")});
    ValueAnimator*va = ValueAnimator::ofPropertyValuesHolder({pvh});
    va->setDuration(100);
    va->start();            // start() -> setCurrentPlayTime(0) -> PathDataEvaluator
    pumpFor(60);
    va->end();
    delete va;
}
