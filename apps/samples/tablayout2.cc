#include <cdroid.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,1280,600);

    TabLayout* tab=new TabLayout(1280,80);
    tab->setSelectedTabIndicatorColor(0xFF00FF00);
    tab->setSelectedTabIndicatorHeight(4);
    tab->setTabIndicatorGravity(Gravity::BOTTOM);//TOP/BOTTOM/CENTER_VERTICAL/FILL_VERTICAL
    for(int i=0;i<6;i++){
        TabLayout::Tab*t=tab->newTab();
        t->setText("Tab"+std::to_string(i));
        tab->addTab(t);
        if(argc>=2&&atoi(argv[1])==i)t->select();
    }
    w->addView(tab);
    w->requestLayout();
    app.exec();
}
