#include <stdio.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <vector>
#include <list>
#include <core/app.h>
#include <view/view.h>
#include <widget/button.h>
#include <widget/edittext.h>
#include <widget/linearlayout.h>
#include <widget/cdwindow.h>
#include <porting/cdlog.h>
using namespace cdroid;
#include <guienvironment.h>

class KEYNAV:public testing::Test{
public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(KEYNAV,btns){
   App&app=App::getInstance();
   ViewGroup*w=GUIEnvironment::content();
   for(int i=0;i<8;i++){
       Button*btn=new Button("Button_"+std::to_string(i),200,50);
       btn->setTextColor(app.getColorStateList("cdroid:color/textview.xml"));
       w->addView(btn);
       btn->setId(100+i);
       btn->layout(10,i*55,200,50);
   }
   View*fv=w->focusSearch(nullptr,View::FOCUS_DOWN);
   while(fv){
      LOGD("focusedview=%p:%d direction=%d \r\n",fv,fv->getId(),View::FOCUS_DOWN);
      fv=w->focusSearch(fv,View::FOCUS_DOWN);
   }
   pumpFor(500);
}
TEST_F(KEYNAV,edts){
   App&app=App::getInstance();
   ViewGroup*w=GUIEnvironment::content();
   for(int i=0;i<6;i++){
       EditText*tv=new EditText("EditText_"+std::to_string(i),200,50);
       w->addView(tv);
       tv->layout(10,i*55,200,50);
   }
   pumpFor(500);
}
TEST_F(KEYNAV,edts1){
   App&app=App::getInstance();
   ViewGroup*w=GUIEnvironment::content();
   LinearLayout*ll1=new LinearLayout(800,200);
   LinearLayout*ll2=new LinearLayout(800,200);
   w->addView(ll1);
   w->addView(ll2);
   ll2->layout(0,210,800,200);
   ll2->setBackgroundColor(0xFF111111);
   for(int i=0;i<6;i++){
       EditText*tv=new EditText("EditText_"+std::to_string(i)+":"+std::to_string((i+1)%6)+std::to_string((i-1+6)%6),600,50);
       tv->setNextFocusDownId((i+1)%6);
       tv->setNextFocusUpId((i+5)%6);
       tv->setId(i);
       if(i<3){ll1->addView(tv);tv->layout(10,10+i*55,600,50);}
       else{ll2->addView(tv);tv->layout(10,10+(i-3)*55,600,50);}
   }
   View*fv=w->focusSearch(nullptr,View::FOCUS_DOWN);
   int loop=0;
   while(fv && loop++<=6){
      LOGD("focusedview=%p:%d FOCUS_DOWN \r\n",fv,fv->getId());
      fv=w->focusSearch(fv,View::FOCUS_DOWN);
      ASSERT_EQ((fv->getId()+1)%6,fv->getNextFocusDownId());
   }
   fv=w->focusSearch(nullptr,View::FOCUS_UP);
   loop=0;
   while(fv && loop++<=6){
      LOGD("focusedview=%p:%d FOCUS_UP \r\n",fv,fv->getId());
      fv=w->focusSearch(fv,View::FOCUS_UP);
      ASSERT_EQ((fv->getId()+5)%6,fv->getNextFocusUpId());
   }

   pumpFor(500);
}


