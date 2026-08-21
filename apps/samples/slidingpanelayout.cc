#include <core/app.h>
#include <widget/button.h>
#include <widget/cdwindow.h>
#include <widget/linearlayout.h>
#include <widget/slidingpanelayout.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    SlidingPaneLayout*spl = new SlidingPaneLayout(&App::getInstance());
    //spl->setPanelSlideGravity(Gravity::LEFT);
    w->addView(spl);
    LinearLayout*ll=new LinearLayout(&app);
    LinearLayout*lr=new LinearLayout(&app);
    ll->setId(100);
    lr->setId(200);
    ll->setOrientation(LinearLayout::VERTICAL);
    lr->setOrientation(LinearLayout::VERTICAL);
    //spl->setParallaxDistance(120);

    SlidingPaneLayout::LayoutParams*lp =new SlidingPaneLayout::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::MATCH_PARENT);
    TextView*tv=new TextView(&app); tv->setText(" Left Panel " );
    Button*btn=new Button(&app); btn->setText("Open/Close" );
    ll->addView(tv,new LinearLayout::LayoutParams(200/*LayoutParams::MATCH_PARENT*/,LayoutParams::WRAP_CONTENT));
    ll->addView(btn,new LinearLayout::LayoutParams(200/*LayoutParams::MATCH_PARENT*/,LayoutParams::WRAP_CONTENT));
    spl->addView(ll,-1,lp);
    
    lp=new SlidingPaneLayout::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::MATCH_PARENT);
    spl->addView(lr,-1,lp);
    auto click=[spl](View& v) {
        LOGD("isopened=%d",spl->isOpen());
        if (spl->isOpen()) {
            spl->closePane();
        }else{
            spl->openPane();
        }
    };
    btn->setOnClickListener(click);
    ll->setOnClickListener(click);
    lr->setOnClickListener(click);
    spl->setBackgroundColor(0xFF222222);
    ll->setBackgroundColor(0xFFFF0000);
    lr->setBackgroundColor(0xFF00FF00);
    SlidingPaneLayout::PanelSlideListener lst;
    lst.onPanelSlide=[](View&panel,float slidOffset){
        LOGD("panel %p:%d offset=%f",&panel,panel.getId(),slidOffset);
    };
    lst.onPanelOpened=[](View&panel){
        LOGD("panel %p:%d opened",&panel,panel.getId());
    };
    lst.onPanelClosed=[](View&panel){
        LOGD("panel %p:%d closed",&panel,panel.getId());
    };
    spl->setPanelSlideListener(lst);
    return app.exec();
}
