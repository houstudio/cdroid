#include<cdroid.h>
#include<cdlog.h>

/*watch's picture got from https://amazfitwatchfaces.com*/

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(100,100,320,320);
    AnalogClock*clk=new AnalogClock(320,320);
    clk->setDial(app.getDrawable("cdroid:mipmap/clock_dial"));
    clk->setHourHand(app.getDrawable("cdroid:mipmap/clock_hand_hour"));
    clk->setMinuteHand(app.getDrawable("cdroid:mipmap/clock_hand_minute"));
    clk->setSecondHand(app.getDrawable("cdroid:mipmap/clock_hand_second"));
    w->addView(clk);
    return app.exec();
}
