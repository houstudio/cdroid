#include <windows.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,1280,600);
    TabLayout* tab=new TabLayout(1280,600);

    for(int i=0;i<10;i++){
        TabLayout::Tab*tbitm=tab->newTab();
        tbitm->setText(std::string("Tab")+std::to_string(i));
        tab->addTab(tbitm);
    }
    w->addView(tab);
    tab->requestLayout();
    w->requestLayout();
    app.exec();
}
