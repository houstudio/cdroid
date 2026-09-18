#include <gtest/gtest.h>
#include <cdroid.h>
#include <core/systemclock.h>
#include <cdlog.h>
#include <functional>
#include <thread>
#include <core/uieventsource.h>
#include <core/handler.h>
#if defined(__linux__)||defined(__unix__)
#include <sys/time.h>
#include <sys/timerfd.h>
#include <unistd.h>
#else
extern void sleep(uint32_t);
extern void usleep(uint32_t);
#endif
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

class TestHandler:public MessageHandler{
   int count;
public:
   TestHandler(){count=0;}
   void handleMessage(Message&msg)override{
       count++;printf("handleMessage(%d)\r\n",msg.what);
   }
   int getCount()const{return count;}
};

// pollonce dropped: it polled the SHARED main looper, which Choreographer frame
// messages wake early — flaky by construction. The official private-looper port
// (looper_libutils_tests.cc LooperTest.PollOnce_WhenNonZeroTimeoutAndNotAwoken_
// WaitsForTimeout) covers the same assertion deterministically.
// sendMessage / sendMessageDelayed / removeMessages coverage moved to CtsHandlerTest
// (handler_tests.cc) and the official LooperTest (looper_libutils_tests.cc);
// this file keeps the CDROID-specific surfaces (handler/eventhandler removal,
// async messages, timerfd fds).

class SelfDestroyHandler:public MessageHandler{
private:
    Looper*mLooper;
public:
   static int Count;
   SelfDestroyHandler(Looper*lp,bool ownedByLooper=false){
       mLooper=lp;
       setOwned(ownedByLooper);
       Count++;
   }
   ~SelfDestroyHandler()override{
       Count--;
       std::cout<<"destroy SelfDestroyHandler:"<<this<<std::endl;
   }
   void handleMessage(Message&msg)override{
       mLooper->removeHandler(this);
       printf("removeHandler %p\r\n",this);
   }
};
int SelfDestroyHandler::Count=0;
class SelfDestroyEventHandler:public EventHandler{
private:
    Looper*mLooper;
public:
    static int Count;
    SelfDestroyEventHandler(Looper*lp,bool ownedByLooper=false){
        mLooper=lp;
        Count++;
        setOwned(ownedByLooper);
    }
    ~SelfDestroyEventHandler()override{
        Count--;
        std::cout<<"destroy SelfDestroyEventHandler:"<<this<<std::endl;
    }
    int checkEvents()override{return 1;};
    int handleEvents()override{
        mLooper->removeEventHandler(this);
        return 1;
    }
};
int SelfDestroyEventHandler::Count=0;
TEST_F(LOOPER,removeHandler){
    Message*msg=Message::obtain(); msg->what=100;
    SelfDestroyHandler*sd = new SelfDestroyHandler(mLooper,true);
    SelfDestroyHandler*sd2= new SelfDestroyHandler(mLooper,false);
    SelfDestroyEventHandler*se= new SelfDestroyEventHandler(mLooper,true);
    SelfDestroyEventHandler*se2=new SelfDestroyEventHandler(mLooper,false);
    mLooper->addHandler(sd);
    mLooper->addHandler(sd2);
    mLooper->addEventHandler(se);
    mLooper->addEventHandler(se2);
    EXPECT_EQ(SelfDestroyHandler::Count,2);
    EXPECT_EQ(SelfDestroyEventHandler::Count,2);
    printf("HANDLE:%p ,%p  EventHandler:%p ,%p\r\n",sd,sd2,se,se2);

    /* 500ms delay: the removal EXPECTs below must not ride the 1ms-truncation
       edge of the old 10ms delay — two pollOnce(100) could leave the messages a
       fraction short of due, the deletes would then strand due envelopes, and the
       listener's pump would dispatch them into freed handlers. */
    mLooper->sendMessageDelayed(500,sd,*msg);
    mLooper->sendMessageDelayed(500,sd2,*msg);
    msg->recycle();
    mLooper->pollOnce(50);

    mLooper->removeHandler(sd);      // looper-owned: flagged, freed by a later walk
    EXPECT_EQ(SelfDestroyHandler::Count,2);
    mLooper->removeHandler(sd2);     // app-owned: unlinked at once
    mLooper->removeMessages(sd);     // ~Handler discipline: no envelope may outlive
    mLooper->removeMessages(sd2);    // its handler
    delete sd2;
    EXPECT_EQ(SelfDestroyHandler::Count,1);
    mLooper->removeEventHandler(se);
    EXPECT_EQ(SelfDestroyEventHandler::Count,1);
    mLooper->removeEventHandler(se2);
    delete se2;
    EXPECT_EQ(SelfDestroyEventHandler::Count,0);

    /* Dispatch-path self-removal: handleMessage() -> removeHandler(this) is why
       removeHandler defers the delete of owned handlers. */
    SelfDestroyHandler*sd3 = new SelfDestroyHandler(mLooper,true);
    mLooper->addHandler(sd3);
    Message m3; m3.what=100;
    mLooper->sendMessageDelayed(20,sd3,m3);
    mLooper->pollOnce(100);          // returns on the enqueue wake, message not yet due
    mLooper->pollOnce(100);          // waits out the delay, dispatches, walk frees sd3
    EXPECT_EQ(SelfDestroyHandler::Count,0);
    EXPECT_EQ(SelfDestroyEventHandler::Count,0);
    printf("#### END ####\r\n\r\n");
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
    // View::post/postDelayed 的延迟 runnable 现经 AttachInfo::mHandler (主 looper MessageQueue) 派发;
    // UIEventSource 已退回纯帧驱动, 不再持有 runnable 队列。本例改测 Handler 路径的身份比对语义。
    Handler*handler=new Handler(Looper::getMainLooper());
    Runnable run([]{});
    handler->postDelayed(run,10);
    ASSERT_TRUE(handler->hasCallbacks(run));
    handler->removeCallbacks(run);
    ASSERT_FALSE(handler->hasCallbacks(run));

    // Runnable 副本共享底层 functor (CallbackBase::operator== 比指针), 故可用任一副本删除
    Runnable run2(run);
    Runnable run3;
    run3=run;
    handler->postDelayed(run2,10);
    ASSERT_TRUE(handler->hasCallbacks(run2));
    handler->removeCallbacks(run2);
    ASSERT_FALSE(handler->hasCallbacks(run2));

    handler->postDelayed(run,10);
    ASSERT_TRUE(handler->hasCallbacks(run3));   // run3 与 run 同身份
    handler->removeCallbacks(run3);
    ASSERT_FALSE(handler->hasCallbacks(run3));

    ASSERT_FALSE(handler->hasCallbacks(run3));  // 已删, 再查为假
    delete handler;
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
    ASSERT_EQ(handler->count,2);   // assert BEFORE delete — reading a freed member is UB
    delete handler;
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
#if defined(__linux__)||defined(__unix__)
static int fdcallback(int fd, int events, void* data){
   struct timespec cur;
   int *loops=(int*)data;
   clock_gettime(CLOCK_MONOTONIC,&cur);
   if(events&Looper::EVENT_INPUT){
       uint64_t count=0;
       if(::read(fd, &count, sizeof(uint64_t))>0){
          (*loops)+=count;
          LOGD("loops +%d = %d",count,*loops);
       }
   }
   if(*loops>=20){
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

    while(loops<20)mLooper->pollAll(10);
    ASSERT_EQ(loops,20);
}
#endif

