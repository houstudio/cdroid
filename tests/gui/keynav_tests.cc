#include <stdio.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <vector>
#include <list>
#include <ngl_os.h>
#include <cdroid.h>

class KEYNAV:public testing::Test{
public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(KEYNAV,btns){
   App app;
   Window*w=new Window(0,0,800,600);
   for(int i=0;i<8;i++){
       Button*btn=new Button("Button_"+std::to_string(i),200,50);
       btn->setTextColor(app.getColorStateList("cdroid:color/textview.xml"));
       w->addView(btn).setId(100+i).setPos(10,i*55);
   }
   View*fv=w->focusSearch(nullptr,View::FOCUS_DOWN);
   while(fv){
      LOGD("focusedview=%p:%d direction=%d \r\n",fv,fv->getId(),View::FOCUS_DOWN);
      fv=w->focusSearch(fv,View::FOCUS_DOWN);
   }
   app.exec();
}
TEST_F(KEYNAV,edts){
   App app;
   Window*w=new Window(0,0,800,600);
   for(int i=0;i<6;i++){
       EditText*tv=new EditText("EditText_"+std::to_string(i),200,50);
       w->addView(tv).setPos(10,i*55);
   }
   app.exec();
}
TEST_F(KEYNAV,edts1){
   App app;
   Window*w=new Window(0,0,800,600);
   LinearLayout*ll1=new LinearLayout(800,200);
   LinearLayout*ll2=new LinearLayout(800,200);
   w->addView(ll1);
   w->addView(ll2).setPos(0,210);
   ll2->setBackgroundColor(0xFF111111);
   for(int i=0;i<6;i++){
       EditText*tv=new EditText("EditText_"+std::to_string(i)+":"+std::to_string((i+1)%6)+std::to_string((i-1+6)%6),600,50);
       tv->setNextFocusDownId((i+1)%6);
       tv->setNextFocusUpId((i+5)%6);
       if(i<3)ll1->addView(tv).setId(i).setPos(10,10+i*55);
       else ll2->addView(tv).setId(i).setPos(10,10+(i-3)*55);
   }
   View*fv=w->focusSearch(nullptr,View::FOCUS_DOWN);
   int loop=0;
   while(fv && loop++<=6){
      LOGD("focusedview=%p:%d direction=%d \r\n",fv,fv->getId(),View::FOCUS_DOWN);
      fv=w->focusSearch(fv,View::FOCUS_DOWN);
   }


