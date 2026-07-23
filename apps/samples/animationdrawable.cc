#include <cdroid.h>
#include <cdlog.h>
#include <fstream>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    cdroid::Context*ctx=&app;
    Window*w=new Window(0,0,1280,800);

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

    ProgressBar*pb4=new ProgressBar(256,256);
    AnimationDrawable*ad=new AnimationDrawable();
    ad->addFrame(new ColorDrawable(0xFFFF0000),500);
    ad->addFrame(new ColorDrawable(0xFF00FF00),500);
    ad->addFrame(new ColorDrawable(0xFF0000FF),500);
    BitmapDrawable*bd=(BitmapDrawable*)ctx->getDrawable("cdroid:mipmap/ic_launcher");
    bd->setTileModeXY(TileMode::MIRROR,TileMode::MIRROR);
    ad->addFrame(bd,1000);
    ad->addFrame(ctx->getDrawable("cdroid:drawable/progress_horizontal.xml"),1000);
    pb4->setIndeterminateDrawable(ad);
    pb4->setIndeterminate(true);
    add(pb4,256,256);
    //pb4->setProgressDrawable(new ColorDrawable(0xFF111111));

    ProgressBar*pb5=new ProgressBar(600,20);
    pb5->setIndeterminateDrawable(ctx->getDrawable("cdroid:drawable/progress_indeterminate_horizontal_holo"));
    pb5->setIndeterminate(true);
    pb5->setProgress(40);
    //pb5->setProgressDrawable(new ColorDrawable(0xFF111111));
    add(pb5,600,20);

    content->requestLayout();
    return app.exec();
}
