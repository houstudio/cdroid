#include<cdroid.h>
#include<cdlog.h>
int main(int argc,const char*argv[]){
    setenv("LANG","zh.CN",1);
    App app(argc,argv);
    LOGD("float.size=%d string.size=%d View.size=%d function.size=%d Runnable.size=%d map.size=%d Insets.size=%d",
         sizeof(float),sizeof(std::string),sizeof(View),sizeof(std::function<void()>),
        sizeof(Runnable),sizeof(std::map<int,int>),sizeof(Insets));
    Window*w=new Window(100,100,800,600);
   #if 10 
    EditText*edit=new EditText(TEXT("Hello world! This value is positive for typical fonts that include"),680,200);
    edit->setTextColor(0xFFFFFFFF);
    edit->setSingleLine(false);
    edit->setInputType(EditText::TYPE_ANY);
    edit->setGravity(Gravity::LEFT|Gravity::CENTER_VERTICAL);
    edit->setBackgroundColor(0xFFFF0000);
    w->addView(edit).setPos(100,0);
    edit->setTextSize(60);
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
    return app.exec();
}
