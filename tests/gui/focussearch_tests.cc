#include <stdio.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <vector>
#include <list>
#include <cdroid.h>

class FOCUS:public testing::Test{
public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(FOCUS,NOFOCUS){
    Window*w=new Window(0,0,800,600);
    for(int i=0;i<6;i++){
        TextView*tv=new TextView("TextView_"+std::to_string(i),200,50);
        w->addView(tv).setPos(10,i*55);
    }
    std::vector<View*>focus;
    w->addFocusables(focus,(int)View::FOCUS_LEFT,(int)View::FOCUSABLES_ALL);
    ASSERT_EQ(1,focus.size());
    focus.clear();

    w->setFocusable(false);
    w->addFocusables(focus,(int)View::FOCUS_RIGHT,(int)View::FOCUSABLES_ALL);
    ASSERT_EQ(0,focus.size());
}

 
TEST_F(FOCUS,ALLFOCUS){
    Window*w=new Window(100,100,800,600);
    for(int i=0;i<6;i++){
        EditText*tv=new EditText("EditText_"+std::to_string(i),200,50);
        w->addView(tv).setPos(10,i*55);
    }
    std::vector<View*>focus;
    w->addFocusables(focus,(int)View::FOCUS_LEFT,(int)View::FOCUSABLES_ALL);
    ASSERT_EQ(6,focus.size());
    focus.clear();
    w->addFocusables(focus,(int)View::FOCUS_RIGHT,(int)View::FOCUSABLES_ALL);
    ASSERT_EQ(6,focus.size());
}

TEST_F(FOCUS,HALFFOCUS){
    Window*w=new Window(100,100,800,600);
    for(int i=0;i<6;i++){
        View*tv;
        if(i%2==0)tv=new EditText("EditText_"+std::to_string(i),200,50);
        else tv=new TextView("TextView_"+std::to_string(i),200,50);
        tv->setFocusable(i%2==0);
        w->addView(tv).setPos(10,i*55);
    }
    std::vector<View*>focus;
    w->addFocusables(focus,(int)View::FOCUS_LEFT,(int)View::FOCUSABLES_ALL);
    ASSERT_EQ(3,focus.size());
    focus.clear();
    w->addFocusables(focus,(int)View::FOCUS_RIGHT,(int)View::FOCUSABLES_ALL);
    ASSERT_EQ(3,focus.size());
}

TEST_F(FOCUS,GROUP){
    Window*w=new Window(100,100,800,600);
    LinearLayout*l[2];
    l[0]=new LinearLayout(0,0,800,300);
    l[1]=new LinearLayout(0,310,800,200);
    for(int i=0;i<4;i++){
        View*tv;
        tv=new EditText("EditText_"+std::to_string(i),200,50);
        l[i%2]->addView(tv);
        tv=new TextView("TextView_"+std::to_string(i),200,50);
        l[i%2]->addView(tv).setFocusable(false);
    }
    w->addView(l[0]);
    w->addView(l[1]);

    std::vector<View*>focus;
    w->addFocusables(focus,View::FOCUS_LEFT,View::FOCUSABLES_ALL);
    View*fv=w->focusSearch(nullptr,View::FOCUS_RIGHT);
    ASSERT_EQ(4,focus.size());
    focus.clear();
    w->addFocusables(focus,(int)View::FOCUS_RIGHT,(int)View::FOCUSABLES_ALL);
    ASSERT_EQ(4,focus.size());
}

TEST_F(FOCUS,NAV_1_LAYER){
    Window*w=new Window(100,100,800,600);
    for(int i=0;i<6;i++){
        View*tv;
        if(i%2==0){
            tv=new EditText("EditText_"+std::to_string(i),200,50);
            w->addView(tv).setId(100+i/2).setPos(10,i*55);
        }else{ 
            tv=new TextView("TextView_"+std::to_string(i),200,50);
            w->addView(tv).setId(200+i/2).setPos(10,i*55).setFocusable(false);;
        }
    }
    std::vector<View*>focus;
    w->addFocusables(focus,View::FOCUS_LEFT,View::FOCUSABLES_ALL);
    ASSERT_EQ(3,focus.size());
  
    const int vids[]={100,101,102};
    View*fv=nullptr;
    for(int i=0;i<3;i++){
        fv=w->focusSearch(fv,View::FOCUS_DOWN);
        ASSERT_EQ(vids[i%3],fv->getId()); 
    }
}

TEST_F(FOCUS,NAV_2_LAYER){
    Window*w=new Window(100,100,800,600);
    LinearLayout*l[2];
    l[0]=new LinearLayout(0,0,800,300);
    l[1]=new LinearLayout(0,310,800,200);

    for(int i=0;i<3;i++){
        View*tv;
        LinearLayout*pl=l[0];
        tv=new EditText("EditText_"+std::to_string(i),200,50);
        pl->addView(tv).setId(100-1+pl->getChildCount());
        tv=new TextView("TextView_"+std::to_string(i),200,50);
        pl->addView(tv).setFocusable(false);
    }
    for(int i=0;i<3;i++){
        View*tv;
        LinearLayout*pl=l[1];
        tv=new EditText("EditText_"+std::to_string(i),200,50);
        pl->addView(tv).setId(200-1+pl->getChildCount());
        tv=new TextView("TextView_"+std::to_string(i),200,50);
        pl->addView(tv).setFocusable(false);
    }
    w->addView(l[0]);
    w->addView(l[1]);
    std::vector<View*>focus;
    w->addFocusables(focus,(int)View::FOCUS_RIGHT,(int)View::FOCUSABLES_ALL);
    ASSERT_EQ(6,focus.size());

    View*fv=nullptr;
    const int vids[]={100,102,104,200,202,204}; 
    for(int i=0;i<6;i++){
        fv=w->focusSearch(fv,View::FOCUS_FORWARD);
        ASSERT_EQ(fv->getId(),vids[i%6]);
    }
}
