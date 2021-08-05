#include <stdio.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <vector>
#include <list>
#include <ngl_os.h>
#include <core/preferences.h>

using namespace cdroid;

class PREFERENCES:public testing::Test{
public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(PREFERENCES,setbool){
   Preferences pref;
   pref.setValue("Video","width",false);
   pref.setValue("Video","width",true);
   pref.setValue("Video","height",true);
   ASSERT_EQ(true,pref.getBool("Video","width",false));
   pref.save("tesbool.pref");
}

TEST_F(PREFERENCES,setint){
   Preferences pref;
   pref.setValue("Video","width",800);
   pref.setValue("Video","width",1800);
   pref.setValue("Video","height",1800);
   ASSERT_EQ(1800,pref.getInt("Video","width",0));
   pref.save("testint.pref");
}

TEST_F(PREFERENCES,setfloat){
   Preferences pref;
   pref.setValue("Video","width",800.12);
   pref.setValue("Video","width",1800.123);
   pref.setValue("Video","height",1800.234);
   ASSERT_FLOAT_EQ(1800.123,pref.getFloat("Video","width",0.f));
   pref.save("testfloat.pref");
}

TEST_F(PREFERENCES,setdouble){
   Preferences pref;
   pref.setValue("Video","width",(double)800.12);
   pref.setValue("Video","width",(double)1800.123);
   pref.setValue("Video","height",(double)1800.234);
   ASSERT_DOUBLE_EQ(1800.123,pref.getDouble("Video","width",0.f));
   pref.save("testdouble.pref");
}
TEST_F(PREFERENCES,setstring){
   Preferences pref;
   pref.setValue("Video","url","url1");
   pref.setValue("Video","url","url2");
   EXPECT_STREQ("url2",pref.getString("Video","url",""));
   pref.save("teststr.pref");
}
TEST_F(PREFERENCES,strings){
   Preferences pref,pld;
   std::string server("videoserver");
   std::string ip("ip");
   std::string port("port");
   pref.setValue(server,port,1234);
   pref.setValue(server,ip,"192.168.1.150");
   
   pref.setValue("server2",port,1234);
   pref.setValue("server2",ip,"192.168.1.150");
   pref.save("server.pref");
 
   pld.load("server.pref");
   ASSERT_EQ(1234,pref.getInt(server,port,0));
   EXPECT_STREQ("192.168.1.150",pref.getString(server,ip,""));
   ASSERT_EQ(1234,pref.getInt("server2",port,0));
   EXPECT_STREQ("192.168.1.150",pref.getString("server2",ip,""));
}

