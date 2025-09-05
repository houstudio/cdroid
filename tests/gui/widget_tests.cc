#include <gtest/gtest.h>
#include <cdroid.h>
#include <widget/keyboardview.h>
#include <guienvironment.h>
#include <cdlog.h>

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
    App app(argc,argv);
    Window*w=new Window(0,0,1280,720);
    const char*strings[]={"LEFT","CENTER","RIGHT","LEFT|TOP","CENTER|TOP","RIGHT|TOP"};
    TextView*t1=new TextView("",400,200);
    TextView*t2=new TextView(std::string(),400,300);
    w->addView(t1);
    w->addView(t2); 
    app.exec();
}

TEST_F(WIDGET,Button){
   App app(argc,argv);
   Window*w=new Window(100,100,800,600);
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
   app.exec();
}

TEST_F(WIDGET,ImageView){
   App app(argc,argv);
   Window*w=new Window(100,100,800,600);
   ImageView*iv=new ImageView(400,400);
   Drawable*d=new BitmapDrawable(nullptr,"/home/houzh/Miniwin/apps/ntvplus/assets/drawable/light2.jpg");
   iv->setImageDrawable(d);
   w->addView(iv);
   iv->layout(100,100,400,400);
   app.exec();
}

TEST_F(WIDGET,EditText){
    const std::string sss[]={
         "AfterInstallationEnableRequiredApacheModulesAndRestartApacheService","",
         "AbcEFG","^[A-Za-z]+$",
         "123","^[1-9]\\d*$",
         "आज सुबह एक ट्रैफिक जैम था","""^[A-Za-z]+$",
         "0.0.0.0","^((25[0-5]|2[0-4]\\d|((1\\d{2})|([1-9]?\\d)))\\.){3}(25[0-5]|2[0-4]\\d|((1\\d{2})|([1-9]?\\d)))$"//"\\d+\\.\\d+\\.\\d+\\.\\d+",
    };
    App app(argc,argv);
    Window*w=new Window(100,100,800,600);
    EditText *e;
    e=new EditText("How to call customized WPF Window in VSTO ribbon?"
"For example, I made customized WPF window by designing the styles. And then I would like to use it in my Presentation addin."
" I can run the WPF window directly, but cannot call it from Ribbon. Thanks very much!",600,100);
    w->addView(e);
    e->layout(10,20,600,100);
    e->setSingleLine(false);
    e->setBreakStrategy(0);
    e->setBackgroundColor(0xFF222222);
    for(int i=0,y=160;i<4;i++,y){
        EditText *e=new EditText(680,((i==0)?100:50)+i*5);
        e->setBackgroundColor(0Xff444444);
        e->setSingleLine(i>0);
        e->setBreakStrategy(i==0);
        e->setTextSize(30.f+i*5);
        e->setText(sss[i*2]);
        e->setInputType(EditText::TYPE_ANY);
        e->setPattern(sss[i*2+1]);     
        w->addView(e);
        e->layout(10,y,680,((i==0)?100:50)+i*5);
        y+=e->getHeight()+1;
        e->setEditMode(i!=2?EditText::INSERT:EditText::REPLACE);
    }
    app.exec();
}

TEST_F(WIDGET,ProgressBar){
    App app(argc,argv);
    int pos=0;
    Window*w=new Window(100,100,800,600);
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

    Runnable run;
    run=[&](){
       ((ProgressBar*)w->findViewById(100))->setProgress(pos);
       ((ProgressBar*)w->findViewById(101))->setProgress(pos);
       ((ProgressBar*)w->findViewById(102))->setProgress(pos);
       pos=(pos+1)%100;
       LOGD("pos=%d",pos);
       w->postDelayed(run,50);
    };
    w->postDelayed(run,500);
    app.exec();
}

TEST_F(WIDGET,SeekBar){
    App app(argc,argv);
    Window*w=new Window(100,100,800,600);
    SeekBar*sb=new SeekBar(400,40);
    w->addView(sb);
    app.exec();
}

static const char*texts[]={
  "ios_base::beg","beginning of the stream",
  "ios_base::cur","current position in the stream",
  "ios_base::end","end of the stream"
};


TEST_F(WIDGET,Selector){
    App app(argc,argv);
    Window*w=new Window(100,100,800,620);

    app.exec();
}

TEST_F(WIDGET,ListView){
    App app(argc,argv);
    Window*w=new Window(100,100,800,620);
    app.exec();
}

TEST_F(WIDGET,Keyboard){
    App app(argc,argv);
    Window*w=new Window(100,100,800,400);
    KeyboardView*kbv=new KeyboardView(800,300);
    kbv->setBackgroundColor(0xFFEEEEEE);
    Keyboard*kbd=new Keyboard(&app,"cdroid:xml/qwerty.xml",800,200);
    kbv->setKeyboard(kbd);
    w->addView(kbv);
    kbv->layout(20,10,800,300);
    app.exec();
}
