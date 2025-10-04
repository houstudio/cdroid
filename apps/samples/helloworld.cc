#include <cdroid.h>
#include <porting/cdlog.h>
#include <widget/candidateview.h>
#include <widget/chronometer.h>
#include <core/bundle.h>
#include <core/process.h>
#include <widgetEx/wear/confirmationoverlay.h>
//#include <mgl2/mgl.h>
int main(int argc,const char*argv[]){
#if defined(__linux__)||defined(__unix__)
    setenv("LANG","zh_CN.UTF-8",1);
#endif
    App app(argc,argv);
    Window*w=new Window(12,23,640,480);
    w->setId(10000);
    w->setBackgroundColor(0xFF4488cc);
    ConfirmationOverlay* cfo=new ConfirmationOverlay();
    cfo->setMessage("Hello world from ConfirmationOverlay").showAbove(w);
    return app.exec();
}
