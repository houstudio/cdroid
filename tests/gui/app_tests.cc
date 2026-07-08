#include <gtest/gtest.h>
#include <core/app.h>
#include <widget/cdwindow.h>
#include <widget/textview.h>
#include <view/velocitytracker.h>
#include <view/inputevent.h>
#include <porting/cdlog.h>
#include <guienvironment.h>

using namespace cdroid;

class APP:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

/* The App is constructed once for the whole process in GUIEnvironment::SetUp,
   so these cases no longer build/tear-down their own App or call the blocking
   App::exec(). They keep the shared Looper ticking briefly, then move on. */
TEST_F(APP,EmptyArgs){
   pumpFor(50);
}

TEST_F(APP,TwoArgs){
   pumpFor(50);
}
TEST_F(APP,exec){
   pumpFor(50);
}
TEST_F(APP,add){
   Window*w=GUIEnvironment::stage();
   w->addView(new TextView("",100,20));
   w->addView(new TextView("",100,20));
   ASSERT_EQ(2,w->getChildCount());
   pumpFor(100);
}
TEST_F(APP,velocity){
   VelocityTracker*velocity=VelocityTracker::obtain();
   //obtain(nsecs_t downTime, nsecs_t eventTime, int action, float x, float y, int metaState)
   for(int i=0;i<10;i++){
       MotionEvent* event=MotionEvent::obtain(i*10,i*10,MotionEvent::ACTION_MOVE,i*5/*x*/,i*10/*y*/,0);
       velocity->addMovement(*event);
       event->recycle();
   }
   velocity->computeCurrentVelocity(1000,8000);
   float ox=velocity->getXVelocity(0);
   float oy=velocity->getYVelocity(0);
   LOGD("xy=%f,%f",ox,oy);
   pumpFor(100);
}
