#include <gtest/gtest.h>
#include <windows.h>
#include <ngl_os.h>

using namespace cdroid;

class TEXTLAYOUT:public testing::Test{

public :
   static void SetUpTestCase(){
       GFXInit();
       GraphDevice::getInstance().createContext(1280,720);
   }
   static void TearDownCase(){
   }

   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};


TEST_F(TEXTLAYOUT,singleline){
    App app;
    Layout ll(30,400,0);
    ll.setMultiline(false);
    ll.setText("Hello wordld!");
    ll.relayout();
}
TEST_F(TEXTLAYOUT,multiline2){
    App app;
    Layout ll(30,400,0);
    ll.setMultiline(true);
    ll.setText("\nline1\nline2");
    ll.relayout();
     
}
