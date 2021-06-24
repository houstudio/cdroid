#include <gtest/gtest.h>
#include <windows.h>
#include <looper/TimerFD.h>
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
   app.addEventSource(new TimerFD(800,true),[&](EventSource&s)->bool{
      app.exit();
      return false;
   });
   app.exec();
}

TEST_F(APP,TwoArgs){
   const char*args[]={"arg1","--alpha=255"};
   App app(2,args);
   app.addEventSource(new TimerFD(800,true),[&](EventSource&s)->bool{
      app.exit();
      return false;
   });
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
   app.addEventSource(new TimerFD(5000,true),[&](EventSource&s)->bool{
      app.exit();
      return false;
   });
   app.exec();
}
