#include <gtest/gtest.h>
#include <cdroid.h>
#include <sys/time.h>
#include <porting/cdinput.h>
#include <linux/input.h>
struct MTEvent{int type,code,value;};
class EVENT:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(EVENT,Coords){
   for(int i=0;i<100;i++){
       PointerCoords *p=new PointerCoords;
       delete p;
   }
}
TEST_F(EVENT,Alloc_Free){
   PointerCoords coords[128];
   PointerCoords*pc=coords;
   PointerProperties ptprops[128];
   for(int i=0;i<100;i++){
      MotionEvent*m=new MotionEvent();
      delete m;
   }
   for(int i=0;i<100;i++){
      MotionEvent*m=new MotionEvent();
      m->initialize(0,0,0,0,0,0,0,0,0,.0f,.0f,.0f,.0f,0,0,i%32,ptprops,coords);
      delete m;
   }
}

TEST_F(EVENT,EventPool){
    PooledInputEventFactory pool(32);
    PointerCoords coords[128];
    PointerCoords*pc=coords;
    PointerProperties ptprops[128];
    for(int i=0;i<10000;i++){
        MotionEvent*m=pool.createMotionEvent();
        m->initialize(0,0,0,0,0,0,0,0,0,.0f,.0f,.0f,.0f,0,0,i%32,ptprops,coords);
        pool.recycle(m);
    }
}
TEST_F(EVENT,Benchmark){
    PooledInputEventFactory pool(32);
    PointerCoords coords[128];
    PointerCoords*pc=coords;
    PointerProperties ptprops[128];
    struct timeval tv1,tv2;

    gettimeofday(&tv1,NULL);
    for(int i=0;i<100000;i++){
        MotionEvent*m=new MotionEvent();
        delete m;
    }
    gettimeofday(&tv2,NULL);
    printf("NoPool usedtime=%ld\r\n",1000L*tv2.tv_sec+tv2.tv_usec/1000-1000L*tv1.tv_sec-tv1.tv_usec/1000);

    gettimeofday(&tv1,NULL);
    for(int i=0;i<100000;i++){
        MotionEvent*m=pool.createMotionEvent();
        pool.recycle(m);
    }
    gettimeofday(&tv2,NULL);
    printf("EventPool usedtime=%ld\r\n",1000L*tv2.tv_sec+tv2.tv_usec/1000-1000L*tv1.tv_sec-tv1.tv_usec/1000);
}


TEST_F(EVENT,exec){
   static const char*args[]={"arg1","alpha",NULL};
   App app(2,args);
   app.exec();
}
