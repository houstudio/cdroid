#include <gtest/gtest.h>
#include <core/app.h>
#include <view/gravity.h>
#include <core/canvas.h>
#include <widget/cdwindow.h>
#include <widget/edgeeffect.h>
#include <guienvironment.h>
using namespace cdroid;

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
       
       mEF->onPull(top?20.f:-20.f);
   }
};

TEST_F(EDGEEFFECT,top){
    App&app=App::getInstance();
    int format[]={Gravity::LEFT,Gravity::CENTER_HORIZONTAL,Gravity::RIGHT};
    EdgeWindow*w=new EdgeWindow(100,50,800,640);
    w->setType(true);
    pumpFor(500);
}

TEST_F(EDGEEFFECT,bottom){
    App&app=App::getInstance();
    EdgeWindow*w=new EdgeWindow(100,50,800,640);
    w->setType(false);
    pumpFor(500);
}


