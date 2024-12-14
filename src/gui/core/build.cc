#include <core/build.h>
#include <gui_features.h>
namespace cdroid{

const std::string Build::VERSION::Release = CDROID_VERSION;
const std::string Build::VERSION::CommitID=CDROID_COMMITID;
const int Build::VERSION::Major = CDROID_VERSION_MAJOR;
const int Build::VERSION::Minor = CDROID_VERSION_MINOR;
const int Build::VERSION::Patch = CDROID_VERSION_PATCH;
const int Build::VERSION::BuildNumber=CDROID_BUILD_NUMBER;

const int Build::VERSION::SDK_INT=Build::VERSION_CODES::P;
}
