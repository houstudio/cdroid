#include <gtest/gtest.h>
#include <cdroid.h>
#include <ngl_os.h>
#include <sys/time.h>
#include <core/systemclock.h>
#include <cdlog.h>
#include <functional>
#include <thread>
#include <core/uieventsource.h>
#include <core/handler.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <time.h>

class LOOPER:public testing::Test{
protected:
    static Looper*mLooper;
public :
    static void SetUpTestCase(){
        Looper::prepare(false);
        mLooper=Looper::getMainLooper();
        if(mLooper==nullptr)Looper::prepareMainLooper();
        mLooper=Looper::myLooper();
    }
    static void TearDownCase(){
    }
    virtual void SetUp(){
    }
    virtual void TearDown(){
       LOGD("\r\n\r\n");
    }
};
Looper*LOOPER::mLooper=nullptr;

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
       count++;printf("handleMessage(%d)\r\n",msg.what);
   }
   int getCount()const{return count;}
};

TEST_F(LOOPER,pollonce){
   int64_t t1=SystemClock::uptimeMillis();
   mLooper->pollOnce(1000);
   int64_t t2=SystemClock::uptimeMillis();
   ASSERT_TRUE((t2-t1)>=1000&&(t2-t2)<1005);
}

TEST_F(LOOPER,sendMessage){
   Message msg(100);
   int  processed=0;
   TestHandler ft;
   mLooper->sendMessage(&ft,msg);
   mLooper->pollOnce(10);
   ASSERT_EQ(ft.getCount(),1);
}

TEST_F(LOOPER,sendMessageDelay){
   Message msg(100);
   TestHandler ft;
   int64_t t1=SystemClock::uptimeMillis();
   mLooper->sendMessageDelayed(1000,&ft,msg);
   while(!ft.getCount()) mLooper->pollOnce(10);
   int64_t t2=SystemClock::uptimeMillis();
   ASSERT_TRUE((t2-t1)>=1000&&(t2-t2)<1005);
}

TEST_F(LOOPER,removeMessage){
   Message msg(100),msg2(200);
   TestHandler ft;
   int64_t t2,t1=SystemClock::uptimeMillis();
   mLooper->sendMessageDelayed(1000,&ft,msg);
   mLooper->sendMessageDelayed(1000,&ft,msg2);
   t2=t1;
   mLooper->removeMessages(&ft,100);
   while(t2-t1<1100){
       mLooper->pollOnce(10);
       t2=SystemClock::uptimeMillis();
   }
   ASSERT_EQ(ft.getCount(),1);
}
class SelfDestroyHandler:public MessageHandler{
private:
    Looper*mLooper;
public:
   SelfDestroyHandler(Looper*lp){mLooper=lp;}
   void handleMessage(Message&msg)override{
       mLooper->removeMessages(this);
   }
};
class SelfDestroyEventHandler:public EventHandler{
private:
    Looper*mLooper;
public:
    SelfDestroyEventHandler(Looper*lp){mLooper=lp;}
    int checkEvents()override{return 1;};
    int handleEvents()override{
        mLooper->removeEventHandler(this);
        return 1;
    }
};
TEST_F(LOOPER,removeHandler){
    SelfDestroyHandler*sd=new SelfDestroyHandler(mLooper);
    SelfDestroyEventHandler*se=new SelfDestroyEventHandler(mLooper);
    Message msg(100);
    mLooper->addEventHandler(se);
    mLooper->sendMessageDelayed(10,sd,msg);

    mLooper->pollOnce(100);
    LOGD("===");
}

class TestRunner:public Runnable{
private:
   int mCount;
public:
   TestRunner():Runnable(){
      mCount=0;
      (*mFunctor)=std::bind(&TestRunner::doit,this);
   }
   void doit(){
       mCount++;
       LOGD("mCount=%d",mCount);
   }
};
TEST_F(LOOPER,eventhandler){
    UIEventSource*handler=new UIEventSource(nullptr,nullptr);
    Runnable run([]{});
    handler->postDelayed(run,10);
    bool rc=handler->removeCallbacks(run);
    ASSERT_TRUE(rc);

    Runnable run2(run);
    Runnable run3;
    run3=run;
    handler->postDelayed(run2,10);
    rc=handler->removeCallbacks(run2);
    ASSERT_TRUE(rc);

    handler->postDelayed(run,10);
    rc=handler->removeCallbacks(run3);
    ASSERT_TRUE(rc);

    rc=handler->removeCallbacks(run3);
    ASSERT_FALSE(rc);
}

class MyHandler:public Handler{
public:
    int count=0;
    void handleMessage(Message&msg)override{
        count++;
    }
    void handleIdle(){
    }
    ~MyHandler(){
        LOGD("MyHandler destroied!");
    }
};

TEST_F(LOOPER,handler){
    Looper *loop= Looper::getMainLooper();
    MyHandler *handler=new MyHandler();
    handler->sendEmptyMessage(1);
    handler->sendEmptyMessageDelayed(2,20);
    Runnable cbk([](){LOGD("---");});
    handler->postDelayed(cbk,30);
    int count=0;
    while(count++<3)
        mLooper->pollAll(10);
    mLooper->removeHandler(handler);
    while(count++<6)mLooper->pollAll(10);
    delete handler;
    ASSERT_EQ(handler->count,2);
}

TEST_F(LOOPER,asyncmsg){
    MyHandler *handler=new MyHandler();
    handler->sendEmptyMessage(0);
    handler->sendEmptyMessageDelayed(0,20);
    Runnable cbk([](){LOGD("---");});
    handler->postDelayed(cbk,30);
    Message msg;
    std::thread th([&](){
        msg.what=0;
        while(msg.what++<10000){
           mLooper->sendMessageDelayed(10,handler,msg);
           usleep(100);
        }
    });
    th.detach();
    int count=0;
    while(count++<200)mLooper->pollAll(10);
    ASSERT_EQ(handler->count,10002);
}

static void ms2timespec(int ms, struct timespec *ts){
    ts->tv_sec = ms / 1000;
    ts->tv_nsec = (ms % 1000) * 1000000;
}

static int fdcallback(int fd, int events, void* data){
   uint64_t count;
   int *loops=(int*)data;
   struct timespec cur;
   clock_gettime(CLOCK_MONOTONIC,&cur);
   if(events&Looper::EVENT_INPUT)
       ::read(fd, &count, sizeof(uint64_t));
   if(*loops>20){
       struct itimerspec new_value={{0,0},{0,0}};
       timerfd_settime(fd,0,&new_value, NULL);
       Looper::getMainLooper()->removeFd(fd);
   }
   return 1;
}

TEST_F(LOOPER,timerfd){
    #define INTERVAL 200 //ms
    int loops=0;
    struct itimerspec new_value={{0,0},{0,0}};
    ms2timespec(INTERVAL,&new_value.it_value);
    ms2timespec(INTERVAL,&new_value.it_interval);
    int fd=timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

    int rc=timerfd_settime(fd, 0/*TFD_TIMER_ABSTIME*/, &new_value,NULL);
    mLooper->addFd(fd,0,Looper::EVENT_INPUT,fdcallback,&loops);

    while(1)mLooper->pollAll(10);
}

TEST_F(LOOPER,timerfd2){
    #define INTERVAL 200 //ms
    App app;
    int loops=0;
    Looper*loop= Looper::getMainLooper();
    struct itimerspec new_value={{0,0},{0,0}};

    ms2timespec(INTERVAL,&new_value.it_value);
    ms2timespec(INTERVAL,&new_value.it_interval);
    int fd=timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

    int rc=timerfd_settime(fd, 0/*TFD_TIMER_ABSTIME*/, &new_value, NULL);
    loop->addFd(fd,0,Looper::EVENT_INPUT,fdcallback,&loops);
    Window*w  = new Window(0,0,-1,-1);
    Button*btn= new Button("Test",200,200);
    w->addView(btn);
    int color=11,color2=0;
    Runnable run={[&](){
        btn->setBackgroundColor(0xFF000000|color|(color2<<8));
        color+=8;color2+=4;
        w->postDelayed(run,100);
    }};
    w->postDelayed(run,100);
    btn->setOnClickListener([](View&v){
        printf("button clicked \r\n");
    });
    app.exec();
}
