#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>
#include <core/app.h>
#include <app/alertdialog.h>
#include <guienvironment.h>

using namespace cdroid;

class DIALOG:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

/* Dialogs auto-finish: show() returns the dialog, we assert it built, pump a
   few frames so it measures/draws, then move on. The button click listeners
   are kept (harmless in automation; a human run can still click them). */
TEST_F(DIALOG,1Button){
   App&app=App::getInstance();
   AlertDialog*dlg=AlertDialog::Builder(&app)
	 .setTitle("DialogTest")
         .setMessage("Hello ,every one! This is an multiline messagebox Example\n this is the second line\nis this OK?")
         .setPositiveButton("OK",[](DialogInterface&,int){App::getInstance().exit(0);})
         .show();
   ASSERT_NE(dlg,nullptr);
   pumpFor(300);
   dlg->dismiss();
}

TEST_F(DIALOG,2Button){
   App&app=App::getInstance();
   AlertDialog*dlg=AlertDialog::Builder(&app)
         .setTitle("DialogTest")
         .setMessage("Hello ,every one! This is an multiline messagebox Example\n this is the second line\nis this OK?")
         .setPositiveButton("Yes",nullptr)
         .setNegativeButton("No",[](DialogInterface&,int){App::getInstance().exit(1);})
         .show();
   ASSERT_NE(dlg,nullptr);
   pumpFor(300);
   dlg->dismiss();
}

TEST_F(DIALOG,3Button){
   App&app=App::getInstance();
   AlertDialog*dlg=AlertDialog::Builder(&app)
         .setTitle("DialogTest")
         .setMessage("Hello ,every one! This is an multiline messagebox Example\n this is the second line\nis this OK?")
         .setPositiveButton("Yes",nullptr)
         .setNegativeButton("No",nullptr)
         .setNeutralButton("Cancel",[](DialogInterface&,int){App::getInstance().exit(2);})
         .show();
   ASSERT_NE(dlg,nullptr);
   pumpFor(300);
   dlg->dismiss();
}

TEST_F(DIALOG,Choices){
   App&app=App::getInstance();
   std::vector<std::string>items;
   for(int i=0;i<10;i++){
       std::ostringstream oss;
       oss<<"item-"<<i;
       items.push_back(oss.str());
   }
   AlertDialog*dlg=AlertDialog::Builder(&app)
         .setTitle("DialogTest")
         .setSingleChoiceItems(items,0,nullptr)
         .setPositiveButton("Yes",nullptr)
         .setNegativeButton("No",[](DialogInterface&,int){App::getInstance().exit(1);})
         .show();
   ASSERT_NE(dlg,nullptr);
   pumpFor(300);
   dlg->dismiss();
}
