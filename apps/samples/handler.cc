#include <cdroid.h>
#include <unistd.h>
#include <thread>
class MyHandler:public Handler{
public:
    MyHandler():Handler(){}
    void handleMessage(Message& msg)override{
        LOGD("handle message %d",msg.what);
    }
};
int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w= new Window(0,0,-1,-1);
    TextView*tv=new TextView("Hello world!",600,40);
    ProgressBar*pb = new ProgressBar(600,40);
    Handler*handler= new MyHandler();
    tv->setTextColor(0xFFFFFFFF);
    tv->setBackgroundColor(0xFF334455);
    tv->setTextSize(32);
    LinearLayout*ll=new LinearLayout(-1,-1);
    ll->setOrientation(LinearLayout::VERTICAL);
    ll->addView(tv);
    ll->addView(pb);
    w->addView(ll);

    std::thread th([&]{
        Runnable r;
        r=[&](){
           const int progress =(pb->getProgress()+1)%pb->getMax();
           pb->setProgress(progress);
           tv->setText(std::to_string(progress));
           tv->invalidate();
        };
        while(1){
           handler->post(r);
           usleep(10000);
        }
    });

    std::thread th2([&](){
        int what=0;
        while(1){
           handler->sendEmptyMessage(what++);
           usleep(1000*500);
        }
    });
    th.detach();
    th2.detach();
    return app.exec();
}
