#include <cdroid.h>
#include <cdlog.h>
#include <getopt.h>
#include <app/alertdialog.h>

std::vector<CLA::Argument> app_options={
    {CLA::EntryType::Switch, "", "dialog"  ,"dialog type",CLA::ValueType::None,   (int)CLA::EntryFlags::Optional},
    {CLA::EntryType::Switch, "", "items"  , "item count",CLA::ValueType::None,   (int)CLA::EntryFlags::Optional}
};

int main(int argc,const char*argv[]){
    setenv("LANG","zh.CN",1);
    App app(argc,argv,app_options);
    auto f=[](DialogInterface& dlg,int which){
        LOGD("click button %d",which);
    };
    AlertDialog*dlg;
    std::vector<std::string>list;
    const int itemCount=app.getArgAsInt("items",10);
    for(int i=0;i<itemCount;i++)
        list.push_back(std::string("item")+std::to_string(i));
    switch(app.getArgAsInt("dialog",0)){
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
