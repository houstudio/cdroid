#include<windows.h>
#include<cdlog.h>

class TestView:public View{
public:
    TestView(int w,int h):View(w,h){
    }
    void onDraw(Canvas&canvas)override{
        canvas.set_source_rgba(0,1,0,.5);
        canvas.rectangle(0,0,mWidth,mHeight);
        canvas.fill();

        canvas.set_source_rgb(0,0,1);
#if 0
        Layout ll(22,mWidth);
        ll.setText("Test");
        ll.draw(canvas); 
#else 
        canvas.set_font_size(32);
        canvas.move_to(40,40);
        canvas.show_text("Test");        
#endif
//     canvas.dump2png("123.png");
    }
};
int main(int argc,const char*argv[]){

    App app(argc,argv);
    Window*w=new Window(100,100,800,600);
    ViewGroup*grp=new ViewGroup(400,400);
    grp->setRotation(30);
    w->addView(grp).setPos(100,100).setBackgroundColor(0xFFFF0000);
    w->setBackgroundColor(0xFF111111);
    //TextView*tv=new TextView("HelloWorld",200,40);
    //grp->addView(tv).setPos(50,50).setBackgroundColor(0xFF00FF00);
    grp->addView(new TestView(100,100)).setPos(50,200);
    float rotation=.0f;
    Runnable r([&](){
        grp->setRotation(rotation); grp->invalidate();
        //tv->setRotation(rotation); tv->invalidate();
        rotation+=5;
        w->postDelayed(r,80);
    });
    w->postDelayed(r,1000);
    return app.exec();
}


