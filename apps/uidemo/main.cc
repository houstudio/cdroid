#include <windows.h>
#include <string>
#include <cdlog.h>
#include <iostream>
extern Window*CreateMultiMedia(); 
static bool onKey(int key){
     switch(key){
     case KEY_F12:
     case KEY_MENU: CreateMultiMedia();return true;
     case KEY_ENTER:return true;
     case KEY_UP:
     case KEY_DOWN:return true;
     case KEY_EPG:return true;
     case KEY_F5:
             return true;
     case KEY_AUDIO:
          return true;
     case KEY_POWER:App::getInstance().exit(0);return true;
     case KEY_VOLUMEUP:
     case KEY_VOLUMEDOWN:
     case KEY_LEFT:
     case KEY_RIGHT:
          break;
     case KEY_SUBTITLE:
          break;
     case KEY_RED  :return true;
     case KEY_GREEN:return true;
     case KEY_YELLOW:return true;
     default:return false;
     }
     return false;
}

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

