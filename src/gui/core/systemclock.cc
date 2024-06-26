#include <systemclock.h>
#include <chrono>
#include <sys/time.h>

namespace cdroid{

/*Returns milliseconds since boot, not counting time spent in deep sleep*/
LONGLONG SystemClock::uptimeMillis(){
    std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch(); 
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return millis;
}
LONGLONG SystemClock::uptimeMicros(){
    std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch(); 
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    return micros;
}
LONGLONG SystemClock::uptimeNanos(){
    std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch(); 
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    return nanos;
}
/*Returns milliseconds since January 1, 1970 00:00:00.0 UTC*/
LONGLONG SystemClock::currentTimeMillis(){
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch(); 
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return millis;
}

bool SystemClock::setCurrentTimeMillis(LONGLONG millis){
    struct timeval tv;
    tv.tv_sec = millis/1000;
    tv.tv_usec= (millis%1000)*1000;
    return settimeofday(&tv,nullptr)==0;
}

LONGLONG SystemClock::currentTimeSeconds(){
    return currentTimeMillis()/1000;
}

LONGLONG SystemClock::elapsedRealtime(){
    return currentTimeMillis();
}
}
