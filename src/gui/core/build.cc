#include <core/build.h>
#include <gui_features.h>
namespace cdroid{

const std::string BUILD::VERSION::Release = CDROID_VERSION;
const std::string BUILD::VERSION::CommitID;
const int BUILD::VERSION::Major = 1;
const int BUILD::VERSION::Minor = 0;
const int BUILD::VERSION::Patch = 0;
const int BUILD::VERSION::BuildNumber=1566;

static void test(){
    //BUILD::VERSION::RELEASE="===";
}

}
