#include <gtest/gtest.h>
#include <windows.h>
#include <looper/TimerFD.h>
#include <ngl_os.h>

using namespace cdroid;

class RECTTEST:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};


TEST_F(RECTTEST,intersect){
    RECT rc1={0,0,100,100};
    RECT rc2={-40,-40,100,100};
    rc2.intersect(rc1);
    printf("%d,%d %d,%d\r\n",rc2.x,rc2.y,rc2.width,rc2.height);
}
TEST_F(RECTTEST,intersect2){
    RECT rc1={0,0,100,100};
    RECT rc2={-40,-40,100,100};
    rc1.intersect(rc2);
    printf("%d,%d %d,%d\r\n",rc1.x,rc1.y,rc1.width,rc1.height);
}
