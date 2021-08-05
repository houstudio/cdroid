#include <windows.h>
#include <cdlog.h>


class TestWindow:public Window{
protected:
    int flag;
    TextView*tv1;
    TextView*tv2;
    TextView*tv3;
public:
    TestWindow(int w,int h);
    bool onMessage(DWORD msg,DWORD wp,ULONG lp);
};

TestWindow::TestWindow(int w,int h):Window(0,0,w,h){
    flag=1;
    LinearLayout*ll=new LinearLayout(LayoutParams::MATCH_PARENT,LayoutParams::MATCH_PARENT);
    ll->setOrientation(LinearLayout::VERTICAL);

    tv1=new TextView("TextView SingleLine alignment is LEFT",0,0);
    tv1->setTextSize(24);
    tv1->setTextColor(0xFF00FF00);
    tv1->setBackgroundColor(0xFF001122);
    ll->addView(tv1,new LinearLayout::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::WRAP_CONTENT));

    TextView*tv=new TextView("TextView alignment is CENTER",0,0);
    tv->setTextAlignment(TEXT_ALIGNMENT_CENTER);tv->setTextSize(26);
    tv->setBackgroundColor(0xFF112233);
    ll->addView(tv,new LinearLayout::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::WRAP_CONTENT));
    tv=new TextView("TextView alignment is RIGHT",0,0);
    tv->setBackgroundColor(0xFF223344);tv->setTextSize(28);
    tv->setTextAlignment(TEXT_ALIGNMENT_TEXT_END);
    ll->addView(tv,new LinearLayout::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::WRAP_CONTENT));


    tv2=new TextView("TextView with drawable/progress_horizontal as background",0,0);
    tv2->setTextSize(32);
    AnimatedRotateDrawable*ad=(AnimatedRotateDrawable*)getContext()->getDrawable("cdroid:drawable/progress_large.xml");
    tv2->setCompoundDrawablesWithIntrinsicBounds(ad,nullptr,nullptr,nullptr);
    tv2->setGravity(Gravity::CENTER_VERTICAL);
    tv2->setBackgroundResource("cdroid:drawable/progress_horizontal.xml");
    ll->addView(tv2);

    tv3=new TextView("TextView support word break and BIDI layout,my attributes is setted with multiline and with clip drawable as foreground,"
                     "each view has a backgrounddrawable as its background,"
                     " and a foreground drawable ontopof the view."
                     "background and foregrond make UI component custmize very easy.",0,0);
    tv3->setSingleLine(false);
    tv3->setTextSize(32);
    tv3->setBackgroundColor(0xFF334455);
    ClipDrawable*cd=new ClipDrawable(new ColorDrawable(0xFF00FF00),Gravity::LEFT,1);
    tv3->setForeground(cd);cd->setLevel(10000);
    ll->addView(tv3,new LinearLayout::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::WRAP_CONTENT));
    tv = new TextView("cdroid is an android like GUI engine\n"
                      "it is friendly for android developers\n"
                      "my padding is (20,5,20,5)",0,0);
    tv->setSingleLine(false);tv->setTextSize(24);
    tv->setPadding(20,5,20,5);
    tv->setBackgroundColor(0xFF445566);
    tv->setGravity(Gravity::RIGHT);
    tv->setTextAlignment(TEXT_ALIGNMENT_TEXT_END);
    ll->addView(tv,new LinearLayout::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::WRAP_CONTENT));
    this->addView(ll).setSize(1080,720);
    ad->start();
    requestLayout();
}

bool TestWindow::onMessage(DWORD msg,DWORD wp,ULONG lp){
    switch(msg){
    case WM_TIMER:{
        srand(time(nullptr));
        tv2->getBackground()->setLevel(rand()%10000);
        int lvl=tv2->getCompoundDrawables()[0]->getLevel();
        tv2->getCompoundDrawables()[0]->setLevel((lvl+200)%10000);

        lvl=tv3->getForeground()->getLevel();
        if(flag>0){lvl+=200;if(lvl>=10000)flag=-1;}
        if(flag<0){lvl-=200;if(lvl<=0)flag=1;}
        tv3->getForeground()->setLevel(lvl);
        //sendMessage(msg,wp,lp,200);
    }break;
    default:return true;//Window::onMessage(msg,wp,lp);
    }
}

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new TestWindow(1080,720);
    //w->sendMessage(View::WM_TIMER,0,0,500);
    app.exec();
    return 0;
}
