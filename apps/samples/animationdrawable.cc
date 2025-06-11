#include <cdroid.h>
#include <cdlog.h>
#include <fstream>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    cdroid::Context*ctx=&app;
    Window*w=new Window(0,0,1280,800);

    ProgressBar*pb4=new ProgressBar(256,256);
    AnimationDrawable*ad=new AnimationDrawable();
    ad->addFrame(new ColorDrawable(0xFFFF0000),500);
    ad->addFrame(new ColorDrawable(0xFF00FF00),500);
    ad->addFrame(new ColorDrawable(0xFF0000FF),500);
    BitmapDrawable*bd=(BitmapDrawable*)ctx->getDrawable("cdroid:mipmap/ic_launcher");
    bd->setTileModeXY(TileMode::MIRROR,TileMode::MIRROR);
    ad->addFrame(bd,1000);
    ad->addFrame(ctx->getDrawable("cdroid:drawable/progress_horizontal.xml"),1000);
    w->addView(pb4).setId(105).layout(800,10,256,256);
    pb4->setIndeterminateDrawable(ad);
    pb4->setIndeterminate(true);
    //pb4->setProgressDrawable(new ColorDrawable(0xFF111111));
    ProgressBar*pb5=new ProgressBar(600,20);
    pb5->setIndeterminateDrawable(ctx->getDrawable("cdroid:drawable/progress_indeterminate_horizontal_holo"));
    pb5->setIndeterminate(true);
    pb5->setProgress(40);
    //pb5->setProgressDrawable(new ColorDrawable(0xFF111111));   
    w->addView(pb5).setId(105).layout(50,150,600,20);
    return app.exec();
}
