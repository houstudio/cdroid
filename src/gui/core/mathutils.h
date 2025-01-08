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

};
}
#endif

