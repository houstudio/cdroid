#include <cdroid.h>
#include <string>
#include <cdlog.h>
#include <iostream>
#include <homewindow.h>
class MYWindow:public Window{
public:
   MYWindow():Window(0,0,-1,-1){}
   void onDraw(Canvas&canvas){
      canvas.set_font_size(128);
      canvas.move_to(100,100);
      canvas.set_source_rgb(1,0,0);
      canvas.show_text("28");
      canvas.fill();
   }
};
int main(int argc,const char*argv[]){
    App app(argc,argv);
    //Window*w=new Window(0,0,-1,-1);//HomeWindow(argc%2);
    Window*w=new HomeWindow();
    std::string str=app.getString("@string/weathertips");
    LOGD("str=%s",str.c_str());
    /*GradientDrawable*d=(GradientDrawable*)app.getDrawable("@drawable/test");
    d->setUseLevel(true);
    d->setInnerRadius(20);
    d->setThickness(20);
    d->setColor(0xff00ff);
    d->setLevel(argc==1?10000:atoi(argv[1]));//.f*359.f/360);
    w->setBackground(d);*/
    return app.exec();
}

