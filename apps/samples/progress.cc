#include <cdroid.h>
#include <cdlog.h>
#include <fstream>
#include <widgetEx/wear/circularprogresslayout.h>
int main(int argc,const char*argv[]){
    App app(argc,argv);
    cdroid::Context*ctx=&app;
    Window*w=new Window(0,0,-1,-1);
    w->setId(1);
    Drawable*d=nullptr;
///////////////////////////////////////////////////////////
    ProgressBar*pb = new ProgressBar(600,40);
    ProgressBar*pb2= new ProgressBar(600,40);
    CircularProgressLayout*cpl=new CircularProgressLayout(256,256);
    w->addView(cpl);
    cpl->layout(50,360,256,256);
    cpl->getProgressDrawable()->setBackgroundColor(0xFF112233);
    cpl->getProgressDrawable()->setColorSchemeColors({0x7FFF0000,0x7F00FF00});
    cpl->getProgressDrawable()->setStartEndTrim(0,0.38f);
    cpl->getProgressDrawable()->start();

    d=ctx->getDrawable("cdroid:drawable/progress_horizontal.xml");
    LOGI("progress_horizontal drawable=%p",d);
    pb->setProgressDrawable(d);
    pb->setProgress(34);
    pb2->setProgressDrawable(d->getConstantState()->newDrawable());
    pb2->setProgress(34);
    pb2->setMirrorForRtl(true);
    pb2->setLayoutDirection(View::LAYOUT_DIRECTION_RTL);
    pb->setSecondaryProgress(15);
    w->addView(pb);
    pb->layout(50,50,600,40);
    w->addView(pb2);
    pb2->layout(50,100,600,40);

    Runnable progress;
    progress=[w,pb,pb2,&progress](){
        pb->setSecondaryProgress((pb->getSecondaryProgress()+1)%100);
        pb->setProgress(time(nullptr)%100);
        pb2->setProgress((pb->getProgress()+13)%100);
        w->postDelayed(progress,200);
    };
    w->post(progress);

    ProgressBar*pb3=new ProgressBar(72,72);
    d=ctx->getDrawable("cdroid:drawable/progress_large");
    pb3->setIndeterminateDrawable(d);
    LOGI("Indeterminate drawable=%p",d);
    w->addView(pb3);
    pb3->setId(104);
    pb3->layout(680,60,72,72);
    pb3->setIndeterminate(true);
    pb3->setProgressDrawable(new ColorDrawable(0xFF112233));

    ProgressBar*pb4=new ProgressBar(256,256);
    AnimationDrawable*ad=new AnimationDrawable();
    ad->addFrame(new ColorDrawable(0xFFFF0000),500);
    ad->addFrame(new ColorDrawable(0xFF00FF00),500);
    ad->addFrame(new ColorDrawable(0xFF0000FF),500);
    BitmapDrawable*bd=(BitmapDrawable*)ctx->getDrawable("cdroid:mipmap/ic_launcher");
    bd->setTileModeXY(TileMode::MIRROR,TileMode::MIRROR);
    ad->addFrame(bd,1000);
    ad->addFrame(ctx->getDrawable("cdroid:drawable/progress_horizontal"),1000);
    w->addView(pb4);
    pb4->setId(105);
    pb4->layout(600,300,256,256);
    pb4->setIndeterminateDrawable(ad);
    pb4->setIndeterminate(true);
    //pb4->setProgressDrawable(new ColorDrawable(0xFF111111));

    ProgressBar*pb5=new ProgressBar(600,20);
    pb5->setIndeterminateDrawable(ctx->getDrawable("cdroid:drawable/progress_indeterminate_horizontal_holo"));
    pb5->setIndeterminate(true);
    pb5->setProgress(40);
    //pb5->setProgressDrawable(new ColorDrawable(0xFF111111));   
    w->addView(pb5);
    pb5->setId(105);
    pb5->layout(50,150,600,20);
//////////////////////////////////////////////////////////
    SeekBar*sb = new SeekBar(800,50);
    SeekBar*sb2= new SeekBar(800,50);

    d=ctx->getDrawable("cdroid:drawable/progress_horizontal");
    sb->setProgressDrawable(d);
    sb2->setProgressDrawable(d->getConstantState()->newDrawable());
    sb2->setOnSeekBarChangeListener((const SeekBar::OnSeekBarChangeListener){
            .onProgressChanged=[](SeekBar&sb,int,bool)->void{return;},
            .onStartTrackingTouch=[](SeekBar&sb)->void{},
            .onStopTrackingTouch=[](SeekBar&)->void{}
        });

    d=ctx->getDrawable("cdroid:drawable/seek_thumb.xml");
    sb->setThumb(d);
    sb2->setThumb(d->getConstantState()->newDrawable());
    d=ctx->getDrawable("cdroid:drawable/seekbar_tick_mark.xml");
    sb->setTickMark(d);
    sb2->setTickMark(d->getConstantState()->newDrawable());
    sb2->setMirrorForRtl(true);
    w->addView(sb);
    sb->setId(200);
    sb->layout(10,180,800,50);
    w->addView(sb2);
    sb2->setId(201);
    sb2->layout(10,234,800,50);
    sb2->setLayoutDirection(View::LAYOUT_DIRECTION_RTL);

    /*ShapeDrawable* shapeDrawable = new ShapeDrawable();
    shapeDrawable->setShape(new OvalShape());
    shapeDrawable->getShape()->setSolidColor(0xFF4488aa);
    pb5 =new ProgressBar(200,200);
    w->addView(pb5);
    pb5->layout(50,300,200,200);
    pb5->setProgressDrawable(shapeDrawable);
    pb5->setProgress(35);*/
    return app.exec();
}
