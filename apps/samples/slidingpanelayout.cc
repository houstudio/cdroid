#include <core/app.h>
#include <widget/cdwindow.h>
#include <widget/linearlayout.h>
#include <widget/slidingpanelayout.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    SlidingPaneLayout*spl = new SlidingPaneLayout(-1,-1);
    //spl->setPanelSlideGravity(Gravity::LEFT);
    w->addView(spl);
    LinearLayout*ll=new LinearLayout(-1,-1);
    LinearLayout*lr=new LinearLayout(-1,-1);
    ll->setId(100);
    lr->setId(200);
    ll->setOrientation(LinearLayout::VERTICAL);
    lr->setOrientation(LinearLayout::VERTICAL);
    spl->setParallaxDistance(120);
    SlidingPaneLayout::LayoutParams*lp =new SlidingPaneLayout::LayoutParams(300,LayoutParams::MATCH_PARENT);
    spl->addView(ll,-1,lp);
    lp=new SlidingPaneLayout::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::MATCH_PARENT);
    spl->addView(lr,-1,lp);
    auto click=[spl](View& v) {
        LOGD("isopened=%d",spl->isOpen());
        if (spl->isOpen()) {
            spl->closePane();
        }
    };
    ll->setOnClickListener(click);
    lr->setOnClickListener(click);
    spl->setBackgroundColor(0xFF222222);
    ll->setBackgroundColor(0xFFFF0000);
    lr->setBackgroundColor(0xFF00FF00);
    return app.exec();
}
