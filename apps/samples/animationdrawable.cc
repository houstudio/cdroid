#include <cdroid.h>
#include <cdlog.h>
#include <widget/R.h>   // public cdroid::R (android.R role)
#include <fstream>

int main(int argc,const char*argv[]){
    App app(argc,argv);

    // Framework resource by name at runtime (AOSP Resources.getIdentifier) —
    // public and internal names alike, no R header needed. Images live in the
    // drawable namespaces now (the pre-arsc mipmap/ directory convention is gone).
    auto fwid = [&app](const char* name) {
        return app.getResources().getIdentifier(name, "drawable", "android");
    };
    cdroid::Context*ctx=&app;
    Window*w=new Window(0,0,1280,800);

    // Window::doLayout now always lays out direct children, so absolute layout() on
    // multiple direct children piles them up at (0,0). Stack them in a LinearLayout.
    LinearLayout*content=new LinearLayout(&App::getInstance());
    content->setOrientation(LinearLayout::VERTICAL);
    w->addView(content);
    auto add=[&](View*v,int ww,int hh){
        LinearLayout::LayoutParams*lp=new LinearLayout::LayoutParams(ww,hh);
        lp->leftMargin=lp->topMargin=10;
        content->addView(v,lp);
    };

    ProgressBar*pb4=new ProgressBar(&App::getInstance());   // progressBarStyle default
    AnimationDrawable*ad=new AnimationDrawable();
    ad->addFrame(new ColorDrawable(0xFFFF0000),500);
    ad->addFrame(new ColorDrawable(0xFF00FF00),500);
    ad->addFrame(new ColorDrawable(0xFF0000FF),500);
    // bare ic_launcher does not exist; the android variant is the launcher asset.
    BitmapDrawable*bd=(BitmapDrawable*)ctx->getDrawable(fwid("ic_launcher_android"));   // demo content, resolved by name
    bd->setTileModeXY(TileMode::MIRROR,TileMode::MIRROR);
    ad->addFrame(bd,1000);
    ad->addFrame(ctx->getDrawable(fwid("progress_horizontal")),1000);
    pb4->setIndeterminateDrawable(ad);
    pb4->setIndeterminate(true);
    add(pb4,256,256);
    //pb4->setProgressDrawable(new ColorDrawable(0xFF111111));

    ProgressBar*pb5=new ProgressBar(&app,nullptr,cdroid::R::attr::progressBarStyleHorizontal);
    pb5->setIndeterminate(true);
    pb5->setProgress(40);
    //pb5->setProgressDrawable(new ColorDrawable(0xFF111111));
    add(pb5,600,20);

    content->requestLayout();
    return app.exec();
}
