#include <cdroid.h>
#include <cdlog.h>
#include <getopt.h>
#include <core/cxxopts.h>
#include <app/alertdialog.h>

int main(int argc,const char*argv[]){
    setenv("LANG","zh.CN",1);
    App app(argc,argv);

    auto f=[](DialogInterface& dlg,int which){
        LOGD("click button %d",which);
    };
    AlertDialog*dlg;
    cxxopts::Options options("main","application");
    options.add_options()
        ("dialog","dialog type",cxxopts::value<int>()->default_value("0"))
        ("items","item count",cxxopts::value<int>()->default_value("10"));
    options.allow_unrecognised_options();
    auto result = options.parse(argc,argv);
    std::vector<std::string>list;
    const int itemCount = result["items"].as<int>();
    for(int i=0;i<itemCount;i++)
        list.push_back(std::string("item")+std::to_string(i));
    LOGD("dialog=%d items=%d args=%d",result["dialog"].as<int>(),itemCount,app.getParamCount());
    switch(result["dialog"].as<int>()){
    case 0:
         dlg=AlertDialog::Builder(&app)
           .setPositiveButton("OK",f)
           .setNegativeButton("Cancel",f)
           .setTitle("This is Title...")
           .setIcon("@cdroid:mipmap/ic_dialog_info")
           .setMessage("Hello world,\nthis is alert dialog test message\n"
                       "text line can be long enough.\nbecause it is placed in a scrollview.\n"
                       "for long text ,it can scroll...!!!")
           .show();
         break;
    case 1:
        dlg=AlertDialog::Builder(&app)
           .setPositiveButton("OK",f)
           .setNegativeButton("Cancel",f)
           .setTitle("This is Items...")
           .setItems(list,nullptr)//[](DialogInterface& dlg,int whitch){ })
           .show();
        break;
    case 2:
        dlg=AlertDialog::Builder(&app)
           .setPositiveButton("OK",f)
           .setNegativeButton("Cancel",f)
           .setTitle("This is SingleChoiceItems...")
           .setSingleChoiceItems(list,2,nullptr)//[](DialogInterface& dlg,int whitch){ })
           .show();
        break;
    case 3:
        dlg=AlertDialog::Builder(&app)
           .setPositiveButton("OK",f)
           .setNegativeButton("Cancel",f)
           .setTitle("This is MultiCholceItems...")
           .setMultiChoiceItems(list,std::vector<bool>{false,true,false,true},nullptr)//[](DialogInterface& dlg,int whitch){ })
           .show();
    }
    return app.exec();
}
