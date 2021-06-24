#include <gtest/gtest.h>
#include <windows.h>
#include <ngl_os.h>
#include <ngl_timer.h>


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


