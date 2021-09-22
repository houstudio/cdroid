#include<windows.h>
#include<cdlog.h>

class TestView:public View{
public:
    TestView(int w,int h):View(w,h){
    }
    void onDraw(Canvas&canvas)override{
        canvas.set_color(SystemClock::uptimeMillis());
        canvas.rectangle(0,0,getWidth(),getHeight());
        canvas.fill_preserve();
        canvas.set_color(0xFFFFFFFF);
        canvas.arc(getWidth()/2,getHeight()/2,getWidth()/2,.0f,M_PI/3.f);
        canvas.stroke();
       

        canvas.set_color(0,0,1);
#if 0
        Layout ll(48,mWidth);
        ll.setText("Test");
        ll.draw(canvas); 
#else 
        canvas.set_font_size(48);
        canvas.move_to(0,50);
        canvas.show_text("Test");        
#endif
    }
};
int main(int argc,const char*argv[]){

    App app(argc,argv);
    Window*w=new Window(100,100,800,600);
    ViewGroup*grp=new ViewGroup(400,400);
    grp->setId(10);//.setRotation(30);
    w->addView(grp).setPos(100,100).setBackgroundColor(0xFFFF0000);
    w->setBackgroundColor(0xFF111111);
    TestView*tv=new TestView(100,100);
    tv->setId(100);
    grp->addView(tv).setPos(50,200);
    float rotation=.0f;
    Runnable r([&](){
        //grp->setRotation(rotation); 
        tv->setRotation(rotation);
        rotation+=5;
        w->postDelayed(r,100);
    });
    w->postDelayed(r,1000);
    return app.exec();
}


