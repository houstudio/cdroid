#include <gtest/gtest.h>
#include <windows.h>
#include <ngl_os.h>
#include <ngl_timer.h>
#include <core/textutils.h>

class EDITTEXT:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(EDITTEXT,edit){
    App app;
    int format[]={Gravity::LEFT,Gravity::CENTER_HORIZONTAL,Gravity::RIGHT};
    Window*w=new Window(100,50,800,640);
    for(int i=0;i<sizeof(format)/sizeof(format[0]);i++){
        EditText*g=new EditText("Hello world!",600,32);
        g->setGravity(format[i]|Gravity::CENTER_VERTICAL);
        w->addView(g).setPos(10,35*i+10);
    }
    app.exec();
}

TEST_F(EDITTEXT,multiline){
    App app;
    Window*w=new Window(100,50,800,640);
    EditText*edt=new EditText("Hello world!\nThis is the second line\n The last line",400,200);
    edt->setSingleLine(false);
    edt->setTextColor(0xFFFFFFFF);
    edt->setBackgroundDrawable(new ColorDrawable(0xFF111111));
    w->addView(edt).setSize(600,200);
    app.exec();
}

TEST_F(EDITTEXT,hebrew){
    App app;
    const wchar_t text[]={0xfb49,0x05d0,0x05ad,0x0020,0x05d0,0x059d,0x0020,0x05d0,0x0599,0x0020,
             0x0020,0x05d0,0x05b6,0x0020,0xfb30,0x0020,0x05d0,0x0592,0x0020,0x05dc,
             0x05ad,0x0020,0x05dc,0x059d,0x0020,0x05dc,0x0599,0x0020,0x05dc,0x05b6,
             0x0020,0xfb3c,0x0020,0x05dc,0x0592,0x00};
    std::string u8text=TextUtils::unicode2utf8(text);
    Window*w=new Window(100,50,800,640);
    EditText*edt=new EditText(u8text,400,200);
    edt->setTextSize(40);
    edt->setSingleLine(false);
    edt->setTextColor(0xFFFFFFFF);
    edt->setBackgroundDrawable(new ColorDrawable(0xFF111111));
    w->addView(edt).setSize(600,200);
    app.exec();

}
TEST_F(EDITTEXT,hindi){//Ó¡µØÓï
    const char*text="?? ???? ?? ??????? ??? ??";
    App app;
    Window*w=new Window(100,50,800,640);
    EditText*edt=new EditText(text,400,200);
    edt->setTextSize(40);
    edt->setSingleLine(false);
    edt->setTextColor(0xFFFFFFFF);
    edt->setBackgroundDrawable(new ColorDrawable(0xFF111111));
    w->addView(edt).setSize(600,200);
    app.exec();
}
