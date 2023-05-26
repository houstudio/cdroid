#include <cdroid.h>
#include <string>
#include <cdlog.h>
#include <iostream>
#include <controlcenter.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new ControlCenter(0,0,-1,-1);
    return app.exec();
}

