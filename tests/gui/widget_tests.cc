#include <gtest/gtest.h>
#include <core/app.h>
#include <core/keyboard.h>
#include <view/view.h>
#include <widget/cdwindow.h>
#include <widget/textview.h>
#include <widget/button.h>
#include <widget/imageview.h>
#include <widget/progressbar.h>
#include <widget/seekbar.h>
#include <widget/linearlayout.h>
#include <widget/keyboardview.h>
#include <widget/toast.h>
#include <drawable/drawable.h>
#include <drawable/bitmapdrawable.h>
#include <porting/cdlog.h>
#include <guienvironment.h>
using namespace cdroid;

#define ID_OK 10
#define ID_CANCEL 15
#define ID_LISTVIEW 20
#define ID_TIPINFO 30

class WIDGET:public testing::Test{
public:
    int argc;
    const char**argv;
    virtual void SetUp(){
        argc = GUIEnvironment::getInstance()->getArgc();
        argv = GUIEnvironment::getInstance()->getArgv();
    }
    virtual void TearDown(){
    }
};
static void onClick(View&v){
    std::string txt="You clicked:";
    txt+=((TextView&)v).getText();
    txt+="   id:"+std::to_string(v.getId());
    Toast::makeText(v.getContext(),txt,2000);
}

TEST_F(WIDGET,View){
    View v(400,80);
    v.setTextAlignment((int)View::TEXT_ALIGNMENT_CENTER);//no (int) will caused link error
    ASSERT_EQ(v.getTextAlignment(),(int)View::TEXT_ALIGNMENT_CENTER);

    v.setTextAlignment((int)View::TEXT_ALIGNMENT_VIEW_START);
    ASSERT_EQ(v.getTextAlignment(),(int)View::TEXT_ALIGNMENT_VIEW_START);

    v.setTextAlignment((int)View::TEXT_ALIGNMENT_VIEW_END);
    ASSERT_EQ(v.getTextAlignment(),(int)View::TEXT_ALIGNMENT_VIEW_END);
}
TEST_F(WIDGET,TextView){
    Window*w=GUIEnvironment::stage();
    const char*strings[]={"LEFT","CENTER","RIGHT","LEFT|TOP","CENTER|TOP","RIGHT|TOP"};
    TextView*t1=new TextView("",400,200);
    TextView*t2=new TextView(std::string(),400,300);
    w->addView(t1);
    w->addView(t2);
    pumpFor(500);
}

TEST_F(WIDGET,Button){
   Window*w=GUIEnvironment::stage();
   LinearLayout*layout=new LinearLayout(800,600);
   Button*btn1=new Button("OK",100,30);
   Button*btn2=new Button("Cancel",100,30);
   btn1->setId(ID_OK);
   btn1->setOnClickListener(onClick); //it's same as following lambda segment
   btn1->setOnClickListener([](View&v){
       std::string txt="You clicked:";
       txt+=((TextView&)v).getText();
       txt+="   id:"+std::to_string(v.getId());
       Toast::makeText(v.getContext(),txt,2000);
   });
   //btn2->setOnClickListener(click);//it click listener is not set ,view's parent will recv WM_CLICK message
   layout->addView(btn1);
   layout->addView(btn2);
   w->addView(layout);
   btn2->setId(ID_CANCEL);
   pumpFor(500);
}

TEST_F(WIDGET,ImageView){
   Window*w=GUIEnvironment::stage();
   ImageView*iv=new ImageView(400,400);
   Drawable*d=new BitmapDrawable(nullptr,"/home/houzh/Miniwin/apps/ntvplus/assets/drawable/light2.jpg");
   iv->setImageDrawable(d);
   w->addView(iv);
   iv->layout(100,100,400,400);
   pumpFor(500);
}

TEST_F(WIDGET,ProgressBar){
    int pos=0,ticks=0;
    Window*w=GUIEnvironment::stage();
    ProgressBar*pb;
    LinearLayout*ll=new LinearLayout(800,600);
    pb=new ProgressBar(800,20);
    ll->addView(pb);pb->setId(100);
    pb->setProgress(30);

    pb = new ProgressBar(30,200);
    ll->addView(pb);
    pb->setId(101);

    pb=new ProgressBar(800,20);
    ll->addView(pb);
    pb->setIndeterminate(true);

    pb=new ProgressBar(200,200);
    w->addView(pb);pb->setId(102);
    pb=new ProgressBar(200,200);
    ll->addView(pb);
    pb->setIndeterminate(true);
    w->addView(ll);

    /* Bounded self-repost: stops after `ticks` reach the cap so the Runnable
       never outlives this case (its &pos/&ticks captures would dangle). */
    Runnable run;
    run=[&](){
       ((ProgressBar*)w->findViewById(100))->setProgress(pos);
       ((ProgressBar*)w->findViewById(101))->setProgress(pos);
       ((ProgressBar*)w->findViewById(102))->setProgress(pos);
       pos=(pos+1)%100;
       if(++ticks<100) w->postDelayed(run,50);
    };
    w->postDelayed(run,500);
    pumpUntil([&]{ return ticks>=100; }, 6000);
}

TEST_F(WIDGET,SeekBar){
    Window*w=GUIEnvironment::stage();
    SeekBar*sb=new SeekBar(400,40);
    w->addView(sb);
    pumpFor(500);
}

static const char*texts[]={
  "ios_base::beg","beginning of the stream",
  "ios_base::cur","current position in the stream",
  "ios_base::end","end of the stream"
};


TEST_F(WIDGET,Selector){
    Window*w=GUIEnvironment::stage();
    pumpFor(500);
}

TEST_F(WIDGET,ListView){
    Window*w=GUIEnvironment::stage();
    pumpFor(500);
}

TEST_F(WIDGET,Keyboard){
    App&app=App::getInstance();
    Window*w=GUIEnvironment::stage();
    KeyboardView*kbv=new KeyboardView(800,300);
    kbv->setBackgroundColor(0xFFEEEEEE);
    Keyboard*kbd=new Keyboard(&app,"cdroid:xml/qwerty.xml",800,200);
    kbv->setKeyboard(kbd);
    w->addView(kbv);
    kbv->layout(20,10,800,300);
    pumpFor(500);
}
