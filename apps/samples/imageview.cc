#include <cdroid.h>
#include <cdlog.h>
#include <fstream>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    cdroid::Context*ctx=&app;
    Window*w=new Window(0,0,-1,-1);
    w->setId(1);
    Drawable*d=nullptr;
    StateListDrawable*sd;
    LOGD("test LOGF %d",__LINE__);
    LOG(DEBUG)<<"Test Stream(DEBUG)";

#if 1
    Button *btn=new Button("ClickME",200,80);
    w->addView(btn); 
    ImageView*img=new ImageView(200,100);
    Drawable*dr=ctx->getDrawable("cdroid:mipmap/bottom_bar");//drawable/btn_radio.xml");
    img->setImageDrawable(dr);
    img->setCornerRadii(20);
    img->setScaleType(ScaleType::FIT_XY);
    img->setBackgroundColor(0xFF112233);
    w->addView(img).setPos(0,100);
    w->requestLayout();	
#endif
    return app.exec();
}
