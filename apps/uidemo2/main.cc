#include <cdroid.h>
#include <string>
#include <cdlog.h>
#include <iostream>
#include <controlcenter.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    unsigned int width,height;
    GFXGetDisplaySize(0,&width,&height);
    printf("screensize=%dx%d\r\n",width,height); 
    if((GFXGetRotation(0)==ROTATE_90)||(GFXGetRotation(0)==ROTATE_270)){
        int tmp = width;
        width = height;
        height=tmp;
    }
    printf("size=%dx%d rotate=%d/%d/%d\r\n",width,height,GFXGetRotation(0),ROTATE_90,ROTATE_270);
    Window*w=new ControlCenter(0,0,width,height);
    return app.exec();
}

