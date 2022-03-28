#include <gtest/gtest.h>
#include <cdroid.h>
#include <ngl_os.h>
#include <sys/time.h>
#include <core/systemclock.h>
#include <cdlog.h>
#include <functional>
#include <core/uieventsource.h>
#include <core/handler.h>

class LOOPER:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
      LOGD("\r\n\r\n");
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
       LOGD("i=%d",i);
   }
   virtual void post(Runnable&){
      LOGD("post(Runnable&)");
   }
   void post(const Runnable&a){
      LOGD("post(const Runnable&)");
      Runnable aa=a;
      post(aa);   
   }
   void post(const std::function<void()>&a){
      LOGD("post(const const std::function<void()>&)");
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
/*TEST_F(LOOPER,loop){
   Looper loop(false);
   UIEventSource*handler=new UIEventSource(nullptr,nullptr);
   loop.addEventHandler(handler);
   Runnable run;
   int count=0;
   run=[&](){
       LOGD("count=%d",count++);
       handler->post(run,count++);
   };
   handler->post(run,10);
   while(1)loop.pollAll(100);
}*/

class MyHandler:public Handler{
public:
    void handleMessage(Message&msg)override{
        LOGD("rcv msg %d",msg.what);
    }
    void handleIdle(){
        LOGD("idle");
    }
    ~MyHandler(){
        LOGD("MyHandler destroied!");
    }
};

TEST_F(LOOPER,handler){
    Looper *loop= Looper::getDefault();
    Handler *handler=new MyHandler();
    handler->sendEmptyMessage(1);
    handler->sendEmptyMessageDelayed(2,20);
    Runnable cbk([](){LOGD("---");});
    handler->postDelayed(cbk,30);
    int count=0;
    while(count++<3)
        loop->pollAll(10);
    loop->removeHandler(handler);
    while(count++<6)loop->pollAll(10);
}

