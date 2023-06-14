#include <cdroid.h>
#include <cdlog.h>
#include <widget/candidateview.h>

int main(int argc,const char*argv[]){
    setenv("LANG","zh_CN.UTF-8",1);
    App app(argc,argv);
    LOGD("bool.size=%d float.size=%d ponter.size=%d string.size=%d View.size=%d function.size=%d Runnable.size=%d Insets.size=%d"
         " shared_ptr.size=%d vector.size=%d map.size=%d",  sizeof(bool),sizeof(float),sizeof(void*),sizeof(std::string),
         sizeof(View),sizeof(std::function<void()>),sizeof(Runnable),sizeof(Insets),
         sizeof(std::shared_ptr<int>),sizeof(std::vector<int>),sizeof(std::map<int,int>));
    Window*w=new Window(0,0,-1,-1);
    w->setId(10000);
    w->setBackgroundColor(0xFF111111);
   #if 10 
    EditText*edit=new EditText("Hello world! This value is positive for typical fonts that include",680,200);
    ColorStateList*cl=app.getColorStateList("cdroid:attr/editTextColor");
    if(cl){edit->setTextColor(cl);cl->dump();}
    else edit->setTextColor(0xFFFFFFFF);
    edit->setFocusableInTouchMode(true);
    LOGD("clist=%p",cl);
    edit->setSingleLine(false);
    edit->setClickable(true);
    edit->setInputType(EditText::TYPE_ANY);
    edit->setGravity(Gravity::LEFT|Gravity::CENTER_VERTICAL);
    edit->setBackgroundColor(0xFFFF0000);
    w->addView(edit).setPos(100,300);
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
