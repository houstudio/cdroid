#include <cdroid.h>
#include <cdlog.h>
#include <core/inputeventsource.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    Window*ws=nullptr;

    w->setId(10000);
    w->setBackgroundColor(0xFF111111);
    EditText*edit=new EditText("ScreenSaver test",680,200);
    edit->setSingleLine(false);
    edit->setClickable(true);
    edit->setInputType(EditText::TYPE_ANY);
    edit->setGravity(Gravity::LEFT|Gravity::CENTER_VERTICAL);
    edit->setBackgroundColor(0xFFFF0000);
    w->addView(edit).setPos(100,300);
    edit->setTextSize(60);
    InputEventSource::getInstance().setScreenSaver([&ws](bool bEnabled){
       if(bEnabled&&ws==nullptr){
           ws=new Window(200,200,200,80);
	   LOGD("Create ScreenSaver %d",bEnabled);
	   Button*btn=new Button("Close",0,0);
	   ws->addView(btn);
	   btn->setOnClickListener([&ws](View&v){
		ws->close();
		InputEventSource::getInstance().closeScreenSaver();
		LOGD("close ScreenSaver");
		ws=nullptr;
	   });
       }
       const bool screenSaverActived= InputEventSource::getInstance().isScreenSaverActived();
       LOGD("screenSaver returned %p actived=%d bEnabled=%d",ws,screenSaverActived,bEnabled);
    },5000);
    return app.exec();
}
