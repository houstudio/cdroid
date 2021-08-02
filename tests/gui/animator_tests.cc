#include <gtest/gtest.h>
#include <windows.h>
#include <ngl_os.h>
#include <ngl_timer.h>
#include <cdlog.h>
#include <animation/objectanimator.h>

class ANIMATOR:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(ANIMATOR,ofInt){
    ValueAnimator*anim=ValueAnimator::ofInt(2,0,100);
    anim->addUpdateListener([](ValueAnimator&anim){
        LOGD("value=%f",anim.getAnimatedValue());
    });
    anim->setDuration(100);
    for(int i=0;i<=10;i++){
        anim->setCurrentFraction((float)i/10.f);
    }
}

TEST_F(ANIMATOR,ofFlot){
    ObjectAnimator*anim=ObjectAnimator::ofFloat(this,"test",2,0,100);
    anim->addUpdateListener([](ValueAnimator&anim){
        LOGD("value=%f",anim.getAnimatedValue());
    });
    anim->setDuration(100);
    for(int i=0;i<=10;i++){
        anim->setCurrentFraction((float)i/10.f);
    }
}


