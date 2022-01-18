#include <cdroid.h>
#include <cdlog.h>
#include <getopt.h>

class TestView:public View{
private:
    std::string mText;
public:
    TestView(const std::string&txt,int w,int h):View(w,h){
        mText=txt;
    }
    void onDraw(Canvas&canvas)override{    
        canvas.set_color(SystemClock::uptimeMillis());
        canvas.rectangle(0,0,getWidth(),getHeight());
        canvas.fill();
        canvas.set_color(0xFFFFFFFF);
#if 1
        Layout ll(28,getWidth());
        ll.setText(mText);
        ll.draw(canvas); 
#else
        canvas.set_font_size(28);
        canvas.move_to(0,getHeight()/4);
        canvas.show_text(mText);
#endif
    }
};
class MyGroup:public ViewGroup{
public:
    MyGroup(int w,int h):ViewGroup(w,h){}
    void onDraw(Canvas&canvas)override{
        canvas.set_color(SystemClock::uptimeMillis()+time(nullptr));
        canvas.rectangle(0,0,getWidth(),getHeight());
        canvas.fill();
        canvas.move_to(40,200);
        canvas.set_font_size(68);
        canvas.set_source_rgb(1,1,1);
        canvas.show_text("ViewGroup");
        ViewGroup::onDraw(canvas);
    }
};
static struct option appargs[]={
   {"type"   ,1,0,0},
   {0,0,0,0}
};
int main(int argc,const char*argv[]){

    App app(argc,argv,appargs);
    Window*w=new Window(0,0,800,600);
    ViewGroup*grp=new MyGroup(400,400);
    //grp->setId(10).setRotation(45);
    w->addView(grp).setBackgroundColor(0xFFFF0000);
    w->setBackgroundColor(0xFF111111);
    View*tv=nullptr;
    switch(app.getArgAsInt("type",1)){
    default: //pass throught
    case 0:  tv=new TextView("TestButton",160,60); break;
    case 1:  tv=new TestView("TestButton",160,60); break;
    case 2:  tv=new ImageView(160,160);
             ((ImageView*)tv)->setImageResource("/home/houzh/images/1.png");
             break;
    }
    tv->setId(100);
    grp->addView(tv).setPos(50,50).setBackgroundColor(0xFF444444);
    float rotation=15.f;
    Runnable r([&](){
        grp->setRotation(rotation); 
        tv->setRotation(rotation);
        rotation+=5;
        w->postDelayed(r,300);
    });
    w->postDelayed(r,1000);
    return app.exec();
}


