#include <cdroid.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);

    // Framework resource by name at runtime (AOSP Resources.getIdentifier) —
    // public and internal names alike, no R header needed. Images live in the
    // drawable namespaces now (the pre-arsc mipmap/ directory convention is gone).
    auto fwid = [&app](const char* name) {
        return app.getResources().getIdentifier(name, "drawable", "android");
    };
    Window*w = new Window(0,0,-1,-1);
    AnimatedStateListDrawable*d=(AnimatedStateListDrawable*)app.getDrawable(fwid("switch_thumb_material_anim"));
    w->setBackground(d);
    return app.exec();
}
