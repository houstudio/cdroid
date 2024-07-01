#include <core/build.h>
#include <gui_features.h>
namespace cdroid{

const std::string BUILD::VERSION::Release = CDROID_VERSION;
const std::string BUILD::VERSION::CommitID=CDROID_COMMITID;
const int BUILD::VERSION::Major = CDROID_VERSION_MAJOR;
const int BUILD::VERSION::Minor = CDROID_VERSION_MINOR;
const int BUILD::VERSION::Patch = CDROID_VERSION_PATCH;
const int BUILD::VERSION::BuildNumber=CDROID_BUILD_NUMBER;

}
