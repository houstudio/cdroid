#ifndef __UI_TRANSFORM_H__
#define __UI_TRANSFORM_H__
#include <array>
#include <string>
#include <sstream>
#include <iostream>
#include <core/rect.h>
namespace cdroid {

//class Region;
using vec2=std::array<float,2>;
using vec3=std::array<float,3>;

namespace ui {
using status_t=uint32_t;
class Transform {
public:
    Transform();
    Transform(const Transform&  other);
    explicit Transform(uint32_t orientation, int w = 0, int h = 0);
    ~Transform();

    enum RotationFlags : uint32_t {
        ROT_0 = 0,
        FLIP_H = 1, // HAL_TRANSFORM_FLIP_H
        FLIP_V = 2, // HAL_TRANSFORM_FLIP_V
        ROT_90 = 4, // HAL_TRANSFORM_ROT_90
        ROT_180 = FLIP_H | FLIP_V,
        ROT_270 = ROT_180 | ROT_90,
        ROT_INVALID = 0x80
    };

    enum type_mask : uint32_t {
        IDENTITY            = 0,
        TRANSLATE           = 0x1,
        ROTATE              = 0x2,
        SCALE               = 0x4,
        UNKNOWN             = 0x8
    };

    // query the transform
    bool preserveRects() const;

    // Returns if bilinear filtering is needed after applying this transform to avoid aliasing.
    bool needsBilinearFiltering() const;

    uint32_t getType() const;
    uint32_t getOrientation() const;
    bool operator==(const Transform& other) const;

    const vec3& operator [] (size_t i) const;  // returns column i
    float   tx() const;
    float   ty() const;
    float dsdx() const;
    float dtdx() const;
    float dtdy() const;
    float dsdy() const;
    float det() const;

    float getScaleX() const;
    float getScaleY() const;

    // modify the transform
    void        reset();
    void        set(float tx, float ty);
    void        set(float a, float b, float c, float d);
    status_t    set(uint32_t flags, float w, float h);
    void        set(const std::array<float, 9>& matrix);

    // transform data
    //Rect    makeBounds(int w, int h) const;
    vec2    transform(float x, float y) const;
    //Region  transform(const Region& reg) const;
    Rect    transform(const Rect& bounds, bool roundOutwards = false) const;
    //FloatRect transform(const FloatRect& bounds) const;
    Transform& operator = (const Transform& other);
    Transform operator * (const Transform& rhs) const;
    Transform operator * (float value) const;
    // assumes the last row is < 0 , 0 , 1 >
    vec2 transform(const vec2& v) const;
    vec3 transform(const vec3& v) const;

    // Expands from the internal 3x3 matrix to an equivalent 4x4 matrix
    //mat4 asMatrix4() const;

    Transform inverse() const;

    // for debugging
    void dump(std::ostream& result, const char* name, const char* prefix = "") const;
    void dump(const char* name, const char* prefix = "") const;

    static RotationFlags toRotationFlags(int rotation){
        switch (rotation) {
        case 0:/*ROTATION_0: */ return ROT_0;
        case 1:/*ROTATION_90:*/ return ROT_90;
        case 2:/*ROTATION_180:*/ return ROT_180;
        case 3:/*ROTATION_270:*/ return ROT_270;
        default: return ROT_INVALID;
        }
    }
    static int toRotation(RotationFlags rotationFlags){
        switch (rotationFlags) {
        case ROT_0:  return 0;//ROTATION_0;
        case ROT_90: return 1;//ROTATION_90;
        case ROT_180:return 2;//ROTATION_180;
        case ROT_270:return 3;//ROTATION_270;
        default:     return 0;//ROTATION_0;
        }
    }
private:
    struct mat33 {
        std::array<float,3>v[3];
        inline const std::array<float,3>& operator [] (size_t i) const { return v[i]; }
        inline std::array<float,3>& operator [] (size_t i) { return v[i]; }
    };

    enum { UNKNOWN_TYPE = 0x80000000 };

    uint32_t type() const;
    static bool absIsOne(float f);
    static bool isZero(float f);

    mat33               mMatrix;
    mutable uint32_t    mType;
};

}  // namespace ui
}  // namespace cdroid
#endif/*__UI_TRANSFORM_H__*/
