#include <gtest/gtest.h>
#include <cdroid.h>
#include <porting/cdinput.h>
#if defined(__linux__)||defined(__unix__) 
#include <sys/time.h>
#include <linux/input.h>
#else
#include <core/eventcodes.h>
#endif

struct MTEvent{int type,code,value;};
class INPUTDEVICE:public testing::Test{
public:
    int EventCount;
    MotionEvent*OutEvents[128];
public:
   virtual void SetUp(){
   }
   virtual void TearDown(){
      for(int i=0;i<EventCount;i++){
         OutEvents[i]->recycle();
      }
   }
   int sendEvents(InputDevice&d,MTEvent*mts,int size,MotionEvent**eventOut){
      int eventCount = 0;
      int32_t tmEVT = 0;
      for(int i=0;i<size;i++){
         d.putEvent(0,tmEVT,mts[i].type,mts[i].code,mts[i].value);
         if((mts[i].type==EV_SYN)&&(mts[i].code==SYN_REPORT))
             tmEVT+=200*1000000;
      }
      eventCount = d.getEventCount();
      LOGI("%d Events",eventCount);
      std::vector<InputEvent*>events;
      d.drainEvents(events);
      for(int i=0;d.getEventCount();i++){
          MotionEvent*e=(MotionEvent*)events.at(i);
          const int pointerCount=e->getPointerCount();
          const int hisCount = e->getHistorySize();
          if(eventOut)eventOut[i] = e;
          LOGI("Event[%d].Action=%d/%d pointers=%d history=%d",i,e->getActionMasked(),e->getActionIndex(),pointerCount,hisCount);
          for(int j=0;j<pointerCount;j++){
             std::ostringstream oss;
             for(int k=0;k<hisCount;k++){
                oss<<"("<<e->getHistoricalX(j,k)<<","<<e->getHistoricalY(j,k)<<"),";
             }
             LOGI("     Point[%d](%d)=(%.f,%.f){%s}",j,e->getPointerId(j),e->getX(j),e->getY(j),oss.str().c_str());
          }
      }
      return eventCount;
   }
};

TEST_F(INPUTDEVICE,ST){
    TouchDevice d(INJECTDEV_TOUCH);
    MTEvent mts[]={
        {EV_KEY,BTN_TOUCH,1},//0
        {EV_ABS,ABS_X,10},
        {EV_ABS,ABS_Y,20},
        {EV_SYN,SYN_REPORT,0},

        {EV_ABS,ABS_X,12},//4
        {EV_ABS,ABS_Y,22},//5
        {EV_SYN,SYN_REPORT,0},

        {EV_ABS,ABS_X,14},//7
        {EV_ABS,ABS_Y,24},
        {EV_SYN,SYN_REPORT,0},

        {EV_KEY,BTN_TOUCH,0},//10
        {EV_ABS,ABS_X,16},
        {EV_ABS,ABS_Y,26},
        {EV_SYN,SYN_REPORT,0},
    };
   EventCount = sendEvents(d,mts,sizeof(mts)/sizeof(MTEvent),OutEvents);
   ASSERT_EQ(EventCount,4);
   ASSERT_EQ(OutEvents[0]->getAction(),MotionEvent::ACTION_DOWN);
   ASSERT_EQ(OutEvents[0]->getPointerId(0),0);
   ASSERT_EQ(OutEvents[0]->getPointerCount(),1);
   ASSERT_EQ(OutEvents[0]->getX(0),mts[1].value);
   ASSERT_EQ(OutEvents[0]->getY(0),mts[2].value);

   ASSERT_EQ(OutEvents[1]->getAction(),MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[1]->getX(0),mts[4].value);
   ASSERT_EQ(OutEvents[1]->getY(0),mts[5].value);

   ASSERT_EQ(OutEvents[2]->getAction(),MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[2]->getX(0),mts[7].value);
   ASSERT_EQ(OutEvents[2]->getY(0),mts[8].value);

   ASSERT_EQ(OutEvents[3]->getAction(),MotionEvent::ACTION_UP);
   ASSERT_EQ(OutEvents[3]->getX(0),mts[11].value);
   ASSERT_EQ(OutEvents[3]->getY(0),mts[12].value);
}

TEST_F(INPUTDEVICE,ST2){
    TouchDevice d(INJECTDEV_TOUCH);
    MTEvent mts[]={
        {EV_KEY,BTN_TOUCH,1},//0
        {EV_ABS,ABS_X,10},
        {EV_ABS,ABS_Y,20},
        {EV_SYN,SYN_REPORT,0},

        {EV_ABS,ABS_Z,12},//4,/*NOX means use th last pointer's X value*/
        {EV_ABS,ABS_Y,22},//5
        {EV_SYN,SYN_REPORT,0},

        {EV_ABS,ABS_X,14},//7
        {EV_ABS,ABS_Y,24},
        {EV_SYN,SYN_REPORT,0},

        {EV_KEY,BTN_TOUCH,0},//10
        {EV_ABS,ABS_X,16},
        {EV_ABS,ABS_Y,26},
        {EV_SYN,SYN_REPORT,0},

        {EV_KEY,BTN_TOUCH,1},//14
        {EV_ABS,ABS_X,10},
        {EV_ABS,ABS_Y,20},
        {EV_SYN,SYN_REPORT,0},

        {EV_KEY,BTN_TOUCH,0},//18
        {EV_ABS,ABS_X,16},
        {EV_ABS,ABS_Y,26},
        {EV_SYN,SYN_REPORT,0},
    };
   EventCount = sendEvents(d,mts,sizeof(mts)/sizeof(MTEvent),OutEvents);
   ASSERT_EQ(EventCount,6);
   ASSERT_EQ(OutEvents[0]->getAction(),MotionEvent::ACTION_DOWN);
   ASSERT_EQ(OutEvents[0]->getPointerId(0),0);
   ASSERT_EQ(OutEvents[0]->getPointerCount(),1);
   ASSERT_EQ(OutEvents[0]->getX(0),mts[1].value);
   ASSERT_EQ(OutEvents[0]->getY(0),mts[2].value);

   ASSERT_EQ(OutEvents[1]->getAction(),MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[1]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[1]->getPointerId(0),0);
   ASSERT_EQ(OutEvents[1]->getX(0),mts[1].value);
   ASSERT_EQ(OutEvents[1]->getY(0),mts[5].value);

   ASSERT_EQ(OutEvents[2]->getAction(),MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[2]->getPointerId(0),0);
   ASSERT_EQ(OutEvents[2]->getX(0),mts[7].value);
   ASSERT_EQ(OutEvents[2]->getY(0),mts[8].value);

   ASSERT_EQ(OutEvents[3]->getAction(),MotionEvent::ACTION_UP);
   ASSERT_EQ(OutEvents[3]->getX(0),mts[11].value);
   ASSERT_EQ(OutEvents[3]->getY(0),mts[12].value);

   ASSERT_EQ(OutEvents[4]->getAction(),MotionEvent::ACTION_DOWN);
   ASSERT_EQ(OutEvents[5]->getAction(),MotionEvent::ACTION_UP);
}

#if defined(USE_TRACKINGID_AS_POINTERID)&&USE_TRACKINGID_AS_POINTERID
#define POINTERID(trackingId,index) trackingId
#else
#define POINTERID(trackingId,index) index
#endif

TEST_F(INPUTDEVICE,MTASST){//some wrong MT device ,can working:)
   TouchDevice d(INJECTDEV_TOUCH);
   MTEvent mts[]={
      {EV_ABS,ABS_MT_TRACKING_ID+1000,0x40},//0
      {EV_ABS,ABS_MT_POSITION_X ,20},
      {EV_ABS,ABS_MT_POSITION_Y ,30},
      {EV_KEY,BTN_TOUCH,1},
      {EV_SYN,SYN_REPORT,0},

      {EV_ABS,ABS_MT_POSITION_X ,22},//5
      {EV_ABS,ABS_MT_POSITION_Y ,33},
      {EV_SYN,SYN_REPORT,0},

      {EV_ABS,ABS_MT_POSITION_X ,24},//8
      {EV_ABS,ABS_MT_POSITION_Y ,34},
      {EV_SYN,SYN_REPORT,0},

      {EV_ABS,ABS_MT_TRACKING_ID+1000,-1},//11
      {EV_KEY,BTN_TOUCH,0},
      {EV_SYN,SYN_REPORT,0},

      {EV_KEY,BTN_TOUCH,1},//14
      {EV_ABS,ABS_MT_POSITION_X,10},
      {EV_ABS,ABS_MT_POSITION_Y,20},
      {EV_SYN,SYN_REPORT,0},

      {EV_ABS,ABS_MT_POSITION_X,15},//18
      {EV_ABS,ABS_MT_POSITION_Y,25},
      {EV_SYN,SYN_REPORT,0},

      {EV_KEY,BTN_TOUCH,0},//21
      {EV_ABS,ABS_MT_POSITION_X,16},
      {EV_ABS,ABS_MT_POSITION_Y,26},
      {EV_SYN,SYN_REPORT,0}
   };
   EventCount = sendEvents(d,mts,sizeof(mts)/sizeof(MTEvent),OutEvents);
   ASSERT_EQ(EventCount,7);
   ASSERT_EQ(OutEvents[0]->getAction(),MotionEvent::ACTION_DOWN);
   ASSERT_EQ(OutEvents[0]->getX(),mts[1].value);
   ASSERT_EQ(OutEvents[0]->getY(),mts[2].value);

   ASSERT_EQ(OutEvents[1]->getActionMasked(),MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[1]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[1]->getPointerId(0),0);
   ASSERT_EQ(OutEvents[1]->getX(),mts[5].value);
   ASSERT_EQ(OutEvents[1]->getY(),mts[6].value);

   ASSERT_EQ(OutEvents[2]->getActionMasked(),MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[2]->getX(),mts[8].value);
   ASSERT_EQ(OutEvents[2]->getY(),mts[9].value);

   ASSERT_EQ(OutEvents[3]->getAction(),MotionEvent::ACTION_UP);
   ASSERT_EQ(OutEvents[3]->getX(),mts[8].value);
   ASSERT_EQ(OutEvents[3]->getY(),mts[9].value);

   ASSERT_EQ(OutEvents[4]->getAction(),MotionEvent::ACTION_DOWN);
   ASSERT_EQ(OutEvents[4]->getPointerCount(),1);
   
   ASSERT_EQ(OutEvents[5]->getActionMasked(),MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[5]->getPointerCount(),1);

   ASSERT_EQ(OutEvents[6]->getAction(),MotionEvent::ACTION_UP);
   ASSERT_EQ(OutEvents[6]->getPointerCount(),1);

}

TEST_F(INPUTDEVICE,MTASST2){//some wrong MT device ,can working:)
   TouchDevice d(INJECTDEV_TOUCH);
   MTEvent mts[]={
      {EV_ABS,ABS_MT_TRACKING_ID,0x40},//0
      {EV_ABS,ABS_MT_POSITION_X ,20},
      {EV_ABS,ABS_MT_POSITION_Y ,30},
      {EV_KEY,BTN_TOUCH,1},
      {EV_SYN,SYN_REPORT,0},

      {EV_ABS,ABS_MT_POSITION_X ,22},//5
      {EV_ABS,ABS_MT_POSITION_Y ,33},
      {EV_SYN,SYN_REPORT,0},

      {EV_ABS,ABS_MT_POSITION_X ,24},//8
      {EV_ABS,ABS_MT_POSITION_Y ,34},
      {EV_SYN,SYN_REPORT,0},

      {EV_ABS,ABS_MT_TRACKING_ID,-1},//11
      {EV_KEY,BTN_TOUCH,0},
      {EV_SYN,SYN_REPORT,0},

      {EV_KEY,BTN_TOUCH,1},//14
      {EV_ABS,ABS_MT_POSITION_X,10},
      {EV_ABS,ABS_MT_POSITION_Y,20},
      {EV_SYN,SYN_REPORT,0},

      {EV_KEY,BTN_TOUCH,0},//18
      {EV_ABS,ABS_MT_POSITION_X,16},
      {EV_ABS,ABS_MT_POSITION_Y,26},
      {EV_SYN,SYN_REPORT,0}
   };
   EventCount = sendEvents(d,mts,sizeof(mts)/sizeof(MTEvent),OutEvents);
   ASSERT_EQ(EventCount,6);
   ASSERT_EQ(OutEvents[0]->getAction(),MotionEvent::ACTION_DOWN);
   ASSERT_EQ(OutEvents[0]->getX(),mts[1].value);
   ASSERT_EQ(OutEvents[0]->getY(),mts[2].value);

   ASSERT_EQ(OutEvents[1]->getAction(),MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[1]->getPointerId(0),0);
   ASSERT_EQ(OutEvents[1]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[1]->getX(),mts[5].value);
   ASSERT_EQ(OutEvents[1]->getY(),mts[6].value);

   ASSERT_EQ(OutEvents[2]->getAction(),MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[2]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[2]->getX(),mts[8].value);
   ASSERT_EQ(OutEvents[2]->getY(),mts[9].value);

   ASSERT_EQ(OutEvents[3]->getAction(),MotionEvent::ACTION_UP);
   ASSERT_EQ(OutEvents[3]->getX(),mts[8].value);
   ASSERT_EQ(OutEvents[3]->getY(),mts[9].value);
}

TEST_F(INPUTDEVICE,MTASST3){//some wrong MT device ,can working:)
   TouchDevice d(INJECTDEV_TOUCH);
   MTEvent mts[]={
      {EV_ABS,ABS_MT_PRESSURE,0x40},//0
      {EV_ABS,ABS_MT_POSITION_X ,20},
      {EV_ABS,ABS_MT_POSITION_Y ,30},
      {EV_KEY,BTN_TOUCH,1},
      {EV_SYN,SYN_REPORT,0},

      {EV_ABS,ABS_MT_POSITION_X ,22},//5
      {EV_ABS,ABS_MT_POSITION_Y ,33},
      {EV_SYN,SYN_REPORT,0},

      {EV_ABS,ABS_MT_POSITION_X ,24},//8
      {EV_ABS,ABS_MT_POSITION_Y ,34},
      {EV_SYN,SYN_REPORT,0},

      {EV_ABS,ABS_MT_PRESSURE,0x34},//11
      {EV_KEY,BTN_TOUCH,0},
      {EV_SYN,SYN_REPORT,0},

      {EV_KEY,BTN_TOUCH,1},//14
      {EV_ABS,ABS_MT_POSITION_X,10},
      {EV_ABS,ABS_MT_POSITION_Y,20},
      {EV_SYN,SYN_REPORT,0},

      {EV_KEY,BTN_TOUCH,0},//18
      {EV_ABS,ABS_MT_POSITION_X,16},
      {EV_ABS,ABS_MT_POSITION_Y,26},
      {EV_SYN,SYN_REPORT,0}
   };
   EventCount = sendEvents(d,mts,sizeof(mts)/sizeof(MTEvent),OutEvents);
   ASSERT_EQ(EventCount,6);
   ASSERT_EQ(OutEvents[0]->getActionMasked(),MotionEvent::ACTION_DOWN);
   ASSERT_EQ(OutEvents[0]->getX(),mts[1].value);
   ASSERT_EQ(OutEvents[0]->getY(),mts[2].value);

   ASSERT_EQ(OutEvents[1]->getActionMasked(),MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[1]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[1]->getPointerId(0),0);
   ASSERT_EQ(OutEvents[1]->getX(),mts[5].value);
   ASSERT_EQ(OutEvents[1]->getY(),mts[6].value);

   ASSERT_EQ(OutEvents[2]->getActionMasked(),MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[2]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[2]->getX(),mts[8].value);
   ASSERT_EQ(OutEvents[2]->getY(),mts[9].value);

   ASSERT_EQ(OutEvents[3]->getActionMasked(),MotionEvent::ACTION_UP);
   ASSERT_EQ(OutEvents[3]->getX(),mts[8].value);
   ASSERT_EQ(OutEvents[3]->getY(),mts[9].value);
}

TEST_F(INPUTDEVICE,MTA){//TypeA Events
   TouchDevice d(INJECTDEV_TOUCH);
   MTEvent mts[]={
      {EV_ABS,ABS_MT_TRACKING_ID,1},//0
      {EV_ABS,ABS_MT_POSITION_X ,20},
      {EV_ABS,ABS_MT_POSITION_Y ,30},
      {EV_SYN,SYN_MT_REPORT,0},
      {EV_SYN,SYN_REPORT,0},

      {EV_ABS,ABS_MT_TRACKING_ID,1},//5
      {EV_ABS,ABS_MT_POSITION_X ,20},
      {EV_ABS,ABS_MT_POSITION_Y ,30},
      {EV_SYN,SYN_MT_REPORT,0},
      {EV_ABS,ABS_MT_TRACKING_ID,2},
      {EV_ABS,ABS_MT_POSITION_X ,120},//10
      {EV_ABS,ABS_MT_POSITION_Y ,130},
      {EV_SYN,SYN_MT_REPORT,0},
      {EV_SYN,SYN_REPORT,0},

      {EV_ABS,ABS_MT_TRACKING_ID,1},//14
      {EV_ABS,ABS_MT_POSITION_X ,20},//15
      {EV_ABS,ABS_MT_POSITION_Y ,30},
      {EV_SYN,SYN_MT_REPORT,0},
      {EV_ABS,ABS_MT_TRACKING_ID,2},
      {EV_ABS,ABS_MT_POSITION_X ,120},
      {EV_ABS,ABS_MT_POSITION_Y ,130},//20
      {EV_SYN,SYN_MT_REPORT,0},
      {EV_ABS,ABS_MT_TRACKING_ID,3},
      {EV_ABS,ABS_MT_POSITION_X ,220},
      {EV_ABS,ABS_MT_POSITION_Y ,230},
      {EV_SYN,SYN_MT_REPORT,0},//25
      {EV_SYN,SYN_REPORT,0},

      {EV_ABS,ABS_MT_TRACKING_ID,1},//27 POINTER_UP finger 2(trackid 3) is up
      {EV_ABS,ABS_MT_POSITION_X ,20},
      {EV_ABS,ABS_MT_POSITION_Y ,30},
      {EV_SYN,SYN_MT_REPORT,0},     //30
      {EV_ABS,ABS_MT_TRACKING_ID,2},
      {EV_ABS,ABS_MT_POSITION_X ,120},
      {EV_ABS,ABS_MT_POSITION_Y ,130},
      {EV_SYN,SYN_MT_REPORT,0},
      {EV_SYN,SYN_REPORT,0},//35

      {EV_ABS,ABS_MT_TRACKING_ID,2},//36 POINTER_UP finger 1(tracckid 1) isup
      {EV_ABS,ABS_MT_POSITION_X ,120},
      {EV_ABS,ABS_MT_POSITION_Y ,130},
      {EV_SYN,SYN_MT_REPORT,0},
      {EV_SYN,SYN_REPORT,0},//40

      {EV_ABS,ABS_MT_TRACKING_ID,2},//41
      {EV_ABS,ABS_MT_POSITION_X ,123},
      {EV_ABS,ABS_MT_POSITION_Y ,134},
      {EV_SYN,SYN_MT_REPORT,0},
      {EV_SYN,SYN_REPORT,0},//45

      {EV_ABS,ABS_MT_TRACKING_ID,-1},//46
      {EV_KEY,BTN_TOUCH,0},//20
      {EV_SYN,SYN_MT_REPORT,0},
      {EV_SYN,SYN_REPORT,0},

      {EV_ABS,ABS_MT_TRACKING_ID,1},//50
      {EV_ABS,ABS_MT_POSITION_X ,20},
      {EV_ABS,ABS_MT_POSITION_Y ,30},
      {EV_SYN,SYN_MT_REPORT,0},
      {EV_SYN,SYN_REPORT,0},

      //{EV_ABS,ABS_MT_TRACKING_ID,-1},//55
      {EV_KEY,BTN_TOUCH,0},
      {EV_SYN,SYN_MT_REPORT,0},
      {EV_SYN,SYN_REPORT,0}

   };
   EventCount = sendEvents(d,mts,sizeof(mts)/sizeof(MTEvent),OutEvents);
   ASSERT_EQ(EventCount,9);
   ASSERT_EQ(OutEvents[0]->getAction(),MotionEvent::ACTION_DOWN);
   ASSERT_EQ(OutEvents[0]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[0]->getPointerId(0),POINTERID(mts[0].value,0));
   ASSERT_EQ(OutEvents[0]->getX(0),mts[1].value);//20
   ASSERT_EQ(OutEvents[0]->getY(0),mts[2].value);//30

   ASSERT_EQ(OutEvents[1]->getActionMasked(),MotionEvent::ACTION_POINTER_DOWN);
   ASSERT_EQ(OutEvents[1]->getActionIndex(),1);
   ASSERT_EQ(OutEvents[1]->getPointerCount(),2);
   ASSERT_EQ(OutEvents[1]->getPointerId(0),POINTERID(mts[5].value,0));
   ASSERT_EQ(OutEvents[1]->getPointerId(1),POINTERID(mts[9].value,1));
   ASSERT_EQ(OutEvents[1]->getX(0),mts[6].value);//20
   ASSERT_EQ(OutEvents[1]->getY(0),mts[7].value);//30
   ASSERT_EQ(OutEvents[1]->getX(1),mts[10].value);//120
   ASSERT_EQ(OutEvents[1]->getY(1),mts[11].value);//130

   ASSERT_EQ(OutEvents[2]->getActionMasked(),MotionEvent::ACTION_POINTER_DOWN);
   ASSERT_EQ(OutEvents[2]->getActionIndex(),2);
   ASSERT_EQ(OutEvents[2]->getPointerCount(),3);
   ASSERT_EQ(OutEvents[2]->getPointerId(0),POINTERID(mts[14].value,0));
   ASSERT_EQ(OutEvents[2]->getPointerId(1),POINTERID(mts[18].value,1));
   ASSERT_EQ(OutEvents[2]->getPointerId(2),POINTERID(mts[22].value,2));
   ASSERT_EQ(OutEvents[2]->getX(0),mts[15].value);//20
   ASSERT_EQ(OutEvents[2]->getY(0),mts[16].value);//30
   ASSERT_EQ(OutEvents[2]->getX(1),mts[19].value);//120
   ASSERT_EQ(OutEvents[2]->getY(1),mts[20].value);//130
   ASSERT_EQ(OutEvents[2]->getX(2),mts[23].value);//120
   ASSERT_EQ(OutEvents[2]->getY(2),mts[24].value);//130

   ASSERT_EQ(OutEvents[3]->getActionMasked(),MotionEvent::ACTION_POINTER_UP);
   ASSERT_EQ(OutEvents[3]->getActionIndex(),2);
   ASSERT_EQ(OutEvents[3]->getPointerCount(),3);
   ASSERT_EQ(OutEvents[3]->getPointerId(0),POINTERID(mts[27].value,0));
   ASSERT_EQ(OutEvents[3]->getPointerId(1),POINTERID(mts[31].value,1));
   ASSERT_EQ(OutEvents[3]->getX(0),mts[28].value);//20
   ASSERT_EQ(OutEvents[3]->getY(0),mts[29].value);//30
   ASSERT_EQ(OutEvents[3]->getX(1),mts[32].value);//120
   ASSERT_EQ(OutEvents[3]->getY(1),mts[33].value);//130

   ASSERT_EQ(OutEvents[4]->getActionMasked(),MotionEvent::ACTION_POINTER_UP);
   ASSERT_EQ(OutEvents[4]->getActionIndex(),1);
   ASSERT_EQ(OutEvents[4]->getPointerId(0),POINTERID(mts[27].value,0));
   ASSERT_EQ(OutEvents[4]->getPointerId(1),POINTERID(mts[31].value,1));
   ASSERT_EQ(OutEvents[4]->getPointerCount(),2);
   ASSERT_EQ(OutEvents[4]->getX(0),mts[28].value);//123
   ASSERT_EQ(OutEvents[4]->getY(0),mts[29].value);//134

   ASSERT_EQ(OutEvents[5]->getActionMasked(),MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[5]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[5]->getPointerId(0),POINTERID(mts[31].value,0));
   ASSERT_EQ(OutEvents[5]->getPointerCount(),1);
   ASSERT_EQ(OutEvents[5]->getX(0),mts[42].value);//123
   ASSERT_EQ(OutEvents[5]->getY(0),mts[43].value);//134

   ASSERT_EQ(OutEvents[6]->getActionMasked(),MotionEvent::ACTION_UP);
   ASSERT_EQ(OutEvents[6]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[6]->getPointerCount(),1);
   ASSERT_EQ(OutEvents[6]->getX(0),mts[42].value);//123
   ASSERT_EQ(OutEvents[6]->getY(0),mts[43].value);//134

   ASSERT_EQ(OutEvents[7]->getActionMasked(),MotionEvent::ACTION_DOWN);
   ASSERT_EQ(OutEvents[7]->getX(0),mts[51].value);
   ASSERT_EQ(OutEvents[7]->getY(0),mts[52].value);
   ASSERT_EQ(OutEvents[7]->getActionIndex(),0);

   ASSERT_EQ(OutEvents[8]->getActionMasked(),MotionEvent::ACTION_UP);
   ASSERT_EQ(OutEvents[8]->getX(0),mts[51].value);
   ASSERT_EQ(OutEvents[8]->getY(0),mts[52].value);
   ASSERT_EQ(OutEvents[8]->getActionIndex(),0);
}

TEST_F(INPUTDEVICE,MTA2){
   TouchDevice d(INJECTDEV_TOUCH);
   MTEvent mts[]={
      {EV_ABS,ABS_MT_TRACKING_ID,1},//0
      {EV_ABS,ABS_MT_POSITION_X ,20},
      {EV_ABS,ABS_MT_POSITION_Y ,30},
      {EV_SYN,SYN_MT_REPORT,0},
      {EV_SYN,SYN_REPORT,0},

      {EV_ABS,ABS_MT_TRACKING_ID,1},//5
      {EV_ABS,ABS_MT_POSITION_X ,22},
      {EV_ABS,ABS_MT_POSITION_Y ,32},
      {EV_SYN,SYN_MT_REPORT,0},
      {EV_SYN,SYN_REPORT,0},

      {EV_ABS,ABS_MT_TRACKING_ID,1},//10
      {EV_ABS,ABS_MT_POSITION_X ,24},
      {EV_ABS,ABS_MT_POSITION_Y ,34},
      {EV_SYN,SYN_MT_REPORT,0},
      {EV_SYN,SYN_REPORT,0},

      {EV_ABS,ABS_MT_TRACKING_ID,1},//15 POINTER_UP finger 2(trackid 3) is up
      {EV_ABS,ABS_MT_POSITION_X ,26},
      {EV_ABS,ABS_MT_POSITION_Y ,36},
      {EV_SYN,SYN_MT_REPORT,0},
      {EV_SYN,SYN_REPORT,0},

      {EV_ABS,ABS_MT_TRACKING_ID,1},//20 POINTER_UP finger 1(tracckid 1) isup
      {EV_ABS,ABS_MT_POSITION_X ,28},
      {EV_ABS,ABS_MT_POSITION_Y ,38},
      {EV_SYN,SYN_MT_REPORT,0},
      {EV_SYN,SYN_REPORT,0},

      {EV_ABS,ABS_MT_TRACKING_ID,1},//25
      {EV_ABS,ABS_MT_POSITION_X ,30},
      {EV_ABS,ABS_MT_POSITION_Y ,40},
      {EV_SYN,SYN_MT_REPORT,0},
      {EV_SYN,SYN_REPORT,0},

      {EV_ABS,ABS_MT_TRACKING_ID,-1},//30
      {EV_KEY,BTN_TOUCH,0},//20
      {EV_SYN,SYN_MT_REPORT,0},
      {EV_SYN,SYN_REPORT,0},

      //the second touch sequences
      {EV_ABS,ABS_MT_TRACKING_ID,1},//34
      {EV_ABS,ABS_MT_POSITION_X ,20},
      {EV_ABS,ABS_MT_POSITION_Y ,30},
      {EV_SYN,SYN_MT_REPORT,0},
      {EV_SYN,SYN_REPORT,0},

      //{EV_ABS,ABS_MT_TRACKING_ID,-1},//39
      {EV_KEY,BTN_TOUCH,0},
      {EV_SYN,SYN_MT_REPORT,0},
      {EV_SYN,SYN_REPORT,0}

   };
   EventCount = sendEvents(d,mts,sizeof(mts)/sizeof(MTEvent),OutEvents);
   ASSERT_EQ(EventCount,9);

   ASSERT_EQ(OutEvents[7]->getActionMasked(),MotionEvent::ACTION_DOWN);
   ASSERT_EQ(OutEvents[7]->getX(0),mts[35].value);
   ASSERT_EQ(OutEvents[7]->getY(0),mts[36].value);
   ASSERT_EQ(OutEvents[7]->getActionIndex(),0);

   ASSERT_EQ(OutEvents[8]->getActionMasked(),MotionEvent::ACTION_UP);
   ASSERT_EQ(OutEvents[8]->getX(0),mts[35].value);
   ASSERT_EQ(OutEvents[8]->getY(0),mts[36].value);
   ASSERT_EQ(OutEvents[8]->getActionIndex(),0);
}

TEST_F(INPUTDEVICE,MTB){//Type B Events
   TouchDevice d(INJECTDEV_TOUCH);
   MTEvent mts[]={
     {EV_KEY,BTN_TOUCH,1},          //ACTION_DOWN
     {EV_ABS,ABS_MT_SLOT,0},
     {EV_ABS,ABS_MT_TRACKING_ID,45},
     {EV_ABS,ABS_MT_POSITION_X ,10},
     {EV_ABS,ABS_MT_POSITION_Y ,20},
     {EV_SYN,SYN_MT_REPORT,0},//5
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,0},//7
     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_MOVE
     {EV_ABS,ABS_MT_POSITION_X ,20},
     {EV_ABS,ABS_MT_POSITION_Y ,30},//10
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,0},//13
     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_POINTER_DOWN finger 0
     {EV_ABS,ABS_MT_POSITION_X ,20},//15
     {EV_ABS,ABS_MT_POSITION_Y ,30},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_SLOT,1},
     {EV_ABS,ABS_MT_TRACKING_ID,46},//ACTION_POINTER_DOWN finger 1
     {EV_ABS,ABS_MT_POSITION_X ,40},//20
     {EV_ABS,ABS_MT_POSITION_Y ,100},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,0},//24
     {EV_ABS,ABS_MT_TRACKING_ID,45},//25 ACTION_POINTER_DOWN finger 0
     {EV_ABS,ABS_MT_POSITION_X ,20},
     {EV_ABS,ABS_MT_POSITION_Y ,30},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_SLOT,1},
     {EV_ABS,ABS_MT_TRACKING_ID,46},//30 ACTION_POINTER_DOWN finger 1
     {EV_ABS,ABS_MT_POSITION_X ,40},
     {EV_ABS,ABS_MT_POSITION_Y ,100},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_SLOT,2},
     {EV_ABS,ABS_MT_TRACKING_ID,47},//35 ACTION_POINTER_DOWN finger 2
     {EV_ABS,ABS_MT_POSITION_X ,200},
     {EV_ABS,ABS_MT_POSITION_Y ,300},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,0},//40
     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_POINTER_UP finger 2
     {EV_ABS,ABS_MT_POSITION_X ,20},
     {EV_ABS,ABS_MT_POSITION_Y ,30},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_SLOT,1},//45
     {EV_ABS,ABS_MT_TRACKING_ID,46},
     {EV_ABS,ABS_MT_POSITION_X ,40},
     {EV_ABS,ABS_MT_POSITION_Y ,100},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_SLOT,2},//50
     {EV_ABS,ABS_MT_TRACKING_ID,-1},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,0},//54
     {EV_ABS,ABS_MT_TRACKING_ID,45},//55 ACTION_POINTER_UP finger 1
     {EV_ABS,ABS_MT_POSITION_X ,20},
     {EV_ABS,ABS_MT_POSITION_Y ,30},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_SLOT,1},
     {EV_ABS,ABS_MT_TRACKING_ID,-1},//60
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,0},//63
     {EV_ABS,ABS_MT_TRACKING_ID,-1},
     {EV_SYN,SYN_MT_REPORT,0},//65
     {EV_SYN,SYN_REPORT,0},

   };
   EventCount = sendEvents(d,mts,sizeof(mts)/sizeof(MTEvent),OutEvents);
   ASSERT_EQ(EventCount,7);
   ASSERT_EQ(OutEvents[0]->getActionMasked(),MotionEvent::ACTION_DOWN);
   ASSERT_EQ(OutEvents[0]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[0]->getPointerCount(),1);
   ASSERT_EQ(OutEvents[0]->getPointerId(0),POINTERID(mts[2].value,0));
   ASSERT_EQ(OutEvents[0]->getX(),mts[3].value);
   ASSERT_EQ(OutEvents[0]->getY(),mts[4].value);

   ASSERT_EQ(OutEvents[1]->getActionMasked(),MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[1]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[1]->getPointerCount(),1);
   ASSERT_EQ(OutEvents[1]->getPointerId(0),POINTERID(mts[8].value,0));
   ASSERT_EQ(OutEvents[1]->getX(),mts[9].value);
   ASSERT_EQ(OutEvents[1]->getY(),mts[10].value);

   ASSERT_EQ(OutEvents[2]->getActionMasked(),MotionEvent::ACTION_POINTER_DOWN);
   ASSERT_EQ(OutEvents[2]->getActionIndex(),1);
   ASSERT_EQ(OutEvents[2]->getPointerCount(),2);
   ASSERT_EQ(OutEvents[2]->getPointerId(0),POINTERID(mts[14].value,0));
   ASSERT_EQ(OutEvents[2]->getPointerId(1),POINTERID(mts[19].value,1));
   ASSERT_EQ(OutEvents[2]->getX(0),mts[15].value);
   ASSERT_EQ(OutEvents[2]->getY(0),mts[16].value);
   ASSERT_EQ(OutEvents[2]->getX(1),mts[20].value);
   ASSERT_EQ(OutEvents[2]->getY(1),mts[21].value);

   ASSERT_EQ(OutEvents[3]->getActionMasked(),MotionEvent::ACTION_POINTER_DOWN);
   ASSERT_EQ(OutEvents[3]->getActionIndex(),2);
   ASSERT_EQ(OutEvents[3]->getPointerCount(),3);
   ASSERT_EQ(OutEvents[3]->getPointerId(0),POINTERID(mts[25].value,0));
   ASSERT_EQ(OutEvents[3]->getPointerId(1),POINTERID(mts[30].value,1));
   ASSERT_EQ(OutEvents[3]->getPointerId(2),POINTERID(mts[35].value,2));
   ASSERT_EQ(OutEvents[3]->getX(0),mts[26].value);
   ASSERT_EQ(OutEvents[3]->getY(0),mts[27].value);
   ASSERT_EQ(OutEvents[3]->getX(1),mts[31].value);
   ASSERT_EQ(OutEvents[3]->getY(1),mts[32].value);
   ASSERT_EQ(OutEvents[3]->getX(2),mts[36].value);
   ASSERT_EQ(OutEvents[3]->getY(2),mts[37].value);

   ASSERT_EQ(OutEvents[4]->getActionMasked(),MotionEvent::ACTION_POINTER_UP);
   ASSERT_EQ(OutEvents[4]->getActionIndex(),2);
   ASSERT_EQ(OutEvents[4]->getPointerCount(),3);
   ASSERT_EQ(OutEvents[4]->getPointerId(0),POINTERID(mts[41].value,0));
   ASSERT_EQ(OutEvents[4]->getPointerId(1),POINTERID(mts[46].value,1));
   ASSERT_EQ(OutEvents[4]->getPointerId(2),POINTERID(mts[35].value,2));
   ASSERT_EQ(OutEvents[4]->getX(0),mts[42].value);
   ASSERT_EQ(OutEvents[4]->getY(0),mts[43].value);
   ASSERT_EQ(OutEvents[4]->getX(1),mts[47].value);
   ASSERT_EQ(OutEvents[4]->getY(1),mts[48].value);
   ASSERT_EQ(OutEvents[4]->getX(2),mts[36].value);
   ASSERT_EQ(OutEvents[4]->getY(2),mts[37].value);

   ASSERT_EQ(OutEvents[5]->getActionMasked(),MotionEvent::ACTION_POINTER_UP);
   ASSERT_EQ(OutEvents[5]->getActionIndex(),1);
   ASSERT_EQ(OutEvents[5]->getPointerCount(),2);
   ASSERT_EQ(OutEvents[5]->getPointerId(0),POINTERID(mts[55].value,0));
   ASSERT_EQ(OutEvents[5]->getPointerId(1),POINTERID(mts[46].value,1));
   ASSERT_EQ(OutEvents[5]->getX(0),mts[56].value);
   ASSERT_EQ(OutEvents[5]->getY(0),mts[57].value);
   ASSERT_EQ(OutEvents[5]->getX(1),mts[47].value);
   ASSERT_EQ(OutEvents[5]->getY(1),mts[48].value);

   ASSERT_EQ(OutEvents[6]->getActionMasked(),MotionEvent::ACTION_UP);
   ASSERT_EQ(OutEvents[6]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[6]->getPointerCount(),1);
   ASSERT_EQ(OutEvents[6]->getPointerId(0),POINTERID(mts[55].value,0));
   ASSERT_EQ(OutEvents[6]->getX(0),mts[56].value);
   ASSERT_EQ(OutEvents[6]->getY(0),mts[57].value);
}

TEST_F(INPUTDEVICE,MTB2){//Type B Events
   TouchDevice d(INJECTDEV_TOUCH);
   MTEvent mts[]={
     {EV_KEY,BTN_TOUCH,1},          //ACTION_DOWN
     {EV_ABS,ABS_MT_SLOT,0},
     {EV_ABS,ABS_MT_TRACKING_ID,45},
     {EV_ABS,ABS_MT_POSITION_X ,10},
     {EV_ABS,ABS_MT_POSITION_Y ,20},
     {EV_SYN,SYN_MT_REPORT,0},//5
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,0},//7
     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_MOVE
     {EV_ABS,ABS_MT_POSITION_X ,20},
     {EV_ABS,ABS_MT_POSITION_Y ,30},//10
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,0},//13
     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_POINTER_DOWN finger 0
     {EV_ABS,ABS_MT_POSITION_X ,20},//15
     {EV_ABS,ABS_MT_POSITION_Y ,30},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_SLOT,1},
     {EV_ABS,ABS_MT_TRACKING_ID,46},//19 ACTION_POINTER_DOWN finger 1
     {EV_ABS,ABS_MT_POSITION_X ,40},//20
     {EV_ABS,ABS_MT_POSITION_Y ,100},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,0},//24  ACTION_MOVE
     {EV_ABS,ABS_MT_TRACKING_ID,45},//25 finger 0 moving
     {EV_ABS,ABS_MT_POSITION_X ,22},
     {EV_ABS,ABS_MT_POSITION_Y ,32},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,1},//30 ACTION_MOVE
     {EV_ABS,ABS_MT_TRACKING_ID,46},//fingler 1
     {EV_ABS,ABS_MT_POSITION_X ,42},
     {EV_ABS,ABS_MT_POSITION_Y ,102},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,0},//36
     {EV_ABS,ABS_MT_TRACKING_ID,45},//37 ACTION_POINTER_UP finger 1
     {EV_ABS,ABS_MT_POSITION_X ,22},
     {EV_ABS,ABS_MT_POSITION_Y ,32},
     {EV_SYN,SYN_MT_REPORT,0},//40
     {EV_ABS,ABS_MT_SLOT,1},//fingler 1 is up
     {EV_ABS,ABS_MT_TRACKING_ID,-1},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,0},//45 ACTION_UP
     {EV_ABS,ABS_MT_TRACKING_ID,-1},//finger 0 us up
     {EV_SYN,SYN_MT_REPORT,0},//50
     {EV_SYN,SYN_REPORT,0},

   };
   EventCount = sendEvents(d,mts,sizeof(mts)/sizeof(MTEvent),OutEvents);
   ASSERT_EQ(EventCount,7);
   ASSERT_EQ(OutEvents[0]->getActionMasked(),MotionEvent::ACTION_DOWN);
   ASSERT_EQ(OutEvents[0]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[0]->getPointerCount(),1);
   ASSERT_EQ(OutEvents[0]->getPointerId(0),POINTERID(mts[2].value,0));
   ASSERT_EQ(OutEvents[0]->getX(),mts[3].value);
   ASSERT_EQ(OutEvents[0]->getY(),mts[4].value);

   ASSERT_EQ(OutEvents[1]->getActionMasked(),MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[1]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[1]->getPointerCount(),1);
   ASSERT_EQ(OutEvents[1]->getPointerId(0),POINTERID(mts[8].value,0));
   ASSERT_EQ(OutEvents[1]->getX(),mts[9].value);
   ASSERT_EQ(OutEvents[1]->getY(),mts[10].value);

   ASSERT_EQ(OutEvents[2]->getActionMasked(),MotionEvent::ACTION_POINTER_DOWN);
   ASSERT_EQ(OutEvents[2]->getActionIndex(),1);
   ASSERT_EQ(OutEvents[2]->getPointerCount(),2);
   ASSERT_EQ(OutEvents[2]->getPointerId(0),POINTERID(mts[14].value,0));
   ASSERT_EQ(OutEvents[2]->getPointerId(1),POINTERID(mts[19].value,1));
   ASSERT_EQ(OutEvents[2]->getX(0),mts[15].value);
   ASSERT_EQ(OutEvents[2]->getY(0),mts[16].value);
   ASSERT_EQ(OutEvents[2]->getX(1),mts[20].value);
   ASSERT_EQ(OutEvents[2]->getY(1),mts[21].value);

   ASSERT_EQ(OutEvents[3]->getActionMasked(),MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[3]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[3]->getPointerCount(),2);
   ASSERT_EQ(OutEvents[3]->getPointerId(0),POINTERID(mts[14].value,0));
   ASSERT_EQ(OutEvents[3]->getPointerId(1),POINTERID(mts[19].value,1));
   ASSERT_EQ(OutEvents[3]->getX(0),mts[26].value);
   ASSERT_EQ(OutEvents[3]->getY(0),mts[27].value);
   ASSERT_EQ(OutEvents[3]->getX(1),mts[20].value);
   ASSERT_EQ(OutEvents[3]->getY(1),mts[21].value);

   ASSERT_EQ(OutEvents[4]->getActionMasked(),MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[4]->getActionIndex(),1);
   ASSERT_EQ(OutEvents[4]->getPointerCount(),2);
   ASSERT_EQ(OutEvents[4]->getPointerId(0),POINTERID(mts[14].value,0));
   ASSERT_EQ(OutEvents[4]->getPointerId(1),POINTERID(mts[31].value,1));
   ASSERT_EQ(OutEvents[4]->getX(0),mts[26].value);
   ASSERT_EQ(OutEvents[4]->getY(0),mts[27].value);
   ASSERT_EQ(OutEvents[4]->getX(1),mts[32].value);
   ASSERT_EQ(OutEvents[4]->getY(1),mts[33].value);

   ASSERT_EQ(OutEvents[5]->getActionMasked(),MotionEvent::ACTION_POINTER_UP);
   ASSERT_EQ(OutEvents[5]->getActionIndex(),1);
   ASSERT_EQ(OutEvents[5]->getPointerCount(),2);
   ASSERT_EQ(OutEvents[5]->getPointerId(0),POINTERID(mts[14].value,0));
   ASSERT_EQ(OutEvents[5]->getPointerId(1),POINTERID(mts[19].value,1));
   ASSERT_EQ(OutEvents[5]->getX(0),mts[38].value);
   ASSERT_EQ(OutEvents[5]->getY(0),mts[39].value);
   ASSERT_EQ(OutEvents[5]->getX(1),mts[32].value);
   ASSERT_EQ(OutEvents[5]->getY(1),mts[33].value);

   ASSERT_EQ(OutEvents[6]->getActionMasked(),MotionEvent::ACTION_UP);
   ASSERT_EQ(OutEvents[6]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[6]->getPointerCount(),1);
   ASSERT_EQ(OutEvents[6]->getPointerId(0),POINTERID(mts[37].value,0));
   ASSERT_EQ(OutEvents[6]->getX(0),mts[38].value);
   ASSERT_EQ(OutEvents[6]->getY(0),mts[39].value);

}

TEST_F(INPUTDEVICE,MTB3){//Type B Events,test SLOT not start from 0
   TouchDevice d(INJECTDEV_TOUCH);
   MTEvent mts[]={
     {EV_KEY,BTN_TOUCH,1},          //ACTION_DOWN
     {EV_ABS,ABS_MT_SLOT,10},
     {EV_ABS,ABS_MT_TRACKING_ID,45},
     {EV_ABS,ABS_MT_POSITION_X ,10},
     {EV_ABS,ABS_MT_POSITION_Y ,20},
     {EV_SYN,SYN_MT_REPORT,0},//5
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,10},//7
     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_MOVE
     {EV_ABS,ABS_MT_POSITION_X ,20},
     {EV_ABS,ABS_MT_POSITION_Y ,30},//10
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,10},//13
     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_POINTER_DOWN finger 0
     {EV_ABS,ABS_MT_POSITION_X ,20},//15
     {EV_ABS,ABS_MT_POSITION_Y ,30},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_SLOT,11},
     {EV_ABS,ABS_MT_TRACKING_ID,46},//19 ACTION_POINTER_DOWN finger 1
     {EV_ABS,ABS_MT_POSITION_X ,40},//20
     {EV_ABS,ABS_MT_POSITION_Y ,100},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,10},//24  ACTION_MOVE
     {EV_ABS,ABS_MT_TRACKING_ID,45},//25 finger 0 moving
     {EV_ABS,ABS_MT_POSITION_X ,22},
     {EV_ABS,ABS_MT_POSITION_Y ,32},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,11},//30 ACTION_MOVE
     {EV_ABS,ABS_MT_TRACKING_ID,46},//fingler 1
     {EV_ABS,ABS_MT_POSITION_X ,42},
     {EV_ABS,ABS_MT_POSITION_Y ,102},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,10},//36
     {EV_ABS,ABS_MT_TRACKING_ID,45},//37 ACTION_POINTER_UP finger 1
     {EV_ABS,ABS_MT_POSITION_X ,22},
     {EV_ABS,ABS_MT_POSITION_Y ,32},
     {EV_SYN,SYN_MT_REPORT,0},//40
     {EV_ABS,ABS_MT_SLOT,11},//fingler 1 is up
     {EV_ABS,ABS_MT_TRACKING_ID,-1},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,10},//45 ACTION_UP
     {EV_ABS,ABS_MT_TRACKING_ID,-1},//finger 0 us up
     {EV_SYN,SYN_MT_REPORT,0},//50
     {EV_SYN,SYN_REPORT,0},
   };

   EventCount = sendEvents(d,mts,sizeof(mts)/sizeof(MTEvent),OutEvents);
   ASSERT_EQ(EventCount,7);
   ASSERT_EQ(OutEvents[0]->getActionMasked(),MotionEvent::ACTION_DOWN);
   ASSERT_EQ(OutEvents[0]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[0]->getPointerCount(),1);
   ASSERT_EQ(OutEvents[0]->getPointerId(0),POINTERID(mts[2].value,0));
   ASSERT_EQ(OutEvents[0]->getX(),mts[3].value);
   ASSERT_EQ(OutEvents[0]->getY(),mts[4].value);

   ASSERT_EQ(OutEvents[1]->getActionMasked(),MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[1]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[1]->getPointerCount(),1);
   ASSERT_EQ(OutEvents[1]->getPointerId(0),POINTERID(mts[8].value,0));
   ASSERT_EQ(OutEvents[1]->getX(),mts[9].value);
   ASSERT_EQ(OutEvents[1]->getY(),mts[10].value);

   ASSERT_EQ(OutEvents[2]->getActionMasked(),MotionEvent::ACTION_POINTER_DOWN);
   ASSERT_EQ(OutEvents[2]->getActionIndex(),1);
   ASSERT_EQ(OutEvents[2]->getPointerCount(),2);
   ASSERT_EQ(OutEvents[2]->getPointerId(0),POINTERID(mts[14].value,0));
   ASSERT_EQ(OutEvents[2]->getPointerId(1),POINTERID(mts[19].value,1));
   ASSERT_EQ(OutEvents[2]->getX(0),mts[15].value);
   ASSERT_EQ(OutEvents[2]->getY(0),mts[16].value);
   ASSERT_EQ(OutEvents[2]->getX(1),mts[20].value);
   ASSERT_EQ(OutEvents[2]->getY(1),mts[21].value);

   ASSERT_EQ(OutEvents[3]->getActionMasked(),MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[3]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[3]->getPointerCount(),2);
   ASSERT_EQ(OutEvents[3]->getPointerId(0),POINTERID(mts[14].value,0));
   ASSERT_EQ(OutEvents[3]->getPointerId(1),POINTERID(mts[19].value,1));
   ASSERT_EQ(OutEvents[3]->getX(0),mts[26].value);
   ASSERT_EQ(OutEvents[3]->getY(0),mts[27].value);
   ASSERT_EQ(OutEvents[3]->getX(1),mts[20].value);
   ASSERT_EQ(OutEvents[3]->getY(1),mts[21].value);

   ASSERT_EQ(OutEvents[4]->getActionMasked(),MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[4]->getActionIndex(),1);
   ASSERT_EQ(OutEvents[4]->getPointerCount(),2);
   ASSERT_EQ(OutEvents[4]->getPointerId(0),POINTERID(mts[14].value,0));
   ASSERT_EQ(OutEvents[4]->getPointerId(1),POINTERID(mts[31].value,1));
   ASSERT_EQ(OutEvents[4]->getX(0),mts[26].value);
   ASSERT_EQ(OutEvents[4]->getY(0),mts[27].value);
   ASSERT_EQ(OutEvents[4]->getX(1),mts[32].value);
   ASSERT_EQ(OutEvents[4]->getY(1),mts[33].value);

   ASSERT_EQ(OutEvents[5]->getActionMasked(),MotionEvent::ACTION_POINTER_UP);
   ASSERT_EQ(OutEvents[5]->getActionIndex(),1);
   ASSERT_EQ(OutEvents[5]->getPointerCount(),2);
   ASSERT_EQ(OutEvents[5]->getPointerId(0),POINTERID(mts[14].value,0));
   ASSERT_EQ(OutEvents[5]->getPointerId(1),POINTERID(mts[19].value,1));
   ASSERT_EQ(OutEvents[5]->getX(0),mts[38].value);
   ASSERT_EQ(OutEvents[5]->getY(0),mts[39].value);
   ASSERT_EQ(OutEvents[5]->getX(1),mts[32].value);
   ASSERT_EQ(OutEvents[5]->getY(1),mts[33].value);

   ASSERT_EQ(OutEvents[6]->getActionMasked(),MotionEvent::ACTION_UP);
   ASSERT_EQ(OutEvents[6]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[6]->getPointerCount(),1);
   ASSERT_EQ(OutEvents[6]->getPointerId(0),POINTERID(mts[37].value,0));
   ASSERT_EQ(OutEvents[6]->getX(0),mts[38].value);
   ASSERT_EQ(OutEvents[6]->getY(0),mts[39].value);

}


TEST_F(INPUTDEVICE,MTB4){//Type B Events,test SLOT not start from 0
   TouchDevice d(INJECTDEV_TOUCH);
   MTEvent mts[]={
     {EV_KEY,BTN_TOUCH,1},          //ACTION_DOWN
     {EV_ABS,ABS_MT_SLOT,10},
     {EV_ABS,ABS_MT_TRACKING_ID,45},
     {EV_ABS,ABS_MT_POSITION_X ,10},
     {EV_ABS,ABS_MT_POSITION_Y ,20},
     {EV_SYN,SYN_MT_REPORT,0},//5
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,10},//7
     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_MOVE
     {EV_ABS,ABS_MT_POSITION_X ,20},
     {EV_ABS,ABS_MT_POSITION_Y ,30},//10
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,10},//13
     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_POINTER_DOWN finger 0
     {EV_ABS,ABS_MT_POSITION_X ,20},//15
     {EV_ABS,ABS_MT_POSITION_Y ,30},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_SLOT,11},
     {EV_ABS,ABS_MT_TRACKING_ID,46},//19 ACTION_POINTER_DOWN finger 1
     {EV_ABS,ABS_MT_POSITION_X ,40},//20
     {EV_ABS,ABS_MT_POSITION_Y ,100},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,10},//24  ACTION_MOVE
     //{EV_ABS,ABS_MT_TRACKING_ID,45},//25 finger 0 moving
     {EV_ABS,ABS_MT_POSITION_X ,22},//25
     {EV_ABS,ABS_MT_POSITION_Y ,32},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,11},//29 ACTION_MOVE
     //{EV_ABS,ABS_MT_TRACKING_ID,46},//fingler 1
     {EV_ABS,ABS_MT_POSITION_X ,42},//30
     {EV_ABS,ABS_MT_POSITION_Y ,102},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,10},//34
     //{EV_ABS,ABS_MT_TRACKING_ID,45},//35 ACTION_POINTER_UP finger 1
     {EV_ABS,ABS_MT_POSITION_X ,22},//35
     {EV_ABS,ABS_MT_POSITION_Y ,32},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_SLOT,11},//fingler 1 is up
     {EV_ABS,ABS_MT_TRACKING_ID,-1},
     {EV_SYN,SYN_MT_REPORT,0},//40
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_SLOT,10},//45 ACTION_UP
     {EV_ABS,ABS_MT_TRACKING_ID,-1},//finger 0 us up
     {EV_SYN,SYN_MT_REPORT,0},//50
     {EV_SYN,SYN_REPORT,0},
   };

   EventCount = sendEvents(d,mts,sizeof(mts)/sizeof(MTEvent),OutEvents);
   ASSERT_EQ(EventCount,7);
   ASSERT_EQ(OutEvents[0]->getActionMasked(),MotionEvent::ACTION_DOWN);
   ASSERT_EQ(OutEvents[0]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[0]->getPointerCount(),1);
   ASSERT_EQ(OutEvents[0]->getPointerId(0),POINTERID(mts[2].value,0));
   ASSERT_EQ(OutEvents[0]->getX(),mts[3].value);
   ASSERT_EQ(OutEvents[0]->getY(),mts[4].value);

   ASSERT_EQ(OutEvents[1]->getActionMasked(),MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[1]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[1]->getPointerCount(),1);
   ASSERT_EQ(OutEvents[1]->getPointerId(0),POINTERID(mts[8].value,0));
   ASSERT_EQ(OutEvents[1]->getX(),mts[9].value);
   ASSERT_EQ(OutEvents[1]->getY(),mts[10].value);

   ASSERT_EQ(OutEvents[2]->getActionMasked(),MotionEvent::ACTION_POINTER_DOWN);
   ASSERT_EQ(OutEvents[2]->getActionIndex(),1);
   ASSERT_EQ(OutEvents[2]->getPointerCount(),2);
   ASSERT_EQ(OutEvents[2]->getPointerId(0),POINTERID(mts[14].value,0));
   ASSERT_EQ(OutEvents[2]->getPointerId(1),POINTERID(mts[19].value,1));
   ASSERT_EQ(OutEvents[2]->getX(0),mts[15].value);
   ASSERT_EQ(OutEvents[2]->getY(0),mts[16].value);
   ASSERT_EQ(OutEvents[2]->getX(1),mts[20].value);
   ASSERT_EQ(OutEvents[2]->getY(1),mts[21].value);

   ASSERT_EQ(OutEvents[3]->getActionMasked(),MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[3]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[3]->getPointerCount(),2);
   ASSERT_EQ(OutEvents[3]->getPointerId(0),POINTERID(mts[14].value,0));
   ASSERT_EQ(OutEvents[3]->getPointerId(1),POINTERID(mts[19].value,1));
   ASSERT_EQ(OutEvents[3]->getX(0),mts[25].value);
   ASSERT_EQ(OutEvents[3]->getY(0),mts[26].value);
   ASSERT_EQ(OutEvents[3]->getX(1),mts[20].value);
   ASSERT_EQ(OutEvents[3]->getY(1),mts[21].value);

   ASSERT_EQ(OutEvents[4]->getActionMasked(),MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[4]->getActionIndex(),1);
   ASSERT_EQ(OutEvents[4]->getPointerCount(),2);
   ASSERT_EQ(OutEvents[4]->getPointerId(0),POINTERID(mts[14].value,0));
   ASSERT_EQ(OutEvents[4]->getPointerId(1),POINTERID(mts[31].value,1));
   ASSERT_EQ(OutEvents[4]->getX(0),mts[25].value);
   ASSERT_EQ(OutEvents[4]->getY(0),mts[26].value);
   ASSERT_EQ(OutEvents[4]->getX(1),mts[30].value);
   ASSERT_EQ(OutEvents[4]->getY(1),mts[31].value);

   ASSERT_EQ(OutEvents[5]->getActionMasked(),MotionEvent::ACTION_POINTER_UP);
   ASSERT_EQ(OutEvents[5]->getActionIndex(),1);
   ASSERT_EQ(OutEvents[5]->getPointerCount(),2);
   ASSERT_EQ(OutEvents[5]->getPointerId(0),POINTERID(mts[14].value,0));
   ASSERT_EQ(OutEvents[5]->getPointerId(1),POINTERID(mts[19].value,1));
   ASSERT_EQ(OutEvents[5]->getX(0),mts[35].value);
   ASSERT_EQ(OutEvents[5]->getY(0),mts[36].value);
   ASSERT_EQ(OutEvents[5]->getX(1),mts[30].value);
   ASSERT_EQ(OutEvents[5]->getY(1),mts[31].value);

   ASSERT_EQ(OutEvents[6]->getActionMasked(),MotionEvent::ACTION_UP);
   ASSERT_EQ(OutEvents[6]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[6]->getPointerCount(),1);
   ASSERT_EQ(OutEvents[6]->getPointerId(0),POINTERID(mts[34].value,0));
   ASSERT_EQ(OutEvents[6]->getX(0),mts[35].value);
   ASSERT_EQ(OutEvents[6]->getY(0),mts[36].value);

}


TEST_F(INPUTDEVICE, MTB5) {
    TouchDevice d(INJECTDEV_TOUCH);
    MTEvent mts[] = {
        {EV_ABS, ABS_MT_TRACKING_ID, 7},        // 0 - 开始跟踪触点ID 7
        {EV_ABS, ABS_MT_POSITION_X, 0x11f},     // 1 - X坐标 287
        {EV_ABS, ABS_MT_POSITION_Y, 0x35e},     // 2 - Y坐标 862
        {EV_ABS, ABS_MT_TOUCH_MAJOR, 0x3f},     // 3 - 触摸大小 63
        {EV_KEY, BTN_TOUCH, 1},                 // 4 - 触摸按下
        {EV_SYN, SYN_REPORT, 0},                // 5 - 同步报告
        
        {EV_ABS, ABS_MT_POSITION_Y, 0x364},     // 6 - Y坐标 868
        {EV_SYN, SYN_REPORT, 0},                // 7 - 同步报告
        
        {EV_ABS, ABS_MT_POSITION_Y, 0x36b},     // 8 - Y坐标 875
        {EV_SYN, SYN_REPORT, 0},                // 9 - 同步报告
        
        {EV_ABS, ABS_MT_POSITION_Y, 0x373},     // 10 - Y坐标 883
        {EV_SYN, SYN_REPORT, 0},                // 11 - 同步报告
        
        {EV_ABS, ABS_MT_POSITION_X, 0x11e},     // 12 - X坐标 286
        {EV_ABS, ABS_MT_POSITION_Y, 0x375},     // 13 - Y坐标 885
        {EV_SYN, SYN_REPORT, 0},                // 14 - 同步报告
        
        {EV_ABS, ABS_MT_POSITION_Y, 0x377},     // 15 - Y坐标 887
        {EV_SYN, SYN_REPORT, 0},                // 16 - 同步报告
        
        {EV_ABS, ABS_MT_POSITION_Y, 0x378},     // 17 - Y坐标 888
        {EV_SYN, SYN_REPORT, 0},                // 18 - 同步报告
        
        {EV_ABS, ABS_MT_POSITION_Y, 0x379},     // 19 - Y坐标 889
        {EV_SYN, SYN_REPORT, 0},                // 20 - 同步报告
        
        {EV_ABS, ABS_MT_POSITION_X, 0x11d},     // 21 - X坐标 285
        {EV_ABS, ABS_MT_POSITION_Y, 0x37a},     // 22 - Y坐标 890
        {EV_ABS, ABS_MT_TOUCH_MAJOR, 0x40},     // 23 - 触摸大小 64
        {EV_SYN, SYN_REPORT, 0},                // 24 - 同步报告
        
        {EV_ABS, ABS_MT_TOUCH_MAJOR, 0x41},     // 25 - 触摸大小 65
        {EV_SYN, SYN_REPORT, 0},                // 26 - 同步报告
        
        {EV_ABS, ABS_MT_POSITION_Y, 0x37b},     // 27 - Y坐标 891
        {EV_SYN, SYN_REPORT, 0},                // 28 - 同步报告
        
        {EV_ABS, ABS_MT_TRACKING_ID, -1},       // 31 - 结束跟踪 (0xffffffff)
        {EV_KEY, BTN_TOUCH, 0},                 // 32 - 触摸抬起
        {EV_SYN, SYN_REPORT, 0},                // 33 - 同步报告

    };
    
    EventCount = sendEvents(d, mts, sizeof(mts)/sizeof(MTEvent), OutEvents);
    
    // 根据原始数据，应该有多个MOVE事件
    ASSERT_EQ(EventCount, 12); // DOWN + 8个MOVE + UP
    
    // 检查DOWN事件
    ASSERT_EQ(OutEvents[0]->getAction(), MotionEvent::ACTION_DOWN);
    ASSERT_EQ(OutEvents[0]->getPointerId(0), 0);
    ASSERT_EQ(OutEvents[0]->getPointerCount(), 1);
    ASSERT_EQ(OutEvents[0]->getX(0), 0x11f); // 287
    ASSERT_EQ(OutEvents[0]->getY(0), 0x35e); // 862
    
    // 检查MOVE事件序列
    ASSERT_EQ(OutEvents[1]->getAction(), MotionEvent::ACTION_MOVE);
    ASSERT_EQ(OutEvents[1]->getX(0), 0x11f); // X保持不变
    ASSERT_EQ(OutEvents[1]->getY(0), 0x364); // Y: 868
    
    ASSERT_EQ(OutEvents[2]->getAction(), MotionEvent::ACTION_MOVE);
    ASSERT_EQ(OutEvents[2]->getX(0), 0x11f); // X保持不变
    ASSERT_EQ(OutEvents[2]->getY(0), 0x36b); // Y: 875
    
    ASSERT_EQ(OutEvents[3]->getAction(), MotionEvent::ACTION_MOVE);
    ASSERT_EQ(OutEvents[3]->getX(0), 0x11f); // X保持不变
    ASSERT_EQ(OutEvents[3]->getY(0), 0x373); // Y: 883
    
    ASSERT_EQ(OutEvents[4]->getAction(), MotionEvent::ACTION_MOVE);
    ASSERT_EQ(OutEvents[4]->getX(0), 0x11e); // X: 286
    ASSERT_EQ(OutEvents[4]->getY(0), 0x375); // Y: 885
    
    ASSERT_EQ(OutEvents[5]->getAction(), MotionEvent::ACTION_MOVE);
    ASSERT_EQ(OutEvents[5]->getX(0), 0x11e); // X保持不变
    ASSERT_EQ(OutEvents[5]->getY(0), 0x377); // Y: 887
    
    ASSERT_EQ(OutEvents[6]->getAction(), MotionEvent::ACTION_MOVE);
    ASSERT_EQ(OutEvents[6]->getX(0), 0x11e); // X保持不变
    ASSERT_EQ(OutEvents[6]->getY(0), 0x378); // Y: 888
    
    ASSERT_EQ(OutEvents[7]->getAction(), MotionEvent::ACTION_MOVE);
    ASSERT_EQ(OutEvents[7]->getX(0), 0x11e); // X保持不变
    ASSERT_EQ(OutEvents[7]->getY(0), 0x379); // Y: 889
    
    ASSERT_EQ(OutEvents[8]->getAction(), MotionEvent::ACTION_MOVE);
    ASSERT_EQ(OutEvents[8]->getX(0), 0x11d); // X: 285
    ASSERT_EQ(OutEvents[8]->getY(0), 0x37a); // Y: 890

    ASSERT_EQ(OutEvents[9]->getAction(), MotionEvent::ACTION_MOVE);
    ASSERT_EQ(OutEvents[9]->getX(0), 0x11d); // X: 285
    ASSERT_EQ(OutEvents[9]->getY(0), 0x37a); // Y: 890

    ASSERT_EQ(OutEvents[10]->getAction(), MotionEvent::ACTION_MOVE);
    ASSERT_EQ(OutEvents[10]->getX(0), 0x11d); // X: 285
    ASSERT_EQ(OutEvents[10]->getY(0), 0x37b); // Y: 891
    
    // 检查UP事件
    ASSERT_EQ(OutEvents[11]->getAction(), MotionEvent::ACTION_UP);
    ASSERT_EQ(OutEvents[11]->getX(0), 0x11d); // 285
    ASSERT_EQ(OutEvents[11]->getY(0), 0x37b); // 891
}


TEST_F(INPUTDEVICE, MTB6) {
   TouchDevice d(INJECTDEV_TOUCH);
   MTEvent mts[] = {
      // 第一个触点按下 (Slot 0)
      {EV_ABS, ABS_MT_TRACKING_ID, 0x123},    // 0 - 触点1 ID
      {EV_ABS, ABS_MT_POSITION_X, 0x7c},      // 1 - 触点1 X: 124
      {EV_ABS, ABS_MT_POSITION_Y, 0x2a2},     // 2 - 触点1 Y: 674
      {EV_ABS, ABS_MT_TOUCH_MAJOR, 0x37},     // 3 - 触点1 触摸大小
      {EV_KEY, BTN_TOUCH, 1},                 // 4 - 触摸按下
      {EV_SYN, SYN_REPORT, 0},                // 5 - 同步报告
      
      // 第二个触点按下 (切换到Slot 1)
      {EV_ABS, ABS_MT_SLOT, 1},               // 6 - 切换到触点2
      {EV_ABS, ABS_MT_TRACKING_ID, 0x124},    // 7 - 触点2 ID
      {EV_ABS, ABS_MT_POSITION_X, 0xae},      // 8 - 触点2 X: 174
      {EV_ABS, ABS_MT_POSITION_Y, 0x4c7},     // 9 - 触点2 Y: 1223
      {EV_ABS, ABS_MT_TOUCH_MAJOR, 0x53},     // 10 - 触点2 触摸大小
      {EV_SYN, SYN_REPORT, 0},                // 11 - 同步报告
      
      //  第1次移动
      {EV_ABS, ABS_MT_POSITION_X, 200},       // 12 - 触点2 X: 200
      {EV_ABS, ABS_MT_POSITION_Y, 1300},      // 13 - 触点2 Y: 1300
      {EV_SYN, SYN_REPORT, 0},                // 14 - 同步报告
      
      // 第2次移动
      {EV_ABS, ABS_MT_POSITION_X, 120},       // 15 - 触点2 X: 120
      {EV_ABS, ABS_MT_POSITION_Y, 1500},      // 16 - 触点2 Y: 1500
      {EV_SYN, SYN_REPORT, 0},                // 17 - 同步报告
      
      // 第3次移动
      {EV_ABS, ABS_MT_SLOT, 0},               // 18 - 切换到触点1
      {EV_ABS, ABS_MT_POSITION_X, 50},        // 19 - 触点2 X: 50
      {EV_ABS, ABS_MT_POSITION_Y, 500},       // 20 - 触点2 Y: 500
      {EV_SYN, SYN_REPORT, 0},                // 21 - 同步报告
      
      // 第4次移动
      {EV_ABS, ABS_MT_POSITION_X, 60},        // 22 - 触点2 X: 60
      {EV_ABS, ABS_MT_POSITION_Y, 600},       // 23 - 触点2 Y: 600
      {EV_SYN, SYN_REPORT, 0},                // 24 - 同步报告
      
      // 第5次移动
      {EV_ABS, ABS_MT_SLOT, 1},               // 25 - 切换到触点2
      {EV_ABS, ABS_MT_POSITION_X, 300},       // 26 - 触点2 X: 300
      {EV_ABS, ABS_MT_POSITION_Y, 1200},      // 27 - 触点2 Y: 1200
      {EV_SYN, SYN_REPORT, 0},                // 28 - 同步报告

      {EV_ABS, ABS_MT_TRACKING_ID, -1},       // 29 - 结束跟踪 (0xffffffff)
      {EV_SYN, SYN_REPORT, 0},                // 30 - 同步报告
      
      {EV_ABS, ABS_MT_SLOT, 0},               // 31
      {EV_ABS, ABS_MT_TRACKING_ID, -1},       // 32 - 结束跟踪 (0xffffffff)
      {EV_KEY, BTN_TOUCH, 0},                 // 33 - 触摸抬起
      {EV_SYN, SYN_REPORT, 0},                // 34 - 同步报告

   };
   
   EventCount = sendEvents(d, mts, sizeof(mts)/sizeof(MTEvent), OutEvents);
   
   // 验证多点触控事件处理
   ASSERT_EQ(EventCount, 9);

   // 第一个触点按下事件
   ASSERT_EQ(OutEvents[0]->getActionMasked(),MotionEvent::ACTION_DOWN);
   ASSERT_EQ(OutEvents[0]->getActionIndex(),0);
   ASSERT_EQ(OutEvents[0]->getPointerCount(),1);
   ASSERT_EQ(OutEvents[0]->getX(0),mts[1].value);  // 触点1 X: 124
   ASSERT_EQ(OutEvents[0]->getY(0),mts[2].value);  // 触点1 Y: 674
   
   // 第二个触点按下事件（应该触发ACTION_POINTER_DOWN）
   ASSERT_EQ(OutEvents[1]->getActionMasked(), MotionEvent::ACTION_POINTER_DOWN);
   ASSERT_EQ(OutEvents[1]->getActionIndex(), 1);
   ASSERT_EQ(OutEvents[1]->getPointerCount(), 2);
   ASSERT_EQ(OutEvents[1]->getX(0), mts[1].value);   
   ASSERT_EQ(OutEvents[1]->getY(0), mts[2].value);   
   ASSERT_EQ(OutEvents[1]->getX(1), mts[8].value);   // 触点2 X: 174
   ASSERT_EQ(OutEvents[1]->getY(1), mts[9].value);   // 触点2 Y: 1223
   
   // 验证移动事件 1
   ASSERT_EQ(OutEvents[2]->getActionMasked(), MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[2]->getActionIndex(), 1);
   ASSERT_EQ(OutEvents[2]->getPointerCount(), 2);
   ASSERT_EQ(OutEvents[2]->getX(0), mts[1].value);    // 触点1 X: 124
   ASSERT_EQ(OutEvents[2]->getY(0), mts[2].value);    // 触点1 Y: 674
   ASSERT_EQ(OutEvents[2]->getX(1), mts[12].value);   // 触点2 X: 200
   ASSERT_EQ(OutEvents[2]->getY(1), mts[13].value);   // 触点2 Y: 1300

   // 验证移动事件 2
   ASSERT_EQ(OutEvents[3]->getActionMasked(), MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[3]->getActionIndex(), 1);
   ASSERT_EQ(OutEvents[3]->getPointerCount(), 2);
   ASSERT_EQ(OutEvents[3]->getX(0), mts[1].value);    // 触点1 X: 124
   ASSERT_EQ(OutEvents[3]->getY(0), mts[2].value);    // 触点1 Y: 674
   ASSERT_EQ(OutEvents[3]->getX(1), mts[15].value);   // 触点2 X: 120
   ASSERT_EQ(OutEvents[3]->getY(1), mts[16].value);   // 触点2 Y: 1500

   // 验证移动事件 3
   ASSERT_EQ(OutEvents[4]->getActionMasked(), MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[4]->getActionIndex(), 0);
   ASSERT_EQ(OutEvents[4]->getPointerCount(), 2);
   ASSERT_EQ(OutEvents[4]->getX(0), mts[19].value);   // 触点1 X: 50
   ASSERT_EQ(OutEvents[4]->getY(0), mts[20].value);   // 触点1 Y: 500
   ASSERT_EQ(OutEvents[4]->getX(1), mts[15].value);   // 触点2 X: 120
   ASSERT_EQ(OutEvents[4]->getY(1), mts[16].value);   // 触点2 Y: 1500

   // 验证移动事件 4
   ASSERT_EQ(OutEvents[5]->getActionMasked(), MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[5]->getActionIndex(), 0);
   ASSERT_EQ(OutEvents[5]->getPointerCount(), 2);
   ASSERT_EQ(OutEvents[5]->getX(0), mts[22].value);   // 触点1 X: 60
   ASSERT_EQ(OutEvents[5]->getY(0), mts[23].value);   // 触点1 Y: 600
   ASSERT_EQ(OutEvents[5]->getX(1), mts[15].value);   // 触点2 X: 120
   ASSERT_EQ(OutEvents[5]->getY(1), mts[16].value);   // 触点2 Y: 1500

   // 验证移动事件 5
   ASSERT_EQ(OutEvents[6]->getActionMasked(), MotionEvent::ACTION_MOVE);
   ASSERT_EQ(OutEvents[6]->getActionIndex(), 1);
   ASSERT_EQ(OutEvents[6]->getPointerCount(), 2);
   ASSERT_EQ(OutEvents[6]->getX(0), mts[22].value);   // 触点1 X: 60
   ASSERT_EQ(OutEvents[6]->getY(0), mts[23].value);   // 触点1 Y: 600
   ASSERT_EQ(OutEvents[6]->getX(1), mts[26].value);   // 触点2 X: 300
   ASSERT_EQ(OutEvents[6]->getY(1), mts[27].value);   // 触点2 Y: 1200
   
   // 验证第二个触点抬起事件
   ASSERT_EQ(OutEvents[7]->getActionMasked(),MotionEvent::ACTION_POINTER_UP);
   ASSERT_EQ(OutEvents[7]->getActionIndex(), 1);
   ASSERT_EQ(OutEvents[7]->getPointerCount(), 2);
   ASSERT_EQ(OutEvents[7]->getX(0), mts[22].value);   // 触点1 X: 60
   ASSERT_EQ(OutEvents[7]->getY(0), mts[23].value);   // 触点1 Y: 600
   ASSERT_EQ(OutEvents[7]->getX(1), mts[26].value);   // 触点2 X: 300
   ASSERT_EQ(OutEvents[7]->getY(1), mts[27].value);   // 触点2 Y: 1200

   // 验证第二个触点抬起事件
   ASSERT_EQ(OutEvents[8]->getActionMasked(),MotionEvent::ACTION_UP);
   ASSERT_EQ(OutEvents[8]->getActionIndex(), 0);
   ASSERT_EQ(OutEvents[8]->getPointerCount(), 1);
   ASSERT_EQ(OutEvents[8]->getX(0), mts[22].value);   // 触点1 X: 60
   ASSERT_EQ(OutEvents[8]->getY(0), mts[23].value);   // 触点1 Y: 600

}
