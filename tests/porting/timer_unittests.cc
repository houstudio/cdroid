#include <gtest/gtest.h>
#include <porting/dtvtimer.h>
#include <time.h>

class TIMER:public testing::Test{
   public :
   void*p;
   virtual void SetUp(){
      printf("sizeof NGL_TIME=%d time_t=%d\r\n",sizeof(NGL_TIME),sizeof(time_t));
   }
   virtual void TearDown(){}
};
#ifdef ENABLE_DTV
TEST(TIMER,SetTime){
   NGL_TIME t=0;
   nglGetTime(&t);
   t+=100L*3600*24*365;
   struct  tm tt;
   tt.tm_hour=18;
   tt.tm_min=18;
   tt.tm_sec=18;
   tt.tm_year=18;
   tt.tm_mon=5; //月份（从一月开始，0代表一月） - 取值区间为[0,11]
   tt.tm_mday=18;
   t=mktime(&tt);
   nglSetTime(&t);
}
#endif
