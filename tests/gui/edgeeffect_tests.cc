#include <gtest/gtest.h>
#include <core/app.h>
#include <view/view.h>
#include <view/gravity.h>
#include <core/canvas.h>
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

/* EdgeEffect drawn in a plain View hosted by the shared content area (was a
   private Window subclass). EdgeEffect just needs a Canvas, so a View's onDraw
   works the same; it is constructed with the App context directly. */
class EdgeView:public View{
public:
   EdgeEffect *mEF;
public:
   EdgeView(int w,int h):View(w,h){
       mEF=new EdgeEffect(&App::getInstance());
       mEF->setSize(w,100);
   }
   void onDraw(Canvas&canvas){
       View::onDraw(canvas);
       mEF->draw(canvas);
   }
   void setType(bool top){
       mEF->onPull(top?20.f:-20.f);
   }
};

TEST_F(EDGEEFFECT,top){
    EdgeView*w=new EdgeView(800,640);
    GUIEnvironment::content()->addView(w);
    w->setType(true);
    pumpFor(500);
}

TEST_F(EDGEEFFECT,bottom){
    EdgeView*w=new EdgeView(800,640);
    GUIEnvironment::content()->addView(w);
    w->setType(false);
    pumpFor(500);
}
