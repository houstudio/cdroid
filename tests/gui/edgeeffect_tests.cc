#include <gtest/gtest.h>
#include <cdroid.h>
#include <ngl_os.h>
#include <ngl_timer.h>
#include <widget/edgeeffect.h>

class EDGEEFFECT:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

class EdgeWindow:public Window{
public:
   EdgeEffect *mEF;
public:
   EdgeWindow(int x,int y,int w,int h):Window(x,y,w,h){
       mEF=new EdgeEffect(getContext());
       mEF->setSize(w,100);
   }
   void onDraw(Canvas&canvas){
       mEF->draw(canvas);
   }
   void setType(bool top){
       
       mEF->onPull(top?20:-20);
   }
};

TEST_F(EDGEEFFECT,top){
    App app;
    int format[]={Gravity::LEFT,Gravity::CENTER_HORIZONTAL,Gravity::RIGHT};
    EdgeWindow*w=new EdgeWindow(100,50,800,640);
    w->setType(true);
    app.exec();
}

TEST_F(EDGEEFFECT,bottom){
    App app;
    EdgeWindow*w=new EdgeWindow(100,50,800,640);
    w->setType(false);
    app.exec();
}


