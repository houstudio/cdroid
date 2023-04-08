#include <gtest/gtest.h>
#include <cdroid.h>
#include <ngl_os.h>

using namespace cdroid;

class ASSETS:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(ASSETS,string){
   App app(0,NULL);
   std::string str=app.getString("cdroid:string/number_picker_decrement_button");
   printf("str=%s\n",str.c_str());
}
TEST_F(ASSETS,array){
   App app(0,NULL);
   std::vector<std::string>array;
   app.getArray("cdroid:array/resolver_target_actions_unpin",array);
   for(auto a:array)printf("%s\r\n",a.c_str());
   ASSERT_TRUE(array.size()>0);
}
TEST_F(ASSETS,color){
    App app(0,NULL);
    ColorStateList*cl=app.getColorStateList("cdroid:attr/editTextColor");
    ASSERT_TRUE(cl!=NULL);
    cl=app.getColorStateList("cdroid:color/textview");
    ASSERT_TRUE(cl!=NULL);
    cl->dump();
}
