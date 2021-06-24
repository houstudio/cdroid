#include <gtest/gtest.h>
#include <windows.h>
#include <ngl_os.h>
//#include <dialog.h>

using namespace cdroid;
#if 0
class DIALOG:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(DIALOG,1Button){
   App app;
   Dialog dialog;
   dialog.setTitle("DialogTest")
         .setMessage("Hello ,every one! This is an multiline messagebox Example\n this is the second line\nis this OK?")
         .setPositiveButton("OK",[](View&){App::getInstance().exit(0);})
         .show();
   ASSERT_EQ(0,app.exec());
}
TEST_F(DIALOG,2Button){
   App app;
   Dialog dialog;
   dialog.setTitle("DialogTest")
         .setMessage("Hello ,every one! This is an multiline messagebox Example\n this is the second line\nis this OK?")
         .setPositiveButton("Yes",nullptr)
         .setNegativeButton("No",[](View&){App::getInstance().exit(1);})
         .show();
   ASSERT_EQ(1,app.exec());
}
TEST_F(DIALOG,3Button){
   App app;
   Dialog dialog;
   dialog.setTitle("DialogTest")
         .setMessage("Hello ,every one! This is an multiline messagebox Example\n this is the second line\nis this OK?")
         .setPositiveButton("Yes",nullptr)
         .setNegativeButton("No",nullptr)
         .setNeutralButton("Cancel",[](View&){App::getInstance().exit(2);})
         .show();
   ASSERT_EQ(2,app.exec());
}
TEST_F(DIALOG,Choices){
   App app;
   Dialog dialog;
   std::vector<std::string>items;
   for(int i=0;i<10;i++){
       std::ostringstream oss;
       oss<<"item-"<<i;
       items.push_back(oss.str());
   }
   dialog.setTitle("DialogTest")
         .setSingleChoiceItems(items,0,nullptr)
         .setPositiveButton("Yes",nullptr)
         .setNegativeButton("No",[](View&){App::getInstance().exit(1);})
         .show();
   app.exec();
}
#endif

