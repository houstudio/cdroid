#include <cdroid.h>
#include <string>
#include <cdlog.h>
#include <iostream>
#include <homewindow.h>
int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new HomeWindow();
    return app.exec();
}

