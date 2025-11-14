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
    ll->setOrientation(LinearLayout::VERTICAL);
    lr->setOrientation(LinearLayout::VERTICAL);
    spl->setParallaxDistance(120);
    spl->addView(ll,-1,new SlidingPaneLayout::LayoutParams(300,LayoutParams::MATCH_PARENT));
    SlidingPaneLayout::LayoutParams*lp=new SlidingPaneLayout::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::MATCH_PARENT);
    lp->slideable = true;
    spl->addView(lr,-1,lp);
    auto click=[spl](View& v) {
        LOGD("isopened=%d",spl->isOpen());
        if (spl->isOpen()) {
            spl->closePane();
        }
    };
    ll->setOnClickListener(click);
    lr->setOnClickListener(click);
    ll->setBackgroundColor(0xFF112233);
    lr->setBackgroundColor(0xFF111111);
    return app.exec();
}
