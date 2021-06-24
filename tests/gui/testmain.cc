#include <gtest/gtest.h>
#include <windows.h>
#include <signal.h>
#include <execinfo.h>
#include <cdlog.h>

class GUIEnvironment: public testing::Environment{
  public:
    void SetUp(){
       printf("GUIEnvironment Setup\r\n");
    }
    void TearDown(){
       printf("GUIEnvironment TearDown\r\n");
    }
};

int main(int argc,char*argv[])
{
    LogParseModules(argc,(const char**)argv);
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
