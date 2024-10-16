#include <gtest/gtest.h>
#include <porting/cdtypes.h>
#include <porting/cdlog.h>
#include <porting/cdgraph.h>
#include <math.h>

class HALEnvironment: public testing::Environment{
  public:
    void SetUp(){
        GFXInit();
    };
    void TearDown(){
        //sleep(5);
    }
};

int main(int argc, char*argv[])
{
    LogParseModules(argc,(const char**)argv);
    testing::AddGlobalTestEnvironment(new HALEnvironment);
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
