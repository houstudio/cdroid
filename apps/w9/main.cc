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
    //Window*w=new Window(0,0,-1,-1);
    Window*w=new HomeWindow();
    std::string str=app.getString("@string/weathertips");
    LOGD("str=%s",str.c_str());
    /*ColorStateList*cls=app.getColorStateList("@w9:color/btn_mode_text");
    TextView*tv=new TextView("Hello world!",200,200);
    w->addView(tv);
    tv->setTextColor(cls);
    tv->setEnabled(argc%2);
    LOGD("cls=%p",cls);*/
    return app.exec();
}

