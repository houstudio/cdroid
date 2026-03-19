#include <cdroid.h>
#include <cdlog.h>
#include <fstream>
#include <widget/cardview.h>
int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    CardView*cv=new CardView(0,0);
    cv->setRadius(120);
    cv->setMaxCardElevation(40);//MaxCardElevation must be set before setCardElevation
    cv->setCardElevation(40);//
    cv->setCardBackgroundColor(0xff00ff00);
    cv->setContentPadding(40,40,40,40);
    w->setBackgroundColor(0xFFFFFFFF);
    w->addView(cv);
    return app.exec();
}
