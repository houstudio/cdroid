#include <gtest/gtest.h>
#include <cdroid.h>
#include <view/velocitytracker.h>
using namespace cdroid;

class VELOCITY:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};


TEST_F(VELOCITY,test1){
   VelocityTracker*vc =VelocityTracker::obtain();
   nsecs_t etime = SystemClock::uptimeMillis();
   for(int i=0,x=0,y=0;i<10;i++){
       MotionEvent*e=MotionEvent::obtain(etime,etime,MotionEvent::ACTION_MOVE,x,y,0);
       vc->addMovement(*e);
       etime+=10;
       y+=10;x+=6;
   }
   vc->computeCurrentVelocity(1000,8000.0f);
   int velocity=vc->getYVelocity(0);
   printf("velocity=%d\r\n",velocity);
}
