#include <gtest/gtest.h>
#include <cdroid.h>
#include <ngl_os.h>
#include <sys/time.h>
#include <core/systemclock.h>
#include <cdlog.h>
#include <functional>
#include <core/uieventsource.h>

class LOOPER:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
      printf("\r\n\r\n");
   }
};

class MyRunner{
public:
   int key;
   std::function<void()>run;
   MyRunner(){}
   MyRunner(const std::function<void()>a){}
   void operator=(const std::function<void()>&a){
       run=a;
   }
   void test(int i){
       printf("i=%d\r\n",i);
   }
   virtual void post(Runnable&){
      printf("post(Runnable&)\r\n");
   }
   void post(const Runnable&a){
      printf("post(const Runnable&)\r\n");
      Runnable aa=a;
      post(aa);   
   }
   void post(const std::function<void()>&a){
      printf("post(const const std::function<void()>&)\r\n");
      Runnable aaa;
      aaa=a;
      post(aaa);
   }
};
class YouRunner:public MyRunner{
public:
   YouRunner():MyRunner(){}
   YouRunner(const std::function<void()>a):MyRunner(a){};
};
TEST_F(LOOPER,function){
   YouRunner r(std::bind(&LOOPER::SetUp,this));
   Runnable rrr;
   r.post(Runnable([](){}));
   r.post(Runnable([](){}));
   r.post(std::bind(&LOOPER::SetUp,this));
   typedef std::function<void()>aaaa;
}

class TestHandler:public MessageHandler{
   int count;
public:
   TestHandler(){count=0;}
   void handleMessage(Message&msg)override{
       count++;
   }
   int getCount()const{return count;}
};

TEST_F(LOOPER,pollonce){
   Looper loop(false);
   int64_t t1=SystemClock::uptimeMillis();
   loop.pollOnce(1000);
   int64_t t2=SystemClock::uptimeMillis();
   ASSERT_TRUE((t2-t1)>=1000&&(t2-t2)<1005);
}

TEST_F(LOOPER,sendMessage){
   Looper loop(false);
   Message msg(100);
   int  processed=0;
   TestHandler ft;
   loop.sendMessage(&ft,msg);
   loop.pollOnce(10);
   ASSERT_EQ(ft.getCount(),1);
}

TEST_F(LOOPER,sendMessageDelay){
   Looper loop(false);
   Message msg(100);
   TestHandler ft;
   int64_t t1=SystemClock::uptimeMillis();
   loop.sendMessageDelayed(1000,&ft,msg);
   while(!ft.getCount()) loop.pollOnce(10);
   int64_t t2=SystemClock::uptimeMillis();
   ASSERT_TRUE((t2-t1)>=1000&&(t2-t2)<1005);
}

TEST_F(LOOPER,removeMessage){
   Looper loop(false);
   Message msg(100),msg2(200);
   TestHandler ft;
   int64_t t2,t1=SystemClock::uptimeMillis();
   loop.sendMessageDelayed(1000,&ft,msg);
   loop.sendMessageDelayed(1000,&ft,msg2);
   t2=t1;
   loop.removeMessages(&ft,100);
   while(t2-t1<1100){
       loop.pollOnce(10);
       t2=SystemClock::uptimeMillis();
   }
   ASSERT_EQ(ft.getCount(),1);
}

TEST_F(LOOPER,eventhandler){
   
}
TEST_F(LOOPER,loop){
   Looper loop(false);
   UIEventSource*handler=new UIEventSource(nullptr,nullptr);
   loop.addEventHandler(handler);
   Runnable run;
   int count=0;
   run=[&](){
       printf("count=%d\r\n",count++);
       handler->post(run,count++);
   };
   handler->post(run,10);
   while(1)loop.pollAll(100);
}
