#include <gtest/gtest.h>
#include <core/app.h>
#include <porting/cdlog.h>
#include <animation/objectanimator.h>
#include <animation/animatorinflater.h>
#include <animation/animationutils.h>
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


