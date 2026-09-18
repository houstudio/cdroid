#include <cdroid.h>
#include <cdlog.h>
#include <fstream>
#include <widget/keyboardview.h>
#include <widget/internal_R.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,1280,720);
    KeyboardView*kbdv=new KeyboardView(&App::getInstance());
    Keyboard*kbd= new Keyboard(&app,cdroid::internal::R::xml::qwerty,1280,240);
    kbdv->setKeyboard(kbd);
    w->addView(kbdv);
    return app.exec();
}
