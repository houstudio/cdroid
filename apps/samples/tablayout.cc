#include <cdroid.h>
#include <porting/cdlog.h>
#include <widget/candidateview.h>
#include <widget/chronometer.h>
#include <core/bundle.h>
#include <core/process.h>
#include <view/asyncinflater.h>
#include <widget/tablayout.h>
//#include <mgl2/mgl.h>
int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,800,600);
    w->setBackgroundColor(0xFF4488cc);
    TabLayout*tl =new TabLayout(800,600);
    w->addView(tl,new Window::LayoutParams(LayoutParams::MATCH_PARENT,64));
    for(int i=0;i<4;i++){
       auto tab =tl->newTab();
       tab->setText("tab");
       tl->addTab(tab);
    }

    LinearLayout*llbtns=new LinearLayout(800,64);
    llbtns->setBackgroundColor(0xFF112233);
    llbtns->setOrientation(LinearLayout::HORIZONTAL);
    Window::LayoutParams*wlp=new Window::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::WRAP_CONTENT);
    wlp->gravity=Gravity::BOTTOM;
    w->addView(llbtns,wlp);

    Button*btn=new Button("Add",120,48);
    LinearLayout::LayoutParams*lp=new LinearLayout::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::WRAP_CONTENT);
    lp->topMargin=200;
    llbtns->addView(btn,lp);
    btn->setOnClickListener([&](View&){
      auto tab =tl->newTab();
      tab->setText("tab");
      tl->addTab(tab);
      tl->requestLayout();
        });
    w->requestLayout();
    return app.exec();
}
