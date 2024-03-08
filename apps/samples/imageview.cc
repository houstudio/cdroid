#include <cdroid.h>
#include <cdlog.h>
#include <fstream>
static const std::vector<CLA::Argument> ARGS={
   {CLA::EntryType::Option, "u", "url",  "url to download", CLA::ValueType::String, (int)CLA::EntryFlags::Optional},
};

int main(int argc,const char*argv[]){
    App app(argc,argv,ARGS);
    cdroid::Context*ctx=&app;
    Window*w=new Window(0,0,-1,-1);
    w->setId(1);
    ImageView *btn=new ImageView(200,200);
    std::string url =app.getArg("url","./595f63928c504efaa7f215c8aef3bc9c.jpeg");
    btn->setImageResource(url);
    w->addView(btn).setPos(220,100); 
    ImageView*img=new ImageView(200,200);
    Drawable*dr=ctx->getDrawable("cdroid:mipmap/bottom_bar");//drawable/btn_radio.xml");
    img->setImageDrawable(dr);
    img->setCornerRadii(20);
    img->setScaleType(ScaleType::FIT_XY);
    img->setBackgroundColor(0xFF112233);
    w->addView(img).setPos(0,100);
    w->requestLayout();	
    return app.exec();
}
