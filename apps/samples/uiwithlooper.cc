#include <cdroid.h>
#include <cdlog.h>
#include <widget/candidateview.h>
#include <thread>

int main(int argc,const char*argv[]){
    setenv("LANG","zh_CN.UTF-8",1);
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    w->setId(10000);
    w->setBackgroundColor(0xFF111111);
    EditText*edit=new EditText("Hello world! This value is positive for typical fonts that include",680,200);
    edit->setTextColor(0xFFFFFFFF);
    edit->setSingleLine(false);
    edit->setClickable(true);
    edit->setInputType(EditText::TYPE_ANY);
    edit->setGravity(Gravity::LEFT|Gravity::CENTER_VERTICAL);
    edit->setBackgroundColor(0xFFFF0000);
    w->addView(edit).layout(100,300,680,200);
    edit->setTextSize(60);

    std::thread th([](){
	Looper*looper=new Looper(0);
	Looper::setForThread(looper);
	while(1)looper->pollOnce(10);
    });
    th.detach();
    return app.exec();
}
