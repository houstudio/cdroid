#include <gtest/gtest.h>
#include <cdroid.h>

using namespace cdroid;

class ATTS:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(ATTS,normalize){
    ASSERT_EQ(AttributeSet::normalize("cdroid","red"),"red");
    ASSERT_EQ(AttributeSet::normalize("cdroid","#123456"),"#123456");
    ASSERT_EQ(AttributeSet::normalize("cdroid","@null"),"null");
    ASSERT_EQ(AttributeSet::normalize("cdroid","@mipmap/test"),"cdroid:mipmap/test");
    ASSERT_EQ(AttributeSet::normalize("cdroid","?textColor"),"cdroid:attr/textColor");
    ASSERT_EQ(AttributeSet::normalize("cdroid","?attr/textColor"),"cdroid:attr/textColor");
    ASSERT_EQ(AttributeSet::normalize("cdroid","?cdroid:attr/textColor"),"cdroid:attr/textColor");
    ASSERT_EQ(AttributeSet::normalize("cdroid","@cdroid:color/background_light"),"cdroid:color/background_light");
}

TEST_F(ATTS,inherit1){
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
TEST_F(ATTS,inherit2){
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
