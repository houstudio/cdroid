#include <drawables/vectordrawable.h>
#include <drawables/drawableinflater.h>
#include <core/path.h>
#include <cdroid.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    TextView*tv=new TextView("AnimatedVectorDrawable",0,0);
    w->setBackgroundColor(0xFF112233);
    w->addView(tv);
    Drawable *d = nullptr;
    if(argc>1){
        if(strstr(argv[1],".xml"))d=DrawableInflater::loadDrawable(&app,argv[1]);
    }else{
        d = DrawableInflater::loadDrawable(&app,"@cdroid:drawable/btn_check_material_anim");
    }
    LOGD("drawable=%p",d);
    tv->setBackground(d);
    if(dynamic_cast<AnimatedVectorDrawable*>(d)){
        //d->setLevel(100);
        LOGD("AnimatedVectorDrawable");
        ((AnimatedVectorDrawable*)d)->start();
    }
    if(dynamic_cast<AnimatedImageDrawable*>(d)){
        ((AnimatedImageDrawable*)d)->start();
        LOGD("===webp");
    }
    tv->setOnClickListener([](View&view){
        Drawable *d =view.getBackground();
        static bool checked = false;
        d->setState(checked?StateSet::CHECKED_STATE_SET:StateSet::NOTHING);
        LOGD("checked=%d",checked);
        checked=!checked;
        if(dynamic_cast<AnimatedVectorDrawable*>(d)){
            ((AnimatedVectorDrawable*)d)->start();
        }
    });
    app.exec();
    return 0;
}
