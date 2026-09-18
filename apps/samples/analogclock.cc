#include<cdroid.h>
#include<cdlog.h>

/*watch's picture got from https://amazfitwatchfaces.com*/

int main(int argc,const char*argv[]){
    App app(argc,argv);

    // Framework resource by name at runtime (AOSP Resources.getIdentifier) —
    // public and internal names alike, no R header needed. Images live in the
    // drawable namespaces now (the pre-arsc mipmap/ directory convention is gone).
    auto fwid = [&app](const char* name) {
        return app.getResources().getIdentifier(name, "drawable", "android");
    };
    Window*w=new Window(0,0,-1,-1);
    AnalogClock*clk=new AnalogClock(&app);
    clk->setDial(app.getDrawable(fwid("clock_dial")));
    clk->setHourHand(app.getDrawable(fwid("clock_hand_hour")));
    clk->setMinuteHand(app.getDrawable(fwid("clock_hand_minute")));
    // clock_hand_second does not exist in the framework res set (the old string
    // call silently set a null second hand).
    w->addView(clk);
    return app.exec();
}
