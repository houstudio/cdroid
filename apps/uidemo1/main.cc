#include <core/app.h>
#include <widget/cdwindow.h>
#include <string>
#include <iostream>
#include <R.h>
#include <porting/cdlog.h>
extern Window*CreateMultiMedia(); 

int main(int argc,const char*argv[]){
    App app(argc,argv);
    auto aset=app.getResources().getLayout(uidemo1::R::layout::layout1);
    printf("\r\nasset[%x]=%p\r\n",uidemo1::R::layout::layout1,aset);
    CreateMultiMedia();
    app.setOpacity(app.getArgAsInt("alpha",255));
    app.getString("Main Menu",app.getArg("language","eng"));
    return app.exec();
}

