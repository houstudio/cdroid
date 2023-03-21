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


TEST_F(ASSETS,arrsy){
   App app(0,NULL);
   std::vector<std::string>array;
   app.getArray("cdroid:array/resolver_target_actions_unpin",array);
   for(auto a:array)printf("%s\r\n",a.c_str());
   ASSERT_TRUE(array.size()>0);
}

