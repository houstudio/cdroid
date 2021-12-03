#include <windows.h>
#include <cdlog.h>
#include <fstream>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    cdroid::Context*ctx=&app;
    Window*w=new Window(0,0,1280,720);
    w->setId(1);
    Drawable*d=nullptr;
///////////////////////////////////////////////////////////
    ProgressBar*pb = new ProgressBar(600,40);
    ProgressBar*pb2= new ProgressBar(600,40);
    d=ctx->getDrawable("cdroid:drawable/progress_horizontal.xml");
    LOGD("progress_horizontal drawable=%p",d);
    pb->setProgressDrawable(d);
    pb->setProgress(34);
    pb2->setProgressDrawable(d->getConstantState()->newDrawable());
    pb2->setProgress(34);
    pb2->setMirrorForRtl(true);
    pb2->setLayoutDirection(View::LAYOUT_DIRECTION_RTL);
    pb->setSecondaryProgress(15);
    w->addView(pb).setPos(150,50);
    w->addView(pb2).setPos(150,100);
    Runnable progress;
    progress=[w,pb,pb2,&progress](){
        pb->setProgress((pb->getProgress()+23)%100);
        pb2->setProgress((pb->getProgress()+13)%100);
        w->postDelayed(progress,2000);
    };
    w->post(progress);

    ProgressBar*pb3=new ProgressBar(72,72);
    d=ctx->getDrawable("cdroid:drawable/progress_large.xml");
    pb3->setIndeterminateDrawable(d);
    LOGD("Indeterminate drawable=%p",d);
    w->addView(pb3).setId(104).setPos(800,60);
    pb3->setProgressDrawable(new ColorDrawable(0xFF112233));
    pb3->setIndeterminate(true);

//////////////////////////////////////////////////////////    

    SeekBar*sb = new SeekBar(800,50);
    SeekBar*sb2= new SeekBar(800,50);

    d=ctx->getDrawable("cdroid:drawable/progress_horizontal.xml");
    sb->setProgressDrawable(d);
    sb2->setProgressDrawable(d->getConstantState()->newDrawable());

    d=ctx->getDrawable("cdroid:drawable/seek_thumb.xml");
    sb->setThumb(d);
    sb2->setThumb(d->getConstantState()->newDrawable());
    d=ctx->getDrawable("cdroid:drawable/seekbar_tick_mark.xml");
    sb->setTickMark(d);
    sb2->setTickMark(d->getConstantState()->newDrawable());
    sb2->setMirrorForRtl(true);
    w->addView(sb).setId(200).setPos(150,240).setKeyboardNavigationCluster(true);
    w->addView(sb2).setId(201).setPos(150,300).setLayoutDirection(View::LAYOUT_DIRECTION_RTL);
    return app.exec();
}
