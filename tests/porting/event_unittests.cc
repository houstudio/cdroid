#include <stdio.h>
#include <sys/time.h>
#include <gtest/gtest.h>
#include <dtvos.h>

class OSEVENT:public testing::Test{
   public :
   struct timeval tv1,tv2;
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
   unsigned long long duration(){
       return tv2.tv_sec*1000l+tv2.tv_usec/1000-tv1.tv_sec*1000l-tv1.tv_usec/1000;
   }
};

#ifdef ENABLE_DTV
TEST_F(OSEVENT,Create_Auto_1){
   HANDLE hdl=nglCreateEvent(0,1);//init as FALSE, auto reset
   ASSERT_NE((HANDLE)nullptr,hdl);
   gettimeofday(&tv1,NULL);
   ASSERT_NE(0,nglWaitEvent(hdl,1000));
   gettimeofday(&tv2,NULL);
   ASSERT_GE(duration(),1000);
   ASSERT_LE(duration(),1010);
   ASSERT_EQ(0,nglDestroyEvent(hdl));
}

TEST_F(OSEVENT,Create_Auto_2){
   HANDLE hdl=nglCreateEvent(1,1);//init as TRUE, auto reset
   ASSERT_NE((HANDLE)nullptr,hdl);
   gettimeofday(&tv1,NULL);
   ASSERT_EQ(0,nglWaitEvent(hdl,1000));//first wait will reset the event
   ASSERT_NE(0,nglWaitEvent(hdl,1000));//follow wait will failed
   gettimeofday(&tv2,NULL);
   ASSERT_GT(duration(),990);
   ASSERT_EQ(0,nglDestroyEvent(hdl));
}

TEST_F(OSEVENT,Create_Manual_1){
   HANDLE hdl=nglCreateEvent(1,0);//manual reset ,after wait success,must call resetevent
   ASSERT_NE((HANDLE)nullptr,hdl);
   gettimeofday(&tv1,NULL);
   ASSERT_EQ(0,nglWaitEvent(hdl,1000));
   gettimeofday(&tv2,NULL);
   ASSERT_LE(duration(),10);

   gettimeofday(&tv1,NULL);
   nglResetEvent(hdl);
   ASSERT_NE(0,nglWaitEvent(hdl,1000));
   gettimeofday(&tv2,NULL);
   ASSERT_GE(duration(),1000);
   ASSERT_EQ(nglDestroyEvent(hdl),E_OK);
}

TEST_F(OSEVENT,Create_Manual_2){
   HANDLE hdl=nglCreateEvent(0,0);
   ASSERT_NE((HANDLE)nullptr,hdl);

   gettimeofday(&tv1,NULL);
   ASSERT_NE(nglWaitEvent(hdl,1000),E_OK);

   gettimeofday(&tv2,NULL);
   ASSERT_GE(duration(),990);
   ASSERT_LE(duration(),1010);

   gettimeofday(&tv1,NULL);
   ASSERT_NE(nglWaitEvent(hdl,1000),E_OK);
   gettimeofday(&tv2,NULL);
   ASSERT_GE(duration(),990);
   ASSERT_LE(duration(),1010); 
   ASSERT_EQ(nglDestroyEvent(hdl),E_OK);
}
static void EventProc(void*p){
   sleep(5);
   nglSetEvent(p);
}
TEST_F(OSEVENT,WAIT_FOREVER){
   HANDLE tid;
   HANDLE hdl=nglCreateEvent(0,0);
   ASSERT_NE((HANDLE)nullptr,hdl);
   nglCreateThread(&tid,0,0,EventProc,hdl);
   gettimeofday(&tv1,NULL);
   ASSERT_EQ(nglWaitEvent(hdl,-1),E_OK);
   gettimeofday(&tv2,NULL);
   ASSERT_GT(duration(),4800);
   nglDestroyEvent(hdl);
}
#endif

