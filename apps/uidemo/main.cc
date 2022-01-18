#include <cdroid.h>
#include <string>
#include <cdlog.h>
#include <iostream>
extern Window*CreateMultiMedia(); 

int main(int argc,const char*argv[]){
    App app(argc,argv);
    setenv("LANG","zh_CN",1);
    LOGD("%s",__FILE__);
    CreateMultiMedia();
    LOGD("sizeof(View)=%d std::vector=%d",sizeof(View),sizeof(std::vector<int>));
    app.setOpacity(app.getArgAsInt("alpha",255));
    app.getString(TEXT("Main Menu"),app.getArg("language","eng"));
    return app.exec();
}

