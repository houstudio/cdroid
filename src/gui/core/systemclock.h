#ifndef __SYSTEM_CLOCK_H__
#define __SYSTEM_CLOCK_H__
#include <cdtypes.h>

namespace cdroid{

class SystemClock{
public:
    constexpr static long NANOS_PER_MS = 1000000;
public:
    /*Returns milliseconds since boot, not counting time spent in deep sleep*/
    static int64_t uptimeMillis();
    static int64_t uptimeMicros();
    static int64_t uptimeNanos();
    /*Returns milliseconds since January 1, 1970 00:00:00.0 UTC*/
    static int64_t currentTimeMillis();
    static int64_t currentTimeSeconds();
    static bool setCurrentTimeMillis(int64_t millis);
    static int64_t elapsedRealtime();
};

}
#endif
