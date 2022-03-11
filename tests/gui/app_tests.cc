#include <gtest/gtest.h>
#include <cdroid.h>
#include <ngl_os.h>

using namespace cdroid;

class APP:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};


TEST_F(APP,EmptyArgs){
   App app(0,NULL);
   app.exec();
}

TEST_F(APP,TwoArgs){
   const char*args[]={"arg1","--alpha=255"};
   App app(2,args);
   app.exec();
}
TEST_F(APP,exec){
   static const char*args[]={"arg1","alpha",NULL};
   App app(2,args);
   app.exec();
}
TEST_F(APP,add){
   App app;
   Window*w=new Window(0,0,100,100);
   w->addView(new TextView("",100,20)); 
   w->addView(new TextView("",100,20));
   ASSERT_EQ(2,w->getChildCount());
   app.exec();
}
TEST_F(APP,velocity){
   App app;
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
   app.exec();
}
