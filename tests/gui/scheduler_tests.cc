#include <gtest/gtest.h>
#include <windows.h>
#include <ngl_os.h>
#include <core/scheduler.h>

class SCHEDULER:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};
struct SubscribeItem{
   int serviceid;
   system_clock::time_point time;
   UINT eventid;
   UINT duration;/*unint in seconds*/
   std::string name;
   void onTriggered(){
      printf("triggered serviceid:%d event.name=%s\r\n",serviceid,name.c_str());
   }
};


TEST_F(SCHEDULER,Once){
   Scheduler sch;
   int count=0;
   SubscribeItem itm;
   itm.serviceid=100;
   itm.name="test";
   sch.schedule([&count](){count++;},system_clock::now()+std::chrono::seconds(10));
   sch.schedule(std::bind(&SubscribeItem::onTriggered,&itm),system_clock::now()+std::chrono::seconds(10));
   sleep(10);
   //sch.check(); 
   //sch.check();
   ASSERT_EQ(count,1);
}

TEST_F(SCHEDULER,FromNow){
   Scheduler sch;
   int count=0;
   sch.scheduleFromNow([&count](){count++;},10);
   sleep(10);
   //sch.check();
   ASSERT_EQ(count,1);
}

TEST_F(SCHEDULER,Every){
   Scheduler sch;
   int count=0;
   SubscribeItem itm;
   itm.serviceid=200;
   itm.name="test.every 10 seconds!";
   sch.scheduleEvery([&count](){count++;},5);
   sch.scheduleEvery(std::bind(&SubscribeItem::onTriggered,&itm),5);
   for(int i=0;i<20;i++){
       sleep(1);
       //sch.check();
   }
   ASSERT_EQ(count,4);
}

TEST_F(SCHEDULER,Hourly){
   Scheduler sch;
   int count=0;
   sch.scheduleHourly([&count](){count++;},system_clock::now());
   sleep(10);
   //sch.check();
   ASSERT_EQ(count,0);
}

TEST_F(SCHEDULER,Hourly1){
   Scheduler sch;
   int count=0;
   sch.scheduleHourly([&count](){count++;},system_clock::now());
   sleep(3600);
   //sch.check();
   ASSERT_EQ(count,1);
}

TEST_F(SCHEDULER,Daily){
   Scheduler sch;
   int count=0;
   sch.scheduleDaily([&count](){count++;},system_clock::now());
   sleep(10);
   //sch.check();
   ASSERT_EQ(count,0);
}

TEST_F(SCHEDULER,Weekly){
   Scheduler sch;
   int count=0;
   sch.scheduleWeekly([&count](){count++;},system_clock::now());
   sleep(10);
   //sch.check();
   ASSERT_EQ(count,0);
}

