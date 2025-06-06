#include <cdroid.h>
#include <core/cxxopts.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    cxxopts::Options options("main","application");
    options.add_options()("scroll","scroll",cxxopts::value<int>()->default_value("1"));
    auto result = options.parse(argc,argv);
    Window*w = new Window(0,0,-1,-1);
    HorizontalScrollView* hs=new HorizontalScrollView(-1,-1);
    hs->setOverScrollMode(result.count("scroll")?View::OVER_SCROLL_ALWAYS:View::OVER_SCROLL_NEVER);
    hs->setHorizontalFadingEdgeEnabled(true);
    hs->setFadingEdgeLength(200);
    hs->setHorizontalScrollBarEnabled(true);
    LinearLayout*layout=new LinearLayout(LayoutParams::WRAP_CONTENT,LayoutParams::WRAP_CONTENT);
    layout->setOrientation(LinearLayout::HORIZONTAL);

    auto click=[](View&v){
       LOGD("You clicked Button %d",v.getId());
    };
    for(int i=0;i<30;i++){
        TextView*tv=new TextView(150,30);
        tv->setText("Hello"+std::to_string(i));
        tv->setBackgroundColor(0xFF000000|i*632);
        LinearLayout::LayoutParams*lp=new LinearLayout::LayoutParams(120/*LayoutParams::WRAP_CONTENT*/,LayoutParams::MATCH_PARENT);
        lp->setMarginsRelative(20,0,20,0);
        tv->setTextAlignment(View::TEXT_ALIGNMENT_GRAVITY);
        tv->setGravity(Gravity::CENTER);
        layout->addView(tv,lp).setId(1000+i);
        tv->setOnClickListener([](View&v){
            LOGD("You clicked Button %d",v.getId());
        });
    }
    hs->addView(layout,new FrameLayout::LayoutParams(LayoutParams::WRAP_CONTENT,-1)).setId(100);
    w->addView(hs).setId(10);
    layout->requestLayout();
    w->requestLayout();
    w->setId(1);
    app.exec();
}
