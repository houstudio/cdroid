#include <stdio.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <vector>
#include <list>
#include <cdroid.h>
#include <guienvironment.h>
using namespace cdroid;

class FOCUS:public testing::Test{
public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

/* Each case builds a small focus tree in the shared content area (instead of a
   private Window) and walks it with addFocusables/focusSearch — those are
   View/ViewGroup methods, so a FrameLayout container behaves identically.
   pumpFor lets you see the focusable grid briefly. */
TEST_F(FOCUS,NOFOCUS){
    ViewGroup*w=GUIEnvironment::content();
    for(int i=0;i<6;i++){
        TextView*tv=new TextView("TextView_"+std::to_string(i),200,50);
        w->addView(tv);
        tv->layout(10,i*55,200,50);
    }
    std::vector<View*>focus;
    w->addFocusables(focus,(int)View::FOCUS_LEFT,(int)View::FOCUSABLES_ALL);
    ASSERT_EQ(1,focus.size());
    focus.clear();

    w->setFocusable(false);
    w->addFocusables(focus,(int)View::FOCUS_RIGHT,(int)View::FOCUSABLES_ALL);
    ASSERT_EQ(0,focus.size());
    pumpFor(300);
}


TEST_F(FOCUS,ALLFOCUS){
    ViewGroup*w=GUIEnvironment::content();
    for(int i=0;i<6;i++){
        EditText*tv=new EditText("EditText_"+std::to_string(i),200,50);
        w->addView(tv);tv->layout(10,i*55,200,50);
    }
    std::vector<View*>focus;
    w->addFocusables(focus,(int)View::FOCUS_LEFT,(int)View::FOCUSABLES_ALL);
    ASSERT_EQ(6,focus.size());
    focus.clear();
    w->addFocusables(focus,(int)View::FOCUS_RIGHT,(int)View::FOCUSABLES_ALL);
    ASSERT_EQ(6,focus.size());
    pumpFor(300);
}

TEST_F(FOCUS,HALFFOCUS){
    ViewGroup*w=GUIEnvironment::content();
    for(int i=0;i<6;i++){
        View*tv;
        if(i%2==0)tv=new EditText("EditText_"+std::to_string(i),200,50);
        else tv=new TextView("TextView_"+std::to_string(i),200,50);
        tv->setFocusable(i%2==0);
        w->addView(tv);
        tv->layout(10,i*55,200,50);
    }
    std::vector<View*>focus;
    w->addFocusables(focus,(int)View::FOCUS_LEFT,(int)View::FOCUSABLES_ALL);
    ASSERT_EQ(3,focus.size());
    focus.clear();
    w->addFocusables(focus,(int)View::FOCUS_RIGHT,(int)View::FOCUSABLES_ALL);
    ASSERT_EQ(3,focus.size());
    pumpFor(300);
}

TEST_F(FOCUS,GROUP){
    ViewGroup*w=GUIEnvironment::content();
    LinearLayout*l[2];
    l[0]=new LinearLayout(0,0,800,300);
    l[1]=new LinearLayout(0,310,800,200);
    for(int i=0;i<4;i++){
        View*tv;
        tv=new EditText("EditText_"+std::to_string(i),200,50);
        l[i%2]->addView(tv);
        tv=new TextView("TextView_"+std::to_string(i),200,50);
        l[i%2]->addView(tv);
        tv->setFocusable(false);
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
    pumpFor(300);
}

TEST_F(FOCUS,NAV_1_LAYER){
    ViewGroup*w=GUIEnvironment::content();
    for(int i=0;i<6;i++){
        View*tv;
        if(i%2==0){
            tv=new EditText("EditText_"+std::to_string(i),200,50);
            w->addView(tv);tv->setId(100+i/2);
            tv->layout(10,i*55,200,50);
        }else{
            tv=new TextView("TextView_"+std::to_string(i),200,50);
            w->addView(tv);tv->setId(200+i/2);
            tv->layout(10,i*55,200,50);
            tv->setFocusable(false);;
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
    pumpFor(300);
}

TEST_F(FOCUS,NAV_2_LAYER){
    ViewGroup*w=GUIEnvironment::content();
    LinearLayout*l[2];
    l[0]=new LinearLayout(0,0,800,300);
    l[1]=new LinearLayout(0,310,800,200);

    for(int i=0;i<3;i++){
        View*tv;
        LinearLayout*pl=l[0];
        tv=new EditText("EditText_"+std::to_string(i),200,50);
        pl->addView(tv);
        tv->setId(100-1+pl->getChildCount());
        tv=new TextView("TextView_"+std::to_string(i),200,50);
        pl->addView(tv);tv->setFocusable(false);
    }
    for(int i=0;i<3;i++){
        View*tv;
        LinearLayout*pl=l[1];
        tv=new EditText("EditText_"+std::to_string(i),200,50);
        pl->addView(tv);
        tv->setId(200-1+pl->getChildCount());
        tv=new TextView("TextView_"+std::to_string(i),200,50);
        pl->addView(tv);
        tv->setFocusable(false);
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
    pumpFor(300);
}
