#include <gtest/gtest.h>
#include <view/keyevent.h>
using namespace cdroid;
class KEYEVENT:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(KEYEVENT,keyCodeToString){
    EXPECT_STREQ(KeyEvent::keyCodeToString(KeyEvent::KEYCODE_UNKNOWN).c_str(),"KEYCODE_UNKNOWN");
    EXPECT_STREQ(KeyEvent::keyCodeToString(KeyEvent::KEYCODE_F1).c_str(),"KEYCODE_F1");
}

TEST_F(KEYEVENT,keyCodeFromString){
    EXPECT_EQ(KeyEvent::keyCodeFromString("KEYCODE_UNKNOWN"),KeyEvent::KEYCODE_UNKNOWN);
    EXPECT_EQ(KeyEvent::keyCodeFromString("KEYCODE_F24"),KeyEvent::KEYCODE_F24);
}
