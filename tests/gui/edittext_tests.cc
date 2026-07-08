#include <gtest/gtest.h>
#include <widget/edittext.h>
#include <widget/cdwindow.h>
#include <drawable/colordrawable.h>
#include <view/gravity.h>
#include <core/textutils.h>
#include <guienvironment.h>
using namespace cdroid;

class EDITTEXT:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(EDITTEXT,edit){
    App&app=App::getInstance();
    int format[]={Gravity::LEFT,Gravity::CENTER_HORIZONTAL,Gravity::RIGHT};
    Window*w=GUIEnvironment::stage();
    for(int i=0;i<sizeof(format)/sizeof(format[0]);i++){
        EditText*g=new EditText("Hello world!",600,32);
        g->setGravity(format[i]|Gravity::CENTER_VERTICAL);
        w->addView(g);
        g->layout(10,35*i+10,600,32);
    }
    pumpFor(500);
}

TEST_F(EDITTEXT,multiline){
    App&app=App::getInstance();
    Window*w=GUIEnvironment::stage();
    EditText*edt=new EditText("Hello world!\nThis is the second line\n The last line",400,200);
    edt->setSingleLine(false);
    edt->setTextColor(0xFFFFFFFF);
    edt->setBackground(new ColorDrawable(0xFF111111));
    w->addView(edt);
    pumpFor(500);
}

TEST_F(EDITTEXT,hebrew){
    App&app=App::getInstance();
    const wchar_t text[]={0xfb49,0x05d0,0x05ad,0x0020,0x05d0,0x059d,0x0020,0x05d0,0x0599,0x0020,
             0x0020,0x05d0,0x05b6,0x0020,0xfb30,0x0020,0x05d0,0x0592,0x0020,0x05dc,
             0x05ad,0x0020,0x05dc,0x059d,0x0020,0x05dc,0x0599,0x0020,0x05dc,0x05b6,
             0x0020,0xfb3c,0x0020,0x05dc,0x0592,0x00};
    std::string u8text=TextUtils::unicode2utf8(text);
    Window*w=GUIEnvironment::stage();
    EditText*edt=new EditText(u8text,400,200);
    edt->setTextSize(40);
    edt->setSingleLine(false);
    edt->setTextColor(0xFFFFFFFF);
    edt->setBackground(new ColorDrawable(0xFF111111));
    w->addView(edt);
    pumpFor(500);

}
TEST_F(EDITTEXT,hindi){//印地语
    const char*text="आज सुबह एक ट्रैफिक जैम था";
    App&app=App::getInstance();
    Window*w=GUIEnvironment::stage();
    EditText*edt=new EditText(text,400,200);
    edt->setTextSize(40);
    edt->setSingleLine(false);
    edt->setTextColor(0xFFFFFFFF);
    edt->setBackground(new ColorDrawable(0xFF111111));
    w->addView(edt);
    pumpFor(500);
}
