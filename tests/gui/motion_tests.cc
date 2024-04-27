#include <gtest/gtest.h>
#include <cdroid.h>
#include <ngl_os.h>
#include <sys/time.h>
#include <porting/cdinput.h>
#include <linux/input.h>

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

TEST_F(EVENT,MT){
   TouchDevice d(INJECTDEV_TOUCH);
   struct MTEvent{int type,code,value;};
   MTEvent mts[]={
     {EV_KEY,BTN_TOUCH,1},          //ACTION_DOWN
     {EV_ABS,ABS_MT_TRACKING_ID,45},
     {EV_ABS,ABS_MT_POSITION_X ,10},
     {EV_ABS,ABS_MT_POSITION_Y ,20},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_MOVE
     {EV_ABS,ABS_MT_POSITION_X ,14},
     {EV_ABS,ABS_MT_POSITION_Y ,20},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},
     
     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_POINTER_DOWN finger 0
     {EV_ABS,ABS_MT_POSITION_X ,14},
     {EV_ABS,ABS_MT_POSITION_Y ,20},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_TRACKING_ID,46},//ACTION_POINTER_DOWN finger 1
     {EV_ABS,ABS_MT_POSITION_X ,10},
     {EV_ABS,ABS_MT_POSITION_Y ,200},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_POINTER_DOWN finger 0
     {EV_ABS,ABS_MT_POSITION_X ,14},
     {EV_ABS,ABS_MT_POSITION_Y ,20},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_TRACKING_ID,46},//ACTION_POINTER_DOWN finger 1
     {EV_ABS,ABS_MT_POSITION_X ,10},
     {EV_ABS,ABS_MT_POSITION_Y ,200},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_TRACKING_ID,47},//ACTION_POINTER_DOWN finger 2
     {EV_ABS,ABS_MT_POSITION_X ,200},
     {EV_ABS,ABS_MT_POSITION_Y ,300},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_POINTER_UP finger 2
     {EV_ABS,ABS_MT_POSITION_X ,16},
     {EV_ABS,ABS_MT_POSITION_Y ,20},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_TRACKING_ID,46},
     {EV_ABS,ABS_MT_POSITION_X ,6},
     {EV_ABS,ABS_MT_POSITION_Y ,200},
     {EV_SYN,SYN_MT_REPORT,0},
     //{EV_ABS,ABS_MT_TRACKING_ID,-1},
     {EV_SYN,SYN_REPORT,0},
#if 10//2 moveevents
     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_POINTER_UP finger 2
     {EV_ABS,ABS_MT_POSITION_X ,18},
     {EV_ABS,ABS_MT_POSITION_Y ,20},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_TRACKING_ID,46},
     {EV_ABS,ABS_MT_POSITION_X ,8},
     {EV_ABS,ABS_MT_POSITION_Y ,200},
     {EV_SYN,SYN_MT_REPORT,0},
     //{EV_ABS,ABS_MT_TRACKING_ID,-1},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_POINTER_UP finger 2
     {EV_ABS,ABS_MT_POSITION_X ,19},
     {EV_ABS,ABS_MT_POSITION_Y ,20},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_TRACKING_ID,46},
     {EV_ABS,ABS_MT_POSITION_X ,10},
     {EV_ABS,ABS_MT_POSITION_Y ,200},
     {EV_SYN,SYN_MT_REPORT,0},
     //{EV_ABS,ABS_MT_TRACKING_ID,-1},
     {EV_SYN,SYN_REPORT,0},
#endif
     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_POINTER_UP finger 1
     {EV_ABS,ABS_MT_POSITION_X ,18},
     {EV_ABS,ABS_MT_POSITION_Y ,20},
     {EV_SYN,SYN_MT_REPORT,0},
     //{EV_ABS,ABS_MT_TRACKING_ID,-1},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_TRACKING_ID,-1},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},
   };
   for(int i=0;i<sizeof(mts)/sizeof(MTEvent);i++){
       int32_t tmEVT = i*20*100000;
       d.putRawEvent({0,tmEVT},mts[i].type,mts[i].code,mts[i].value);
   }
   for(int i=0;i<d.getEventCount();i++){
       MotionEvent*e=(MotionEvent*)d.popEvent();
       const int pointerCount=e->getPointerCount();
       const int hisCount = e->getHistorySize();
       LOGI("Event[%d].Action=%d pointers=%d history=%d",i,e->getActionMasked(),pointerCount,hisCount);
       for(int j=0;j<pointerCount;j++){
	  std::ostringstream oss;
	  for(int k=0;k<hisCount;k++){
	     oss<<"["<<e->getHistoricalRawX(j,k)<<","<<e->getHistoricalRawY(j,k)<<"],";
	  }
          LOGI("     Point[%d](%d)=(%.f,%.f)[%s]",j,e->getPointerId(j),e->getX(j),e->getY(j),oss.str().c_str());
       }
   }
}

TEST_F(EVENT,exec){
   static const char*args[]={"arg1","alpha",NULL};
   App app(2,args);
   app.exec();
}
