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
#ifndef __CALENDAR_UTILS_H__
#define __CALENDAR_UTILS_H__
#include <cstdint>

namespace cdroid{

// Port of sun.util.calendar.CalendarUtils. Pure-integer floor-division
// primitives used by the calendar field math, replacing the previous libc
// gmtime_r/timegm based implementation. All wide overloads use int64_t
// because Android `long` is 64-bit and CDROID targets 32-bit ARM where
// `long` is 32-bit.
class CalendarUtils {
public:
    static bool isGregorianLeapYear(int gregorianYear) {
        return ((gregorianYear % 4) == 0)
            && (((gregorianYear % 100) != 0) || ((gregorianYear % 400) == 0));
    }

    // Floor of the quotient. floorDivide(-1,4) == -1 (whereas -1/4 == 0).
    static int64_t floorDivide(int64_t n, int64_t d) {
        return (n >= 0) ? (n / d) : (((n + 1) / d) - 1);
    }

    static int floorDivide(int n, int d) {
        return (n >= 0) ? (n / d) : (((n + 1) / d) - 1);
    }

    // Floor quotient with the modulus remainder written to r. Matches Java's
    // floorDivide(int,int,int[]): floorDivide(-1,4,r) sets r=3 and returns -1.
    static int floorDivide(int n, int d, int& r) {
        if (n >= 0) {
            r = n % d;
            return n / d;
        }
        int q = ((n + 1) / d) - 1;
        r = n - (q * d);
        return q;
    }

    static int floorDivide(int64_t n, int d, int& r) {
        if (n >= 0) {
            r = static_cast<int>(n % d);
            return static_cast<int>(n / d);
        }
        int q = static_cast<int>(((n + 1) / d) - 1);
        r = static_cast<int>(n - (static_cast<int64_t>(q) * d));
        return q;
    }

    static int64_t mod(int64_t x, int64_t y) {
        return x - y * floorDivide(x, y);
    }

    static int mod(int x, int y) {
        return x - y * floorDivide(x, y);
    }

    // mod that returns y instead of 0.
    static int amod(int x, int y) {
        int z = mod(x, y);
        return (z == 0) ? y : z;
    }

    static int64_t amod(int64_t x, int64_t y) {
        int64_t z = mod(x, y);
        return (z == 0) ? y : z;
    }
};

}//namespace
#endif
