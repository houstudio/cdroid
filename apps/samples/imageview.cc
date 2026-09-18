#include <cdroid.h>
#include <cdlog.h>
#include <fstream>
#include <core/cxxopts.h>

// Registered at static init, before App's ctor parses argv: App's --help then
// lists this app's options beside the framework's. Values are still read from
// this app's own cxxopts pass in main().
static const bool sAppOptionsRegistered = App::addAppOptions("Application",
        [](cxxopts::OptionAdder& add){
            add("u,url","image url",cxxopts::value<std::string>());
        });

int main(int argc,const char*argv[]){
    App app(argc,argv);

    // Framework resource by name at runtime (AOSP Resources.getIdentifier) —
    // public and internal names alike, no R header needed. Images live in the
    // drawable namespaces now (the pre-arsc mipmap/ directory convention is gone).
    auto fwid = [&app](const char* name) {
        return app.getResources().getIdentifier(name, "drawable", "android");
    };
    cxxopts::Options options("main","application");
    options.add_options()("u,url","image url",cxxopts::value<std::string>());
    options.allow_unrecognised_options();
    auto result = options.parse(argc,argv);

    Window*w = new Window(0,0,-1,-1);
    w->setId(1);

    // Window::doLayout now always lays out direct children, so absolute layout() on
    // multiple direct children piles them up at (0,0). Stack them in a LinearLayout.
    LinearLayout*content=new LinearLayout(&app);
    content->setOrientation(LinearLayout::VERTICAL);
    w->addView(content);
    auto add=[&](View*v,int ww,int hh){
        LinearLayout::LayoutParams*lp=new LinearLayout::LayoutParams(ww,hh);
        lp->leftMargin=lp->topMargin=10;
        content->addView(v,lp);
    };

    ImageView *btn=new ImageView(&app);
    if(result.count("url")){
        std::string url = result["url"].as<std::string>();
        btn->setImageResource(url);
    }
    add(btn,200,200);

    ImageView*img=new ImageView(&app);
    Drawable*dr=app.getDrawable(fwid("bottom_bar"));
    img->setImageDrawable(dr);
    img->setCornerRadii(20);
    img->setScaleType(ScaleType::FIT_XY);
    img->setBackgroundColor(0xFF112233);
    add(img,200,200);

    content->requestLayout();
    return app.exec();
}
