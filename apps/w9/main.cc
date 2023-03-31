#include <cdroid.h>
#include <string>
#include <cdlog.h>
#include <iostream>
#include <homewindow.h>
int main(int argc,const char*argv[]){
    App app(argc,argv);
    //Window*w=new Window(0,0,-1,-1);//HomeWindow(argc%2);
    Window*w=new HomeWindow(argc%2);
    GradientDrawable*d=(GradientDrawable*)app.getDrawable("@drawable/test");
    d->setUseLevel(true);
    /*d->setInnerRadius(20);
    d->setThickness(20);
    d->setColor(0xff00ff);*/
    d->setLevel(argc==1?10000:atoi(argv[1]));//.f*359.f/360);
    w->setBackground(d);
    return app.exec();
}

