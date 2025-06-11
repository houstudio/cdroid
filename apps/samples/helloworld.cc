#include <cdroid.h>
#include <porting/cdlog.h>
#include <widget/candidateview.h>
#include <widget/chronometer.h>
#include <core/bundle.h>
#include <core/process.h>
int main(int argc,const char*argv[]){
#if defined(__linux__)||defined(__unix__)
    setenv("LANG","zh_CN.UTF-8",1);
#endif
    App app(argc,argv);
    Bundle bd;
    bd.putInt("age",123);
    LOGD("bd.age=%d",bd.getInt("age"));
    LOGD("bool.size=%d float.size=%d ponter.size=%d string.size=%d View.size=%d function.size=%d Runnable.size=%d Insets.size=%d"
         " shared_ptr.size=%d vector.size=%d map.size=%d",  sizeof(bool),sizeof(float),sizeof(void*),sizeof(std::string),
         sizeof(View),sizeof(std::function<void()>),sizeof(Runnable),sizeof(Insets),
         sizeof(std::shared_ptr<int>),sizeof(std::vector<int>),sizeof(std::map<int,int>));
    Window*w=new Window(12,23,640,480);
    w->setId(10000);
    w->setBackgroundColor(0xFF4488cc);
   #if 10 
    EditText*edit=new EditText("Hello world! This value is positive for typical fonts that include",640,200);
    ColorStateList*cl=app.getColorStateList("cdroid:attr/editTextColor");
    if(cl){edit->setTextColor(cl);cl->dump();}
    else edit->setTextColor(0xFFFFFFFF);
    edit->setFocusableInTouchMode(true);
    LOGD("clist=%p",cl);
    edit->setSingleLine(false);
    edit->setClickable(true);
    edit->setInputType(EditText::TYPE_ANY);
    edit->setGravity(Gravity::LEFT|Gravity::CENTER_VERTICAL);
    edit->setBackgroundColor(0x80FF0000);
    w->addView(edit).layout(100,300,640,200);
    edit->setTextSize(60);
    Chronometer*ch=new Chronometer(200,40);
    ch->setFormat("Time:MM:SS");
    w->addView(ch).layout(0,400,200,40);
    ch->setCountDown(true);
    ch->setBase(SystemClock::elapsedRealtime() + 120 * 1000);
    ch->start();
    LOGD("window.xy=(%.f,%.f) LT=(%d,%d)",w->getX(),w->getY(),w->getLeft(),w->getTop());
    #endif
#if 0
    LayerDrawable*ld=(LayerDrawable*)app.getDrawable("cdroid:drawable/analog.xml");
    clk->setBackgroundDrawable(ld);
    ld->setCallback(clk);
    AnimatedRotateDrawable*ad=(AnimatedRotateDrawable*)ld->getDrawable(1);
    ad->setFramesDuration(500);
    ad->start();
    ad=(AnimatedRotateDrawable*)ld->getDrawable(2);
    ad->setFramesDuration(500);
    ad->start();
//#else
    AnimatedRotateDrawable*ad=(AnimatedRotateDrawable*)app.getDrawable("cdroid:drawable/animate.xml");
    ad->setFramesDuration(500);
    ad->start();
    clk->setBackgroundDrawable(ad);
#endif
    Process p(Looper::myLooper());
    w->animate().setDuration(5000).translationX(100).scaleX(0.5).scaleY(0.5).start();
    p.exec("ls",{"-l"},[](Process&p){std::cout<<p.readAllStandardOutput();});
    return app.exec();
}
