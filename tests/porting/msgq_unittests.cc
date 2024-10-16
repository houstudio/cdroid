#include <stdio.h>
#if defined(__linux__)||defined(__unix___)
#include <sys/time.h>
#else
#include <Windows.h>
#endif
#include <gtest/gtest.h>
extern "C"{
#include <porting/dtvos.h>
#include <porting/dtvmsgq.h>
}

class OSMSGQ:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
   unsigned long long gettime(){
#if defined(__linux__)||defined(__unix__)
       struct timeval tv;
       gettimeofday(&tv,NULL);
       return tv.tv_sec*1000+tv.tv_usec/1000;
#elif defined(_WIN32)||defined(_WIN64)
       return GetTickCount64();
#endif
   }
};
typedef struct{
   HANDLE q;
   int snd_delay;
   int rcv_delay;
   uint32_t*msgs;
}TESTQUEUE;

#ifdef ENABLE_DTV

TEST_F(OSMSGQ,Create_1){
   HANDLE q=nglMsgQCreate(10,10);
   ASSERT_NE((HANDLE)nullptr,q);
   ASSERT_TRUE(0==nglMsgQDestroy(q));
}

TEST_F(OSMSGQ,Create_Error_1){
   HANDLE q=nglMsgQCreate(0,10);
   ASSERT_EQ(nullptr,q);
}

TEST_F(OSMSGQ,Create_Error_2){
   HANDLE q=nglMsgQCreate(10,0);
   ASSERT_EQ(nullptr,q);
}

static void MsgQProc(void*p){
   int msg=3201;
   sleep(5);
   nglMsgQSend((HANDLE)p,&msg,sizeof(int),-1);
}

TEST_F(OSMSGQ,Recv_1){
   int msg=1023;
   HANDLE tid;
   HANDLE q=nglMsgQCreate(10,sizeof(int));
   ASSERT_NE((HANDLE)nullptr,q);
   ASSERT_EQ(0,nglMsgQSend(q,&msg,sizeof(int),-1));
   msg=0;
   ASSERT_EQ(0,nglMsgQReceive(q,&msg,sizeof(int),-1)); 
   ASSERT_EQ(1023,msg);
   nglCreateThread(&tid,0,0,MsgQProc,(void*)q);
   ASSERT_EQ(0,nglMsgQReceive(q,&msg,sizeof(int),-1));
   ASSERT_EQ(3201,msg);
   ASSERT_EQ(0,nglMsgQDestroy(q));
}


TEST_F(OSMSGQ,Recv_Timeout_1){
   int msg=1023;
   unsigned long long tv1,tv2;
   HANDLE q=nglMsgQCreate(10,sizeof(int));
   ASSERT_NE((HANDLE)nullptr,q);
   tv1=gettime();
   ASSERT_NE(0,nglMsgQReceive(q,&msg,sizeof(int),1000));
   tv2=gettime();
   ASSERT_TRUE(tv2-tv1<1010&&tv2-tv1>990);
   ASSERT_EQ(0,nglMsgQDestroy(q));
}

TEST_F(OSMSGQ,Send_Timeout_1){
   int msg=1023;
   unsigned long long tv1,tv2;
   HANDLE q=nglMsgQCreate(2,sizeof(int));
   ASSERT_NE((HANDLE)nullptr,q);
   tv1=gettime();
   ASSERT_EQ(0,nglMsgQSend(q,&msg,sizeof(int),1000));
   ASSERT_EQ(0,nglMsgQSend(q,&msg,sizeof(int),1000));
   ASSERT_NE(0,nglMsgQSend(q,&msg,sizeof(int),1000));
   tv2=gettime();
   ASSERT_TRUE(tv2-tv1<1010&&tv2-tv1>990);
   ASSERT_EQ(0,nglMsgQDestroy(q));
}

TEST_F(OSMSGQ,Send_Recv_1){
   int i,msgs[]={1,2,3,4,5,6,7,8};
   HANDLE q=nglMsgQCreate(sizeof(msgs)/sizeof(int),sizeof(int));
   for(i=0;i<sizeof(msgs)/sizeof(int);i++)
       ASSERT_EQ(0,nglMsgQSend(q,&msgs[i],sizeof(int),1000));
   for(i=0;i<sizeof(msgs)/sizeof(int);i++){
       int msg;
       ASSERT_EQ(0,nglMsgQReceive(q,&msg,sizeof(int),1000));
       ASSERT_EQ(msg,i+1);
   }
}
static void SendProc(void*p){
   TESTQUEUE*dt=(TESTQUEUE*)p;
   uint32_t msg;
   uint32_t *msgs=dt->msgs;
   int i=0;
   while(msgs[i]){
       ASSERT_EQ(0,nglMsgQSend(dt->q,&msgs[i],sizeof(uint32_t),1000));
       i++;
       usleep(dt->snd_delay);
   }
}
static void RecvProc(void*p){
   TESTQUEUE*dt=(TESTQUEUE*)p;
   uint32_t msg;
   uint32_t *msgs=dt->msgs;
   int i=0;
   while(msgs[i]){
      ASSERT_EQ(0,nglMsgQReceive(dt->q,&msg,sizeof(uint32_t),1000));
      ASSERT_EQ(msg,msgs[i]);
      i++;
      usleep(dt->rcv_delay);
   }
}

TEST_F(OSMSGQ,Send_Recv_2){//slow recv & fast send
   void* tid_snd,*tid_rcv;
   uint32_t msgs[]={100,200,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,0};
   TESTQUEUE dt;
   dt.msgs=msgs;
   dt.q=nglMsgQCreate(6,sizeof(uint32_t));
   dt.rcv_delay=200;dt.snd_delay=100;
   nglCreateThread(&tid_snd,0,0,SendProc,(void*)&dt);
   nglCreateThread(&tid_rcv,0,0,RecvProc,(void*)&dt);
   sleep(2);
}

TEST_F(OSMSGQ,Send_Recv_3){//fast recv & slow send
   void* tid_snd,*tid_rcv;
   TESTQUEUE dt;
   uint32_t i,msgs[]={200,100,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,0};
   dt.q=nglMsgQCreate(6,sizeof(uint32_t));
   dt.msgs=msgs;
   dt.rcv_delay=100;dt.snd_delay=200;
   
   nglCreateThread(&tid_snd,0,0,SendProc,(void*)&dt);
   nglCreateThread(&tid_rcv,0,0,RecvProc,(void*)&dt);
   sleep(2);
}
#endif
