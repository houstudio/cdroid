#include <gtest/gtest.h>
#include <cdroid.h>

using namespace cdroid;

class ATTRIBUTESET:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(ATTRIBUTESET,normalize){
   AttributeSet atts(nullptr,"cdroid");
   const char*kvs[]={
       "color1","?textColor",
       "color2","?attr/textColor",
       "color3","?cdroid:attr/textColor",
       nullptr
   };
   ASSERT_EQ(atts.normalize("cdroid","?textColor"),"cdroid:attr/textColor");
   ASSERT_EQ(atts.normalize("cdroid","?attr/textColor"),"cdroid:attr/textColor");
   ASSERT_EQ(atts.normalize("cdroid","?cdroid:attr/textColor"),"cdroid:attr/textColor");

}


