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

    // Window::doLayout now always lays out direct children, so absolute layout() on
    // multiple direct children piles them up at (0,0). Stack them in a LinearLayout.
    LinearLayout*content=new LinearLayout(-1,-1);
    content->setOrientation(LinearLayout::VERTICAL);
    w->addView(content);
    auto add=[&](View*v,int ww,int hh){
        LinearLayout::LayoutParams*lp=new LinearLayout::LayoutParams(ww,hh);
        lp->leftMargin=lp->topMargin=10;
        content->addView(v,lp);
    };

    ImageView *btn=new ImageView(200,200);
    if(result.count("url")){
        std::string url = result["url"].as<std::string>();
        btn->setImageResource(url);
    }
    add(btn,200,200);

    ImageView*img=new ImageView(200,200);
    Drawable*dr=app.getDrawable("cdroid:mipmap/bottom_bar");//drawable/btn_radio.xml");
    img->setImageDrawable(dr);
    img->setCornerRadii(20);
    img->setScaleType(ScaleType::FIT_XY);
    img->setBackgroundColor(0xFF112233);
    add(img,200,200);

    content->requestLayout();
    return app.exec();
}
