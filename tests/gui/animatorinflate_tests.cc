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

TEST_F(ANIMATORINFLATOR,test1){
    App app;
    StateListAnimator*sl=AnimatorInflater::loadStateListAnimator(&app,"cdroid:animator/test1.xml");
    app.exec();
}
TEST_F(ANIMATORINFLATOR,test2){
    App app;
    StateListAnimator*sl=AnimatorInflater::loadStateListAnimator(&app,"cdroid:animator/test2.xml");
    app.exec();
}



