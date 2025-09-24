/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <chrono>
#include <core/systemclock.h>
#if defined(__linux__)||defined(__unix__)
  #include <sys/time.h>
#endif
namespace cdroid{

/*Returns milliseconds since boot, not counting time spent in deep sleep*/
int64_t SystemClock::uptimeMillis(){
    std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch(); 
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return millis;
}

int64_t SystemClock::uptimeMicros(){
    std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch(); 
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    return micros;
}

int64_t SystemClock::uptimeNanos(){
    std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch(); 
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    return nanos;
}

/*Returns milliseconds since January 1, 1970 00:00:00.0 UTC*/
int64_t SystemClock::currentTimeMillis(){
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch(); 
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return millis;
}

bool SystemClock::setCurrentTimeMillis(int64_t millis){
#if defined(__linux__)||defined(__unix__)
    struct timeval tv;
    tv.tv_sec = millis/1000;
    tv.tv_usec= (millis%1000)*1000;
    return settimeofday(&tv,nullptr)==0;
#elif defined(_WIN32)||defined(_WIN64)
    return false;
#endif
}

int64_t SystemClock::currentTimeSeconds(){
    return currentTimeMillis()/1000;
}

int64_t SystemClock::elapsedRealtime(){
    return uptimeMillis();
}

}
