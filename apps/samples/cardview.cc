#include <cdroid.h>
#include <cdlog.h>
#include <fstream>
#include <widget/cardview.h>
int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    CardView*cv=new CardView(0,0);
    cv->setRadius(40);
    cv->setMaxCardElevation(40);//MaxCardElevation must be set before setCardElevation or setElevation
    cv->setElevation(40);//
    cv->setCardBackgroundColor(0xff00ff00);
    w->setBackgroundColor(0xFFFFFFFF);
    w->addView(cv);
    return app.exec();
}
