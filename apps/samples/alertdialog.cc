#include <cdroid.h>
#include <cdlog.h>
#include <app/alertdialog.h>

int main(int argc,const char*argv[]){
    setenv("LANG","zh.CN",1);
    App app(argc,argv);
    auto f=[](DialogInterface& dlg,int which){
        LOGD("click button %d",which);
    };
    AlertDialog*dlg=AlertDialog::Builder(&app)
       .setPositiveButton("OK",f)
       .setNegativeButton("Cancel",f)
       .setTitle("This is Title...")
       .setIcon("@cdroid:mipmap/ic_dialog_info")
       .setMessage("Hello world!!!")
       .show();
    return app.exec();
}
