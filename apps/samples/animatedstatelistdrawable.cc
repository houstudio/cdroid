#include <cdroid.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w = new Window(0,0,-1,-1);
    AnimatedStateListDrawable*d=(AnimatedStateListDrawable*)app.getDrawable("cdroid:drawable/switch_thumb_material_anim");
    w->setBackground(d);
    return app.exec();
}
