#ifndef __GESTURE_POINT_H__
#define __GESTURE_POINT_H__
#include <gesture/gesturestore.h>
namespace cdroid{
class GesturePoint {
public:
    float x;
    float y;
    long timestamp;
public:
    GesturePoint(float x, float y, long t) {
        this->x = x;
        this->y = y;
        timestamp = t;
    }
    static GesturePoint deserialize(std::istream& in) {
        // Read X and Y
        const float x = GestureIOHelper::readFloat(in);
        const float y = GestureIOHelper::readFloat(in);
        // Read timestamp
        const int64_t timeStamp = GestureIOHelper::readLong(in);
        return GesturePoint{x, y, timeStamp};
    }
};
}/*endof namespace*/
#endif/*__GESTURE_POINT_H__*/
