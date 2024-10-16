#include <stdio.h>
#if defined(__linux__)||defined(__unix__)
#include <sys/time.h>
#elif defined(_WIN32)||defined(_WIN64)
#include <Windows.h>
#endif
#include <gtest/gtest.h>
#include <porting/dtvos.h>

class OSSEM:public testing::Test{
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
TEST_F(OSSEM,SEM_Create_1){
   NGLSemaphore sem;
   ASSERT_EQ(0,nglCreateSemaphore(&sem,0));
   ASSERT_EQ(0,nglDeleteSemaphore(sem));
}

TEST_F(OSSEM,SEM_Create_2){
   NGLSemaphore sem;
   ASSERT_NE(0,nglCreateSemaphore(NULL,0));
}

TEST_F(OSSEM,SEM_Delete_1){
   NGLSemaphore sem;
   ASSERT_EQ(0,nglCreateSemaphore(&sem,0));
   ASSERT_EQ(0,nglDeleteSemaphore(sem));
}

TEST_F(OSSEM,SEM_Delete_Error_1){
   ASSERT_NE(0,nglDeleteSemaphore(0));
}

TEST_F(OSSEM,SEM_Acquire_1){
   NGLSemaphore sem;
   ASSERT_EQ(0,nglCreateSemaphore(&sem,1));
   ASSERT_EQ(0,nglAcquireSemaphore(sem,-1));
}

TEST_F(OSSEM,SEM_Acquire_2){
   NGLSemaphore sem;
   ASSERT_EQ(0,nglCreateSemaphore(&sem,2));
   ASSERT_EQ(0,nglAcquireSemaphore(sem,-1));
   ASSERT_EQ(0,nglAcquireSemaphore(sem,-1));
   ASSERT_EQ(0,nglDeleteSemaphore(sem));
}

static void SemPostProc(void*p){
   printf("SemPostProc sem=%p\r\n",p);
   sleep(5);
   nglReleaseSemaphore(p);
   printf("nglReleaseSemaphore %p\r\n",p);
}
TEST_F(OSSEM,SEM_Acquire_FOREVER){
   NGLSemaphore sem; 
   HANDLE tid;
   ASSERT_EQ(0,nglCreateSemaphore(&sem,0));
   nglCreateThread(&tid,0,0,SemPostProc,(void*)sem);
   ASSERT_EQ(0,nglAcquireSemaphore(sem,-1));
}

TEST_F(OSSEM,SEM_Acquire_Timeout_1){
   NGLSemaphore sem;
   ASSERT_EQ(0,nglCreateSemaphore(&sem,1));
   gettimeofday(&tv1,NULL);
   ASSERT_EQ(0,nglAcquireSemaphore(sem,1000));
   gettimeofday(&tv2,NULL);
   unsigned long long dur=duration();
   ASSERT_LE(dur,10);
   ASSERT_EQ(0,nglDeleteSemaphore(sem));
}
TEST_F(OSSEM,SEM_Acquire_Timeout_2){
   NGLSemaphore sem;
   ASSERT_EQ(0,nglCreateSemaphore(&sem,0));
   gettimeofday(&tv1,NULL);
   ASSERT_NE(0,nglAcquireSemaphore(sem,1000));
   gettimeofday(&tv2,NULL);
   unsigned long long dur=duration();
   ASSERT_LE(dur,1010);
   ASSERT_GT(dur,990);
   ASSERT_EQ(0,nglDeleteSemaphore(sem));
}


TEST_F(OSSEM,SEM_Release_1){
   NGLSemaphore sem;
   ASSERT_EQ(0,nglCreateSemaphore(&sem,1));
   ASSERT_EQ(0,nglReleaseSemaphore(sem));
   ASSERT_EQ(0,nglDeleteSemaphore(sem));
}

TEST_F(OSSEM,SEM_Release_2){
   NGLSemaphore sem;
   ASSERT_EQ(0,nglCreateSemaphore(&sem,0));
   ASSERT_EQ(0,nglReleaseSemaphore(sem));
   ASSERT_EQ(0,nglAcquireSemaphore(sem,-1));
   ASSERT_EQ(0,nglDeleteSemaphore(sem));
}
#endif
