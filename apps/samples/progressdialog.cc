#include <cdroid.h>
#include <cdlog.h>
#include <app/progressdialog.h>

int main(int argc,const char*argv[]){
    setenv("LANG","zh.CN",1);
    App app(argc,argv);
    auto f=[](DialogInterface& dlg,int which){
        LOGD("click button %d",which);
    };
    ProgressDialog*dlg=ProgressDialog::show(&app,"Progress","download progress");
    dlg->setProgress(16);
    dlg->setButton(DialogInterface::BUTTON_POSITIVE,"OK",f);
    return app.exec();
}
