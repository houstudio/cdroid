#include <cdroid.h>
#include <cdlog.h>
#include <widget/candidateview.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window* w = new Window(0,0,-1,-1);
    w->setBackgroundColor(0xFF112233);
    Toast::makeText(&app,"Press OK to exit",4000)->show();
    return app.exec();
}
