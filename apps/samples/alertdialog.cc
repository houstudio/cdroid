#include <cdroid.h>
#include <cdlog.h>
#include <app/alertdialog.h>

int main(int argc,const char*argv[]){
    setenv("LANG","zh.CN",1);
    App app(argc,argv);
    auto f=[](DialogInterface& dlg,int which){
        LOGD("click button %d",which);
    };
    AlertDialog*dlg;
    std::vector<std::string>list;
    for(int i=0;i<10;i++)list.push_back(std::string("item")+std::to_string(i));
    switch(argc){
    case 1:
         dlg=AlertDialog::Builder(&app)
           .setPositiveButton("OK",f)
           .setNegativeButton("Cancel",f)
           .setTitle("This is Title...")
           .setIcon("@cdroid:mipmap/ic_dialog_info")
           .setMessage("Hello world,this is alert dialog test message!!!")
           .show();
         break;
    case 2:
        dlg=AlertDialog::Builder(&app)
           .setPositiveButton("OK",f)
           .setNegativeButton("Cancel",f)
           .setTitle("This is Title...")
           .setItems(list,nullptr)//[](DialogInterface& dlg,int whitch){ })
           .show();
        break; 
    }
    return app.exec();
}
