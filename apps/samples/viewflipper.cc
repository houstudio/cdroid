#include<cdroid.h>
#include<animation/animations.h>
#include<cdlog.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,1280,640);
    ViewFlipper*vfp=new ViewFlipper(400,600);
    w->addView(vfp);
    vfp->setFlipInterval(1500);
    ScaleAnimation*anim=new ScaleAnimation(0,1.f,0.f,1.f,200,300);
    anim->setDuration(1000);
    vfp->setInAnimation(anim);
    ScaleAnimation*anim2= new ScaleAnimation(0,1.f,0.f,1.f,200,300);
    anim2->setDuration(1000);
    vfp->setOutAnimation(anim2); 
    for(int i=0;i<10;i++){
        TextView*tv=new TextView(std::to_string(i),400,600);
        tv->setTextSize(120);
        tv->setTextAlignment(View::TEXT_ALIGNMENT_CENTER);//setGravity(Gravity::CENTER);
        tv->setTextColor(0xFFFFFFFF);
        tv->setBackgroundColor(0xFF000000|(i*10)<<((i%2+1)*8));
        vfp->addView(tv,i,new ViewFlipper::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::MATCH_PARENT));
    }
    vfp->startFlipping(); 
    w->requestLayout();
    app.exec();
}
