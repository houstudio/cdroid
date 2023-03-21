#include <cdroid.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    HorizontalScrollView* hs=new HorizontalScrollView(1280,400);
    //hs->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    hs->setHorizontalScrollBarEnabled(true);

    LinearLayout*layout=new LinearLayout(1280,100);
    layout->setOrientation(LinearLayout::HORIZONTAL);

    auto click=[](View&v){
       LOGD("You clicked Button %d",v.getId());
    };
    for(int i=0;i<30;i++){
        TextView*tv=new Button(0,0);//TextView(150,30);
        tv->setText("Hello"+std::to_string(i));
        tv->setBackgroundColor(0xFF000000|i*632);
        LinearLayout::LayoutParams*lp=new LinearLayout::LayoutParams(120/*LayoutParams::WRAP_CONTENT*/,LayoutParams::MATCH_PARENT);
        lp->setMarginsRelative(20,0,20,0);
        tv->setTextAlignment(View::TEXT_ALIGNMENT_GRAVITY);
        tv->setGravity(Gravity::CENTER);
        tv->setId(i);
        layout->addView(tv,lp);
        tv->setFocusable(true);
        tv->setOnClickListener([](View&v){
            LOGD("You clicked Button %d",v.getId());
        });
    }
    hs->addView(layout,new FrameLayout::LayoutParams(LayoutParams::WRAP_CONTENT,-1));
    w->addView(hs);
    layout->requestLayout();
    w->requestLayout();
    app.exec();
}
