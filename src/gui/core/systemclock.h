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
#ifndef __SYSTEM_CLOCK_H__
#define __SYSTEM_CLOCK_H__
#include <cstdint>
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
