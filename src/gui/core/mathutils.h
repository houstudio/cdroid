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

};
}
#endif

