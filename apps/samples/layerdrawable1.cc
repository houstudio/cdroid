#include <cdroid.h>
#include <cdlog.h>

View* createClockView(const std::string&bgres,int num,...){
    View*v=new View(320,320);
    LayerDrawable*ld=new LayerDrawable();
    cdroid::Context*ctx=&App::getInstance();
    ld->addLayer(new BitmapDrawable(ctx,bgres));//fixed background layer
    va_list ap;
    va_start(ap,num);
    for(int i=0;i<num;i++){
        const char*s=va_arg(ap,const char*);
        Drawable*d =new RotateDrawable(new BitmapDrawable(ctx,s));//add rotate drawable for other layers
        LOGD("drawable %p:%s",d,s);
        ld->addLayer(d);
    }
    v->setBackground(ld);
    return v;
}

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,800,600);

    View*v1=createClockView("cdroid:mipmap/watch.png",1,"cdroid:mipmap/clock_dial.png");
    w->addView(v1).layout(20,20,320,320);

    View*v2=createClockView("cdroid:mipmap/clock_dial.png",2,"cdroid:mipmap/clock_hand_hour.png","cdroid:mipmap/clock_hand_minute.png");
    w->addView(v2).layout(400,20,320,320);

    int level=0;
    Runnable clock;clock=[&](){
        Drawable*d=v1->getBackground();
        d->setLevel(level);

        LayerDrawable*ld=dynamic_cast<LayerDrawable*>(v2->getBackground());
        d=ld->getDrawable(1);
        d->setLevel(level/10);//hour hand
        d=ld->getDrawable(2);
        d->setLevel(level);//minute hand
        level+=50;
        w->postDelayed(clock,100);
    };
    w->postDelayed(clock,100);
    app.exec();
}
