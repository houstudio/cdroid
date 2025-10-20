#include <gtest/gtest.h>
#if defined(__linux__)||defined(__unix__) 
#include <sys/time.h>
#elif defined(_WIN32)||defined(_WIN64)
#include <WinSock2.h>
extern void gettimeofday(struct timeval* t1, struct timezone* zone);
#undef RECT
#undef SIZE
#undef POINT
#undef RGB
#undef IN
#undef OUT
#undef WINDING
#undef TRANSPARENT
#undef OPAQUE
#undef INFINITE
#undef ABSOLUTE
#endif
#include <cdroid.h>

struct MTEvent{int type,code,value;};
class MOTIONEVENT:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(MOTIONEVENT,Coords){
   for(int i=0;i<100;i++){
       PointerCoords *p=new PointerCoords;
       delete p;
   }
}

TEST_F(MOTIONEVENT,Alloc_Free){
   PointerCoords coords[128];
   PointerCoords*pc=coords;
   PointerProperties ptprops[128];
   for(int i=0;i<100;i++){
      MotionEvent*m=new MotionEvent();
      delete m;
   }
   for(int i=0;i<100;i++){
      MotionEvent*m=new MotionEvent();
      m->initialize(0,0,0,0,0,0,0,0,0,0,0.f,0.f,0.f,0.f,0,0,0,0,i%32,ptprops,coords);
      delete m;
   }
}

TEST_F(MOTIONEVENT,Time){
    PointerCoords coords[2];
    PointerProperties props[2];
    nsecs_t mMoveTime=SystemClock::uptimeMillis();
    for(int i=0;i<100000;i++){
        coords[0].setAxisValue(MotionEvent::AXIS_X,i);
        coords[0].setAxisValue(MotionEvent::AXIS_Y,i*2);
        MotionEvent*e=MotionEvent::obtain(mMoveTime , mMoveTime , 0 , 1,props,coords, 0/*metaState*/,0,
                     0,0/*x/yPrecision*/,0/*deviceId*/, 0/*edgeFlags*/, 0, 0/*flags*/,0);
        ASSERT_EQ(mMoveTime,e->getEventTime());
        ASSERT_EQ(e->getX(),i);
        ASSERT_EQ(e->getY(),i*2);
        e->recycle();
    }
}

TEST_F(MOTIONEVENT,offset){
    PointerCoords coords[2];
    PointerProperties props[2];
    coords[0].setAxisValue(MotionEvent::AXIS_X,100);
    coords[0].setAxisValue(MotionEvent::AXIS_Y,200);
    MotionEvent*e = MotionEvent::obtain(0,0,0,1,props,coords, 0/*metaState*/,0,
            0,0/*x/yPrecision*/,0/*deviceId*/, 0/*edgeFlags*/, 
            InputDevice::SOURCE_CLASS_POINTER,/*offset must be pointer/touch class*/
            0/*flags*/,0/*classification*/);
    ASSERT_EQ(e->getX(),100);
    ASSERT_EQ(e->getY(),200);
    e->offsetLocation(10,20);
    ASSERT_EQ(e->getX(),110);
    ASSERT_EQ(e->getY(),220);
    e->offsetLocation(-10,-20);
    ASSERT_EQ(e->getX(),100);
    ASSERT_EQ(e->getY(),200);
}

TEST_F(MOTIONEVENT,Rotation) {
    // The un-rotated frame size.
    constexpr int width = 600;
    constexpr int height = 1000;
    MotionEvent* event = MotionEvent::obtain(0 /* downTime */, 0 /* eventTime */,
            MotionEvent::ACTION_DOWN, 30 /* x */, 50 /* y */, 0 /* metaState */);
    event->setSource(InputDevice::SOURCE_TOUCHSCREEN);
    ASSERT_EQ(0/*Surface.ROTATION_0*/, event->getSurfaceRotation());

    MotionEvent* rot90 = MotionEvent::obtain(*event);
    rot90->transform(MotionEvent::createRotateMatrix(1/*Surface::ROTATION_90*/, height, width));
    ASSERT_EQ(50, (int) rot90->getX());
    ASSERT_EQ(570, (int) rot90->getY());
    ASSERT_EQ(1/*Surface::ROTATION_90*/, rot90->getSurfaceRotation());
    rot90->recycle();

    MotionEvent* rot180 = MotionEvent::obtain(*event);
    rot180->transform(MotionEvent::createRotateMatrix(2/*Surface::ROTATION_180*/, width, height));
    ASSERT_EQ(570, (int) rot180->getX());
    ASSERT_EQ(950, (int) rot180->getY());
    ASSERT_EQ(2/*Surface::ROTATION_180*/, rot180->getSurfaceRotation());
    rot180->recycle();

    MotionEvent* rot270 = MotionEvent::obtain(*event);
    rot270->transform(MotionEvent::createRotateMatrix(3/*Surface::ROTATION_270*/, height, width));
    ASSERT_EQ(950, (int) rot270->getX());
    ASSERT_EQ(30, (int) rot270->getY());
    ASSERT_EQ(3/*Surface.ROTATION_270*/, rot270->getSurfaceRotation());
    rot270->recycle();

    MotionEvent* compoundRot = MotionEvent::obtain(*event);
    compoundRot->transform(MotionEvent::createRotateMatrix(1/*Surface::ROTATION_90*/, height, width));
    compoundRot->transform(MotionEvent::createRotateMatrix(2/*Surface::ROTATION_180*/, height, width));
    ASSERT_EQ(950, (int) compoundRot->getX());
    ASSERT_EQ(30, (int) compoundRot->getY());
    ASSERT_EQ(3/*Surface::ROTATION_270*/, compoundRot->getSurfaceRotation());
    compoundRot->recycle();

    MotionEvent* rotInvalid = MotionEvent::obtain(*event);
    //mat.setValues(new float[]{1, 2, 3, -4, -5, -6, 0, 0, 1});
    rotInvalid->transform({1, 2, 3, -4, -5, -6, 0, 0, 1});
    ASSERT_EQ(-1, rotInvalid->getSurfaceRotation());
    rotInvalid->recycle();
    event->recycle();
}

TEST_F(MOTIONEVENT,EventPool){
    PooledInputEventFactory pool(32);
    PointerCoords coords[128];
    PointerCoords*pc=coords;
    PointerProperties ptprops[128];
    for(int i=0;i<10000;i++){
        MotionEvent*m=pool.createMotionEvent();
        m->initialize(0,0,0,0,0,0,0,0,0,0,0.f,0.f,0.f,0.f,0,0,0,0,i%32,ptprops,coords);
        pool.recycle(m);
    }
}

TEST_F(MOTIONEVENT,Benchmark){
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

TEST_F(MOTIONEVENT,exec){
   static const char*args[]={"arg1","alpha",NULL};
   App app(2,args);
   app.exec();
}
