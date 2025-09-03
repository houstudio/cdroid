#include <guienvironment.h>
#include <cdroid.h>
#include <signal.h>
#include <cdlog.h>

GUIEnvironment*GUIEnvironment::mInst=nullptr;
int main(int argc,char*argv[])
{
    LogParseModules(argc,(const char**)argv);
    testing::InitGoogleTest(&argc,argv);
    ::testing::AddGlobalTestEnvironment(new GUIEnvironment(argc, (const char**)argv));
    return RUN_ALL_TESTS();
}
