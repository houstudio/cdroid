#ifndef __SYSTEM_CLOCK_H__
#define __SYSTEM_CLOCK_H__
#include <cdtypes.h>

namespace cdroid{

class SystemClock{
public:
    /*Returns milliseconds since boot, not counting time spent in deep sleep*/
    static LONGLONG uptimeMillis();
    static LONGLONG uptimeMicros();
    static LONGLONG uptimeNanos();
    /*Returns milliseconds since January 1, 1970 00:00:00.0 UTC*/
    static LONGLONG currentTimeMillis();
    static LONGLONG currentTimeSeconds();
    static LONGLONG elapsedRealtime();
};

}
#endif
