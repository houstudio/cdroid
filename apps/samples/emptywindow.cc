#include <cdroid.h>
#include <cdlog.h>
#include <widget/candidateview.h>

class MyWindow:public Window{
public:
    MyWindow(int x,int y,int w,int h):Window(x,y,w,h){};
    void onDraw(Canvas&canvas)override{
        LOGD("%p onDraw",this);
        Rect rc={0,0,getWidth(),getHeight()};
        canvas.set_source_rgb(1,0,0);
        for(int i=0;i<10;i++){
            canvas.rectangle(rc.left,rc.top,rc.width,rc.height);
            canvas.stroke();
            rc.inflate(-10,-10);
        }
    }
};
int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new MyWindow(0,0,-1,-1);
    return app.exec();
}
