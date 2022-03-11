#include <gtest/gtest.h>
#include <cdroid.h>
#include <ngl_os.h>
#include <fstream>

static const char*texts[]={"Creates 中国智造"," the specified format and dimensions.",
            "Initially the surface contents"," are set to 0.","(Specifically, within each pixel,",
            " each color or alpha channel","belonging to format will be 0.","The contents","of bits within a pixel,",
            " but not belonging","必须使用UTF8编码 " };
class WINDOW:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

class TestWindow:public Window{
public:
   TestWindow(int x,int y,int w,int h):Window(x,y,w,h){}
   virtual bool onKeyRelease(KeyEvent& evt){
     App::getInstance().exit(0);
     //sendMessage(View::WM_DESTROY,0,0);
     return true;
   }
};

TEST_F(WINDOW,EmptyWindow){
   App app;
   Window*w=new TestWindow(100,100,800,600);
   app.exec();
}

TEST_F(WINDOW,LocationInWindow1){
    Window* w=new Window(100,100,400,400);
    View*v=new View(200,200);
    w->addView(v).setPos(10,10);
    POINT pt;
    w->getLocationInWindow((int*)&pt);
    ASSERT_EQ(pt.x,0);
    ASSERT_EQ(pt.y,0);
    v->getLocationInWindow((int*)&pt);
    ASSERT_EQ(pt.x,10);
    ASSERT_EQ(pt.y,10);
    delete w;
}

TEST_F(WINDOW,LocationInWindow2){
    Window* w=new Window(100,100,400,400);
    View*v=new View(200,200);
    ViewGroup*vg=new ViewGroup(200,200);
    w->addView(vg).setPos(10,10);
    vg->addView(v).setPos(10,10);
    POINT pt;
    vg->getLocationInWindow((int*)&pt);

    ASSERT_EQ(pt.x,10);
    ASSERT_EQ(pt.y,10);

    v->getLocationInWindow((int*)&pt);
    ASSERT_EQ(pt.x,20);
    ASSERT_EQ(pt.y,20);
    delete w;
}

TEST_F(WINDOW,LocationInWindow3){
    Window* w=new Window(100,100,400,400);
    View*v=new View(200,200);
    ViewGroup*vg=new ViewGroup(200,200);
    w->addView(vg).setPos(10,10);
    vg->addView(v).setPos(10,10);
    vg->scrollTo(10,5);
    POINT pt;
    vg->getLocationInWindow((int*)&pt);

    ASSERT_EQ(pt.x,0);
    ASSERT_EQ(pt.y,5);

    v->getLocationInWindow((int*)&pt);
    ASSERT_EQ(pt.x,10);
    ASSERT_EQ(pt.y,15);
    delete w;
}
TEST_F(WINDOW,create){
   App app;
   Window*w=new TestWindow(100,100,1000,600);
   LinearLayout*layout=new LinearLayout(1000,600);
   for(int i=0;i<6;i++)
      layout->addView(new TextView(texts[i],200,32));
   w->addView(layout);
   app.exec();
}

TEST_F(WINDOW,inflate){
    App app;
    Window*w=new Window(0,0,800,600);
    LayoutInflater::from(&app)->inflate("cdroid:layout/window.xml",w);
    app.exec();
}

TEST_F(WINDOW,showhide){
   App app;
   int index=0;
   Window*w[8];
   int wc=sizeof(w)/sizeof(w[0]);
   unsigned int cls[]={0xFFFFFFFF,0xFF000000,0xFFFF0000,0xFF00FF00,0xFF0000FF,0xFFFFFF00,0xFF00FFFF,0xFFFF00FF,0xFFFFFFFF};
   for(int i=0;i<wc;i++){
       w[i]=new Window(80*i,50*i,480,320);
       w[i]->setBackgroundColor(cls[i]);
   }
   /*TimerFD *tfd=new TimerFD(50,false);
   app.addEventSource(tfd,[&](EventSource&s)->bool{
      w[index]->show();
      index=(index+1)%wc;
      w[index]->hide();
      return true; 
   });*/
   app.exec();
}
TEST_F(WINDOW,multilayer){
    App app;
    Window*w=new Window(100,100,800,600);
    w->setBackgroundColor(0xFFFFFFFF);
    ViewGroup*v1=new ViewGroup(400,300);
    v1->setBackgroundColor(0xFFFF0000);
    Button*v2=new Button("Helloworld",200,100);
    v2->setBackgroundColor(0xFF00FF00);
    v2->setTextColor(0xFF0000FF);
    v1->addView(v2).setPos(100,50).setId(100);
    w->addView(v1).setPos(100,50).setId(10);
    w->setId(1);
    v2->setOnClickListener([&](View&v){
        w->scrollBy(10,10);
        v1->invalidate(true);
    });
    app.exec();
}

TEST_F(WINDOW,memleak){
   App app;
   Window*w=new Window(0,0,1280,720);
   sleep(10);
   Button*btn=new Button("test",200,100);
   w->addView(btn);
   for(int i=0;i<1000000;i++){
       btn->invalidate(true);
   }
}
TEST_F(WINDOW,LinearLayout){
   App app;
   Window*w=new TestWindow(100,100,820,620);
   LinearLayout*g1=new LinearLayout(0,0,800,400);
   LinearLayout*g2=new LinearLayout(0,410,800,200);

   int count=sizeof(texts)/sizeof(char*);
   for(int i=0;i<8;i++){
       g1->addView(new Button(texts[i%count],200,40));
       g2->addView(new Button(texts[(i+7)%count],200,40));
   }
   for(int i=0;i<0;i++){
       ImageView*img=new ImageView(300-i*50,300-i*50);
       g1->addView(img);
       ImageView*img2=new ImageView(200-i*50,200-i*60);
       g2->addView(img2);
   }
   w->addView(g1);    
   w->addView(g2);    
   app.exec();
}

class MWindow:public TestWindow{
private:
   int dx,dy;
public:
   MWindow(int x,int y,int w,int h):TestWindow(x,y,w,h){dx=dy=50;}
   /*virtual bool onMessage(DWORD msgid,DWORD wp,ULONG lp){
      if(View::WM_TIMER==msgid){
         sendMessage(msgid,wp,lp,lp);
         int x=getX();
         int y=getY();
         if(x+getWidth()+dx>1280)dx*=-1;
         if(y+getHeight()+dy>720)dy*=-1;
         if(x+dx<0)dx*=-1;
         if(y+dy<0)dy*=-1;
         setPos(x+dx,y+dy);
         return true;
      }
      return false;
   }*/
   void setDir(int x,int y){dx=x;dy=y;}
};

TEST_F(WINDOW,multiwindow){
    App app; 
    MWindow*w3=new MWindow(200,300,410,280);
    MWindow*w1=new MWindow(100,100,400,320);
    MWindow*w2=new MWindow(300,200,420,300);
    w1->setBackgroundColor(0xFFFF0000);
    w2->setBackgroundColor(0xFF00FF00);
    //w1->sendMessage(View::WM_TIMER,0,100,100);w1->setDir(50,40);
    //w2->sendMessage(View::WM_TIMER,0,100,80);w2->setDir(66,53);
    //w3->sendMessage(View::WM_TIMER,0,100,85);w3->setDir(69,57);
    app.exec();
}

TEST_F(WINDOW,TOAST){
    App app;
    const static char*texts[]={"Hello world","Program is scrambed by provider",
       "The quick brown fox jumps over a lazy dog",
       "Innovation in China","Innovation by Shenzhen"};
    Window*w1=new TestWindow(100,100,820,620);
    int cnt=sizeof(texts)/sizeof(texts[0]);
    for(int i=0;i<8;i++){
       printf("toast text:%s\r\n",texts[i%cnt]);
       Toast::makeText(std::string(texts[i%cnt]),600+200*i)->setPos(i*80,i*40);
    }
    app.exec();
}

static ToastWindow*createCustomToast(const std::string&txt,int w,int h,int timeout){
    return Toast::makeWindow([&]()->ToastWindow*{
            ToastWindow*win=new ToastWindow(500,100);
            TextView*tv=new TextView(txt,w-20,h-20);
            tv->setTextSize(20);
            tv->setSingleLine(false);
            win->addView(tv).setPos(10,10);
            return win;
     },timeout);
}

TEST_F(WINDOW,TOAST_MAKEWINDOW){
    App app;
    int index=0;
    const static char*texts[]={"Hello world","Program is scrambed by provider",
       "The quick brown fox jumps over a lazy dog",
       "Innovation in China","Innovation by Shenzhen"};
    int cnt=sizeof(texts)/sizeof(texts[0]);
    for(int i=0;i<cnt;i++){
       createCustomToast(texts[i],300,80,10000-i*1000)->setPos(i*50,i*50);
    }
    /*TimerFD *tfd=new TimerFD(800,false);
    app.addEventSource(tfd,[&](EventSource&s)->bool{
       createCustomToast(texts[index],300,80,1000-index*100)->setPos(index*50,index*50);
       index=(index+1)%cnt;
       return true;
    });*/
    app.exec();
}

class PopWindow{
protected:
  ToastWindow*mt;
  static PopWindow*mInst;
  friend class MyToast;
  PopWindow(){mt=nullptr;}
public:
  static PopWindow*getInstance(){
      if(mInst==nullptr)
        mInst=new PopWindow();
      return mInst;
  }
  void pop(){
      mt=Toast::makeText("Test Message",2000);
  }
  void hide(){
      if(mt)
      mt->close();
      mt=nullptr;
  }
};
PopWindow*PopWindow::mInst=nullptr;

class MyToast:public Toast{
public:
   MyToast(int w,int h):Toast(w,h){}
   ~MyToast(){
       PopWindow::getInstance()->mt=nullptr;
   }
};

TEST_F(WINDOW,TOAST_SINGLE){
   App app;
   int index=0;
   /*TimerFD *tfd=new TimerFD(800,false);
   app.addEventSource(tfd,[&](EventSource&s)->bool{
        if(index%2)PopWindow::getInstance()->hide();
        else PopWindow::getInstance()->pop();
        index++;
        return true;
   });*/
   app.exec();
}



TEST_F(WINDOW,OrderedChildList){
    App app;
    Window*ow=new Window(0,0,640,480);
    for(int i=0;i<4;i++)
        ow->addView(new TextView(std::string("textview")+std::to_string(i),100,40)).setId(i).setZ(4-i);
    std::vector<View*>views=ow->buildTouchDispatchChildList();
    ASSERT_EQ(views[0]->getId(),3);
    ASSERT_EQ(views[3]->getId(),0);
    app.exec();
}
