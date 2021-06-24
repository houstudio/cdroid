#include <systemclock.h>
#include <chrono>

namespace cdroid{

/*Returns milliseconds since boot, not counting time spent in deep sleep*/
LONGLONG SystemClock::uptimeMillis(){
    std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch(); 
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return millis;
}

/*Returns milliseconds since January 1, 1970 00:00:00.0 UTC*/
LONGLONG SystemClock::currentTimeMillis(){
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch(); 
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return millis;
}

LONGLONG SystemClock::currentTimeSeconds(){
    return currentTimeMillis()/1000;
}

LONGLONG SystemClock::elapsedRealtime(){
    return currentTimeMillis();
}
}
