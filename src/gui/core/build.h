#ifndef __CDROID_BUILD_H__
#define __CDROID_BUILD_H__
#include <string>
namespace cdroid{

class Build{
public:
    class VERSION_CODES;
    class VERSION{
    public:
        static const std::string Release;
        static const std::string CommitID;
        static const int Major;
        static const int Minor;
        static const int Patch;
        static const int BuildNumber;
        static const int SDK_INT;
    };
};
using BUILD=Build;

class Build::VERSION_CODES {
public:
    /**
     * Magic version number for a current development build, which has
     * not yet turned into an official release.
     */
    // This must match VMRuntime.SDK_VERSION_CUR_DEVELOPMENT.
    static constexpr int CUR_DEVELOPMENT = 10000;
    static constexpr int BASE = 1;
    static constexpr int BASE_1_1 = 2;
    static constexpr int CUPCAKE = 3;
    static constexpr int DONUT = 4;
    static constexpr int ECLAIR = 5;
    static constexpr int ECLAIR_0_1 = 6;
    static constexpr int ECLAIR_MR1 = 7;
    static constexpr int FROYO = 8;
    static constexpr int GINGERBREAD = 9;
    static constexpr int GINGERBREAD_MR1 = 10;
    static constexpr int HONEYCOMB = 11;
    static constexpr int HONEYCOMB_MR2 = 13;
    static constexpr int ICE_CREAM_SANDWICH = 14;
    static constexpr int ICE_CREAM_SANDWICH_MR1 = 15;
    static constexpr int JELLY_BEAN = 16;
    static constexpr int JELLY_BEAN_MR1 = 17;
    static constexpr int JELLY_BEAN_MR2 = 18;
    static constexpr int KITKAT = 19;
    static constexpr int KITKAT_WATCH = 20;
    static constexpr int L = 21;
    static constexpr int LOLLIPOP = 21;
    static constexpr int LOLLIPOP_MR1 = 22;
    static constexpr int M = 23;
    static constexpr int N = 24;
    static constexpr int N_MR1 = 25;
    static constexpr int O = 26;
    static constexpr int O_MR1 = 27;
    static constexpr int P = 28;
    static constexpr int Q = 29;
    static constexpr int R = 30;
    static constexpr int S = 31;
    static constexpr int S_V2 = 32;
    static constexpr int TIRAMISU = 33;
    static constexpr int UPSIDE_DOWN_CAKE = 34;
};
}
#endif/*__BUILD_H__*/
