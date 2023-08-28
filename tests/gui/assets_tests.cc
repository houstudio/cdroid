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
   printf("size=%d\r\n",array.size());
   ASSERT_TRUE(array.size()>0);
}

TEST_F(ASSETS,array2){
   App app(0,NULL);
   std::vector<std::string>array;
   app.getArray("@cdroid:array/preloaded_drawables",array);
   for(auto a:array)printf("%s\r\n",a.c_str());
   printf("size=%d\r\n",array.size());
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
TEST_F(ASSETS,drawable){
    App app;
    ColorDrawable*cl=(ColorDrawable*)app.getDrawable("@cdroid:color/black");
    ASSERT_TRUE(cl!=NULL);
    LOGD("COLOR=%x",(uint32_t)cl->getColor());
    ASSERT_EQ((uint32_t)cl->getColor(),(uint32_t)0xFF000000);
    cl=(ColorDrawable*)app.getDrawable("@cdroid:color/transparent");
    ASSERT_TRUE(cl!=NULL);
    LOGD("COLOR=%x",(uint32_t)cl->getColor());
    ASSERT_EQ((uint32_t)cl->getColor(),0);
    app.exec();
}
