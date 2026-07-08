#include <gtest/gtest.h>
#include <core/app.h>
#include <porting/cdlog.h>
#include <animation/objectanimator.h>
#include <animation/animatorinflater.h>
#include <guienvironment.h>
using namespace cdroid;
#include <guienvironment.h>
class ANIMATORINFLATOR:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(ANIMATORINFLATOR,statelistanimator1){
    App&app=App::getInstance();
    StateListAnimator*sl=AnimatorInflater::loadStateListAnimator(&app,"cdroid:animator/test1.xml");
    ASSERT_NE(sl,(void*)nullptr);
    pumpFor(300);
}

TEST_F(ANIMATORINFLATOR,statelistanimator2){
    App&app=App::getInstance();
    StateListAnimator*sl=AnimatorInflater::loadStateListAnimator(&app,"cdroid:animator/test2.xml");
    ASSERT_NE(sl,(void*)nullptr);
    pumpFor(300);
}

TEST_F(ANIMATORINFLATOR,test2){
    App&app=App::getInstance();
    StateListAnimator*sl=AnimatorInflater::loadStateListAnimator(&app,"cdroid:animator/test2");
    ASSERT_NE(sl,(void*)nullptr);
    pumpFor(300);
}

TEST_F(ANIMATORINFLATOR,fade_in){
    App&app=App::getInstance();
    Animator*sl=AnimatorInflater::loadAnimator(&app,"cdroid:anim/fad_in");
    ASSERT_NE(sl,(void*)nullptr);
    pumpFor(300);
}
TEST_F(ANIMATORINFLATOR,slide_in_left){
    App&app=App::getInstance();
    Animator*sl=AnimatorInflater::loadAnimator(&app,"cdroid:anim/slide_in_left");
    ASSERT_NE(sl,(void*)nullptr);
    pumpFor(300);
}


