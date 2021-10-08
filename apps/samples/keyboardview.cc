#include <windows.h>
#include <cdlog.h>
#include <fstream>
#include <widget/keyboardview.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,1280,720);
    KeyboardView*kbdv=new KeyboardView(1280,240);
    Keyboard*kbd= new Keyboard(&app,"cdroid:xml/qwerty.xml",1280,240);
    kbdv->setKeyboard(kbd);
    w->addView(kbdv);
    return app.exec();
}
