#include <cdroid.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,1280,600);

    TabLayout* tab=new TabLayout(1280,40);
    tab->setSelectedTabIndicatorColor(0xFF00FF00);
    tab->setSelectedTabIndicatorHeight(4);
    for(int i=0;i<6;i++){
        TabLayout::Tab*t=tab->newTab();
        t->setText("Tab"+std::to_string(i));
        tab->addTab(t);
        if(argc>=2&&atoi(argv[1])==i)t->select();
    }
    TabLayout::OnTabSelectedListener ls;
    ls.onTabSelected=[](TabLayout::Tab&tab){
        LOGD("select %d %s",tab.getPosition(),tab.getText().c_str());
    };
    ls.onTabUnselected=[](TabLayout::Tab&tab){
        LOGD("unselect %d %s",tab.getPosition(),tab.getText().c_str());
    };
    ls.onTabReselected=[](TabLayout::Tab&tab){
        LOGD("reSelect %d %s",tab.getPosition(),tab.getText().c_str());
    };
    tab->addOnTabSelectedListener(ls);
    w->addView(tab);
    w->requestLayout();
    app.exec();
}
