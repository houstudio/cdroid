#include <cdroid.h>
#include <stddef.h>
#include <wpasockclient.h>
char reply[1024];
size_t replylen;
int main(int argc,const char*argv[]){
   App app(argc,argv);
   Window*w=new Window(0,0,-1,-1);
   w->setBackgroundColor(0xFFFF0000);
   Button*btn=new Button("scan",200,200);
   w->addView(btn);
   btn->setBackgroundColor(0xFF223344);
   cdroid::WpaClientSocket wpa;
   wpa.bind("/tmp/wifi/run/wpa_supplicant/wlan0");
   wpa.start();
   wpa.write("SCAN",reply,&replylen);
   btn->setOnClickListener([&](View&v){
       printf("getting scan_result\r\n");
       wpa.write("SCAN_RESULTS",reply,&replylen);
   });
   int color=0;
   Runnable run={[&](){
       btn->setBackgroundColor(0xFF000000|color);
       color+=8;
       w->postDelayed(run,100);
   }};
   w->postDelayed(run,100);
   app.exec();
   return 0;
}
