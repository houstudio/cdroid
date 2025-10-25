#include <stdio.h>
#include <gtest/gtest.h>
#include <porting/cdinput.h>
#if defined(__linux__)||defined(__unix__)
#include <linux/input.h>
#else
#include <core/eventcodes.h>
#endif
#include <fcntl.h>
class INPUT:public testing::Test{
   public :
   virtual void SetUp(){
       InputInit();
       printf("inputevent.size=%ld\r\n",sizeof(INPUTEVENT));
   }
   virtual void TearDown(){}
};

#define test_bit(bit, array)    ((array)[(bit)/8] & (1<<((bit)%8)))
TEST_F(INPUT,GetDeviceInfo){
   INPUTDEVICEINFO info;
   InputGetDeviceInfo(INJECTDEV_KEY,&info);
   ASSERT_TRUE(test_bit(EV_KEY,info.keyBitMask));
   ASSERT_GT(strlen(info.name),0);
   InputGetDeviceInfo(INJECTDEV_TOUCH,&info);
   ASSERT_GT(strlen(info.name),0);
   ASSERT_TRUE(test_bit(EV_ABS,info.absBitMask));
}

TEST_F(INPUT,InjectEvent){
   INPUTEVENT e;
   INPUTEVENT e2;
   e.type=EV_KEY;
   e.code = 100;// KEY_ENTER;
   e.device=INJECTDEV_KEY;
   ASSERT_EQ(1,InputInjectEvents(&e,1,0));
   ASSERT_EQ(1,InputGetEvents(&e2,1,1));
   ASSERT_EQ(e.type,e2.type);
   ASSERT_EQ(e.code,e2.code);
   ASSERT_EQ(e.value,e2.value);
}

TEST_F(INPUT,GetKey){
   int i=0;
   int rc=0;
   while(i++<100){
      INPUTEVENT keys[16];
      rc=InputGetEvents(keys,16,500);
      for(int j=0;j<rc;j++){
	  printf("%02d:[%3d,%3d,%3d]\r\n",j,keys[j].type,keys[j].code,keys[j].value);
      }
      if(rc)printf("\r\n");
   }
   ASSERT_GT(rc,0);
}
TEST_F(INPUT,AxisInfo){
    int axises=0,i=0;
    while(i++<10){
      INPUTEVENT keys[16];
      INPUTDEVICEINFO info;
      if(0<InputGetEvents(keys,16,500)){
	   InputGetDeviceInfo(keys[0].device,&info);
	   for(int j=0;(j<ABS_CNT)&&(j<sizeof(info.axis)/sizeof(INPUTAXISINFO));j++){
	       INPUTAXISINFO*a=info.axis+j;
	       if(std::abs(a->maximum-a->minimum)>0){
		  axises++;
	          printf("axis[%d] range=[%d,%d]\r\n",a->axis,a->minimum,a->maximum);
	       }
	   }
      }
   }
    ASSERT_GT(axises,0);
}
