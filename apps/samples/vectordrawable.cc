#include <drawables/vectordrawable.h>
#include <drawables/drawableinflater.h>
#include <cdroid.h>
int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    TextView*tv=new TextView("AnimatedVectorDrawable testcase",0,0);
    w->setBackgroundColor(0xFF112233);
    w->addView(tv);
    Drawable *d = nullptr;
    /*Animator*anim=AnimatorInflater::loadAnimator(&app,"cdroid:anim/btn_radio_to_off_mtrl_ring_outer_animation");
    Animator*anim2=anim->clone();
    LOGD("anim=%p anim2=%d",anim,anim2);
    delete anim;
    delete anim2;*/
    if(argc>1)d=DrawableInflater::loadDrawable(&app,argv[1]);
    else d= DrawableInflater::loadDrawable(&app,"@cdroid:drawable/btn_radio_on_to_off_mtrl_animation");
    tv->setBackground(d);
    if(dynamic_cast<AnimatedVectorDrawable*>(d)){
        ((AnimatedVectorDrawable*)d)->start();
    }
    app.exec();
    return 0;
}
