#include <gtest/gtest.h>
#include <cdroid.h>
#include <cdlog.h>
#include <animation/objectanimator.h>
#include <animation/animatorinflater.h>
class ANIMATORINFLATOR:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(ANIMATORINFLATOR,statelistanimator1){
    App app;
    StateListAnimator*sl=AnimatorInflater::loadStateListAnimator(&app,"cdroid:animator/test1.xml");
    ASSERT_NE(sl,(void*)nullptr);
    app.exec();
}
TEST_F(ANIMATORINFLATOR,statelistanimator2){
    App app;
    StateListAnimator*sl=AnimatorInflater::loadStateListAnimator(&app,"cdroid:animator/test2.xml");
    ASSERT_NE(sl,(void*)nullptr);
    app.exec();
}

TEST_F(ANIMATORINFLATOR,fade_in){
    App app;
    Animator*sl=AnimatorInflater::loadAnimator(&app,"cdroid:anim/fad_in");
    ASSERT_NE(sl,(void*)nullptr);
    app.exec();
}
TEST_F(ANIMATORINFLATOR,slide_in_left){
    App app;
    Animator*sl=AnimatorInflater::loadAnimator(&app,"cdroid:anim/slide_in_left");
    ASSERT_NE(sl,(void*)nullptr);
    app.exec();
}




