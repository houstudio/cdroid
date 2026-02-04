#include <gtest/gtest.h>
#include <view/keyevent.h>
#include <view/motionevent.h>

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

TEST_F(KEYEVENT,metaStateToString){
    EXPECT_STREQ(KeyEvent::metaStateToString(KeyEvent::META_ALT_ON|KeyEvent::META_CAP_LOCKED).c_str(),
            "META_ALT_ON|META_CAP_LOCKED");
    EXPECT_STREQ(KeyEvent::metaStateToString(KeyEvent::META_ALT_LEFT_ON|KeyEvent::META_CTRL_RIGHT_ON).c_str(),
            "META_ALT_LEFT_ON|META_CTRL_RIGHT_ON");
}

const char*const axisName[]={
    "AXIS_X",    "AXIS_Y",    "AXIS_PRESSURE",    "AXIS_SIZE",
    "AXIS_TOUCH_MAJOR",   "AXIS_TOUCH_MINOR",
    "AXIS_TOOL_MAJOR",    "AXIS_TOOL_MINOR",
    "AXIS_ORIENTATION",
    "AXIS_VSCROLL",    "AXIS_HSCROLL",    "AXIS_Z",
    "AXIS_RX",    "AXIS_RY",    "AXIS_RZ",
    "AXIS_HAT_X",    "AXIS_HAT_Y",
    "AXIS_LTRIGGER", "AXIS_RTRIGGER",
    "AXIS_THROTTLE", "AXIS_RUDDER",
    "AXIS_WHEEL",    "AXIS_GAS",
    "AXIS_BRAKE",    "AXIS_DISTANCE",
    "AXIS_TILT",     "AXIS_SCROLL",
    "AXIS_RELATIVE_X",   "AXIS_RELATIVE_Y",
    "AXIS_RESERVED_29",  "AXIS_RESERVED_30",   "AXIS_RESERVED_31",
    "AXIS_GENERIC_1",    "AXIS_GENERIC_2",    "AXIS_GENERIC_3",
    "AXIS_GENERIC_4",    "AXIS_GENERIC_5",    "AXIS_GENERIC_6",
    "AXIS_GENERIC_7",    "AXIS_GENERIC_8",    "AXIS_GENERIC_9",
    "AXIS_GENERIC_10",   "AXIS_GENERIC_11",   "AXIS_GENERIC_12",
    "AXIS_GENERIC_13",   "AXIS_GENERIC_14",   "AXIS_GENERIC_15",
    "AXIS_GENERIC_16",
    "AXIS_GESTURE_X_OFFSET",    "AXIS_GESTURE_Y_OFFSET",
    "AXIS_GESTURE_SCROLL_X_DISTANCE",    "AXIS_GESTURE_SCROLL_Y_DISTANCE",
    "AXIS_GESTURE_PINCH_SCALE_FACTOR",   "AXIS_GESTURE_SWIPE_FINGER_COUNT"
};

TEST_F(KEYEVENT,axisToString){
    for(int i=MotionEvent::AXIS_X;i<=MotionEvent::AXIS_GESTURE_SWIPE_FINGER_COUNT;i++){
       EXPECT_STREQ(MotionEvent::axisToString(i).c_str(),axisName[i]);
    }
}

TEST_F(KEYEVENT,axisFromString){
    for(int i=MotionEvent::AXIS_X;i<=MotionEvent::AXIS_GESTURE_SWIPE_FINGER_COUNT;i++){
        EXPECT_EQ(MotionEvent::axisFromString(axisName[i]),i);
    }
}


