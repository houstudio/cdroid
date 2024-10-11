#ifndef __CDROID_BUILD_H__
#define __CDROID_BUILD_H__
#include <string>
namespace cdroid{

class BUILD{
public:
    class VERSION{
    public:
        static const std::string Release;
        static const std::string CommitID;
        static const int Major;
        static const int Minor;
        static const int Patch;
        static const int BuildNumber;
    };
};
}
#endif/*__BUILD_H__*/
