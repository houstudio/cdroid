#include <cdroid.h>
#include <cdlog.h>
#include <fstream>
#include <widgetEx/wear/circularprogresslayout.h>
#include <widget/R.h>   // public cdroid::R (android.R role)
int main(int argc,const char*argv[]){
    App app(argc,argv);

    // Framework resource by name at runtime (AOSP Resources.getIdentifier) —
    // public and internal names alike, no R header needed. Images live in the
    // drawable namespaces now (the pre-arsc mipmap/ directory convention is gone).
    auto fwid = [&app](const char* name) {
        return app.getResources().getIdentifier(name, "drawable", "android");
    };
    cdroid::Context*ctx=&app;
    Window*w=new Window(0,0,-1,-1);
    w->setId(1);
    Drawable*d=nullptr;

    // Window::doLayout now always measures+lays out its (FrameLayout) children, so
    // views added directly to the Window pile up at (0,0). Wrap the widget zoo in a
    // vertical LinearLayout inside a ScrollView; any number of widgets stack+scroll.
    ScrollView*scroller=new ScrollView(&app);
    scroller->setSmoothScrollingEnabled(true);
    scroller->setVerticalScrollBarEnabled(true);
    w->addView(scroller);
    LinearLayout*content=new LinearLayout(&app);
    content->setOrientation(LinearLayout::VERTICAL);
    scroller->addView(content);
    auto row=[&](View*v,int width,int height){
        LinearLayout::LayoutParams*lp=new LinearLayout::LayoutParams(width,height);
        lp->topMargin=lp->leftMargin=8;
        content->addView(v,lp);
    };
///////////////////////////////////////////////////////////
    ProgressBar*pb = new ProgressBar(&app,nullptr,cdroid::R::attr::progressBarStyleHorizontal);
    ProgressBar*pb2= new ProgressBar(&app,nullptr,cdroid::R::attr::progressBarStyleHorizontal);

    d=ctx->getDrawable(fwid("progress_horizontal"));
    LOGI("progress_horizontal drawable=%p",d);
    pb->setProgressDrawable(d);
    pb->setProgress(34);
    pb2->setProgressDrawable(d->getConstantState()->newDrawable());
    pb2->setProgress(34);
    pb2->setMirrorForRtl(true);
    pb2->setLayoutDirection(View::LAYOUT_DIRECTION_RTL);
    pb->setSecondaryProgress(15);
    row(pb,600,40);
    row(pb2,600,40);

    Runnable progress;
    progress=[w,pb,pb2,&progress](){
        pb->setSecondaryProgress((pb->getSecondaryProgress()+1)%100);
        pb->setProgress(time(nullptr)%100);
        pb2->setProgress((pb->getProgress()+13)%100);
        w->postDelayed(progress,200);
    };
    w->post(progress);

    ProgressBar*pb3=new ProgressBar(&app);
    d=ctx->getDrawable(fwid("progress_large"));
    pb3->setIndeterminateDrawable(d);
    LOGI("Indeterminate drawable=%p",d);
    pb3->setOnClickListener([&pb3](View&v){
         static int cc=0;
         pb3->setIndeterminate(cc++%2);
    });
    pb3->setId(104);
    pb3->setIndeterminate(true);
    pb3->setProgressDrawable(new ColorDrawable(0xFF112233));
    row(pb3,72,72);

    ProgressBar*pb4=new ProgressBar(&app);
    AnimationDrawable*ad=new AnimationDrawable();
    ad->addFrame(new ColorDrawable(0xFFFF0000),500);
    ad->addFrame(new ColorDrawable(0xFF00FF00),500);
    ad->addFrame(new ColorDrawable(0xFF0000FF),500);
    // bare ic_launcher does not exist; the android variant is the launcher asset.
    BitmapDrawable*bd=(BitmapDrawable*)ctx->getDrawable(fwid("ic_launcher_android"));
    bd->setTileModeXY(TileMode::MIRROR,TileMode::MIRROR);
    ad->addFrame(bd,1000);
    ad->addFrame(ctx->getDrawable(fwid("progress_horizontal")),1000);
    pb4->setId(105);
    pb4->setIndeterminateDrawable(ad);
    pb4->setIndeterminate(true);
    row(pb4,256,256);
    //pb4->setProgressDrawable(new ColorDrawable(0xFF111111));

    ProgressBar*pb5=new ProgressBar(&app,nullptr,cdroid::R::attr::progressBarStyleHorizontal);
    pb5->setIndeterminateDrawable(ctx->getDrawable(fwid("progress_indeterminate_horizontal_holo")));
    pb5->setIndeterminate(true);
    pb5->setProgress(40);
    pb5->setId(105);
    //pb5->setProgressDrawable(new ColorDrawable(0xFF111111));
    row(pb5,600,20);
//////////////////////////////////////////////////////////
    SeekBar*sb = new SeekBar(&app);
    SeekBar*sb2= new SeekBar(&app);

    d=ctx->getDrawable(fwid("progress_horizontal"));
    sb->setProgressDrawable(d);
    sb2->setProgressDrawable(d->getConstantState()->newDrawable());
    sb2->setOnSeekBarChangeListener({
            .onProgressChanged=[](SeekBar&sb,int progress,bool)->void{LOGD("progress=%d",progress);},
            .onStartTrackingTouch=[](SeekBar&sb){LOGD("onStartTrackingTouch");},
            .onStopTrackingTouch=[](SeekBar&sb){LOGD("onStopTrackingTouch");}
        });

    d=ctx->getDrawable(fwid("seek_thumb"));
    sb->setThumb(d);
    sb2->setThumb(d->getConstantState()->newDrawable());
    // legacy seekbar_tick_mark is absent from the slim res set; material is the default.
    d=ctx->getDrawable(fwid("seekbar_tick_mark_material"));
    sb->setTickMark(d);
    sb2->setTickMark(d->getConstantState()->newDrawable());
    sb2->setMirrorForRtl(true);
    sb->setId(200);
    row(sb,800,50);
    sb2->setId(201);
    sb2->setLayoutDirection(View::LAYOUT_DIRECTION_RTL);
    row(sb2,800,50);

    /*ShapeDrawable* shapeDrawable = new ShapeDrawable();
    shapeDrawable->setShape(new OvalShape());
    shapeDrawable->getShape()->setSolidColor(0xFF4488aa);
    pb5 =new ProgressBar(&app);
    pb5->setProgressDrawable(shapeDrawable);
    pb5->setProgress(35);
    row(pb5,200,200);*/

    content->requestLayout();
    return app.exec();
}
