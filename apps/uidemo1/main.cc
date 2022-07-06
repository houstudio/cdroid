#include <cdroid.h>
#include <string>
#include <cdlog.h>
#include <iostream>
extern Window*CreateMultiMedia(); 

int main(int argc,const char*argv[]){
    App app(argc,argv);
    CreateMultiMedia();
    app.setOpacity(app.getArgAsInt("alpha",255));
    app.getString("Main Menu",app.getArg("language","eng"));
    return app.exec();
}

