#include <gtest/gtest.h>
#include <cdroid.h>
#include <ngl_os.h>
#include <sys/time.h>
#include <core/inputeventlabels.h>
#include <core/eventcodes.h>

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
   TouchDevice d(-1);
   struct MTEvent{int type,code,value;};
   MTEvent mts[]={
     {EV_ABS,ABS_MT_TRACKING_ID,45},
     {EV_ABS,ABS_MT_POSITION_X ,10},
     {EV_ABS,ABS_MT_POSITION_X ,20},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_TRACKING_ID,45},
     {EV_ABS,ABS_MT_POSITION_X ,14},
     {EV_ABS,ABS_MT_POSITION_X ,20},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},
     
     {EV_ABS,ABS_MT_TRACKING_ID,45},
     {EV_ABS,ABS_MT_POSITION_X ,14},
     {EV_ABS,ABS_MT_POSITION_X ,20},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_TRACKING_ID,46},
     {EV_ABS,ABS_MT_POSITION_X ,100},
     {EV_ABS,ABS_MT_POSITION_X ,200},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_TRACKING_ID,45},
     {EV_ABS,ABS_MT_POSITION_X ,14},
     {EV_ABS,ABS_MT_POSITION_X ,20},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_TRACKING_ID,46},
     {EV_ABS,ABS_MT_POSITION_X ,100},
     {EV_ABS,ABS_MT_POSITION_X ,200},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_TRACKING_ID,47},
     {EV_ABS,ABS_MT_POSITION_X ,200},
     {EV_ABS,ABS_MT_POSITION_X ,300},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_TRACKING_ID,45},
     {EV_ABS,ABS_MT_POSITION_X ,16},
     {EV_ABS,ABS_MT_POSITION_X ,20},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_TRACKING_ID,46},
     {EV_ABS,ABS_MT_POSITION_X ,106},
     {EV_ABS,ABS_MT_POSITION_X ,200},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_TRACKING_ID,-1},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_TRACKING_ID,45},
     {EV_ABS,ABS_MT_POSITION_X ,14},
     {EV_ABS,ABS_MT_POSITION_X ,20},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_TRACKING_ID,-1},
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
}

TEST_F(EVENT,keylabel){
 
    for(int i=0;i<sizeof(KEYCODES)/sizeof(KEYCODES[0])-1;i++){
        const char*label=getLabelByKeyCode(KEYCODES[i].value);
        printf("%d:%s:%d\r\n",i,label,KEYCODES[i].value);
        EXPECT_STRCASEEQ(label,KEYCODES[i].literal);
        ASSERT_EQ(KeyEvent::getKeyCodeFromLabel(label),KEYCODES[i].value);
    }
}
TEST_F(EVENT,exec){
   static const char*args[]={"arg1","alpha",NULL};
   App app(2,args);
   app.exec();
}
