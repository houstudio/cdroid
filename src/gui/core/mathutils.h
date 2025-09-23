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
#ifndef __MATH_UTILS_H__
#define __MATH_UTILS_H__
#include <math.h>
namespace cdroid{

class MathUtils{
public:
    template<typename T>
    static T constrain(T value, T min, T max) {
        if (value > max) {
            return max;
        } else if (value < min) {
            return min;
        } else {
            return value;
        }
   }

   template<typename T>
   static int numberOfTrailingZeros(T value) {
       if (value == 0) return sizeof(T)*8;
       int count = 0;
       while ((value & 1) == 0) {
           value >>= 1;
           count++;
       }
       return count;
   }

   template <typename T>
   static int signum(T val) {
       return (T(0) < val) - (val < T(0));
   }

   template <typename T>
   static T lerp(T start, T stop, T amount) {
       return start + (stop - start) * amount;
   }

   template<typename T>
   static T clamp(T v, T lo, T hi) {
       return (v < lo) ? lo : (hi < v) ? hi : v;
   }

   static double toRadians(double degrees) {
       return degrees * (M_PI / 180.0);
   }

   static double toDegrees(double radians) {
       return radians * (180.0 / M_PI);
   }
};
}
#endif

