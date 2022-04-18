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


