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
   ASSERT_EQ(AttributeSet::normalize("cdroid","?textColor"),"cdroid:attr/textColor");
   ASSERT_EQ(AttributeSet::normalize("cdroid","?attr/textColor"),"cdroid:attr/textColor");
   ASSERT_EQ(AttributeSet::normalize("cdroid","?cdroid:attr/textColor"),"cdroid:attr/textColor");
}

TEST_F(ATTRIBUTESET,inherit1){
   AttributeSet att1(nullptr,"cdroid");
   AttributeSet att2(nullptr,"cdroid");
   const char*kvs[]={
       "color1","?textColor",
       "color2","?attr/textColor",
       "color3","?cdroid:attr/textColor",
       nullptr
   };
   att1.set(kvs);
   att2.inherit(att1);
   ASSERT_EQ(att2.getString("color1"),"cdroid:attr/textColor");
   ASSERT_EQ(att2.getString("color2"),"cdroid:attr/textColor");
   ASSERT_EQ(att2.getString("color3"),"cdroid:attr/textColor");
}
TEST_F(ATTRIBUTESET,inherit2){
   AttributeSet att1(nullptr,"cdroid");
   AttributeSet att2(nullptr,"app");
   const char*kvs[]={
       "color1","?textColor",
       "color2","?attr/textColor",
       "color3","?cdroid:attr/textColor",
       nullptr
   };
   att1.set(kvs);
   att2.inherit(att1);
   ASSERT_EQ(att2.getString("color1"),"cdroid:attr/textColor");
   ASSERT_EQ(att2.getString("color2"),"cdroid:attr/textColor");
   ASSERT_EQ(att2.getString("color3"),"cdroid:attr/textColor");
}
