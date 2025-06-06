#include <cdroid.h>
#include <cdlog.h>
#include <fstream>
#include <core/cxxopts.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    cxxopts::Options options("main","application");
    options.add_options()("u,url","image url",cxxopts::value<std::string>());
    options.allow_unrecognised_options();
    auto result = options.parse(argc,argv);

    Window*w = new Window(0,0,-1,-1);
    w->setId(1);
    ImageView *btn=new ImageView(200,200);
    if(result.count("url")){
        std::string url = result["url"].as<std::string>();
        btn->setImageResource(url);
    }
    w->addView(btn).setPos(220,100); 
    ImageView*img=new ImageView(200,200);
    Drawable*dr=app.getDrawable("cdroid:mipmap/bottom_bar");//drawable/btn_radio.xml");
    img->setImageDrawable(dr);
    img->setCornerRadii(20);
    img->setScaleType(ScaleType::FIT_XY);
    img->setBackgroundColor(0xFF112233);
    w->addView(img).setPos(0,100);
    w->requestLayout();	
    return app.exec();
}
