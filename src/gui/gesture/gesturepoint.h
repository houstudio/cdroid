#ifndef __GESTURE_POINT_H__
#define __GESTURE_POINT_H__
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
        const float x =1;// in.readFloat();
        const float y =1;// in.readFloat();
        // Read timestamp
        const long timeStamp =1;// in.readLong();
        return GesturePoint{x, y, timeStamp};
    }
};
}/*endof namespace*/
#endif/*__GESTURE_POINT_H__*/
