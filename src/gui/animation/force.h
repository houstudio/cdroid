#ifndef __DYNAMIC_ANIMATION_FORCE_H__
#define __DYNAMIC_ANIMATION_FORCE_H__
namespace cdroid{
class Force {
public:
    // Acceleration based on position.
    virtual float getAcceleration(float position, float velocity)const=0;

    virtual bool isAtEquilibrium(float value, float velocity)const=0;
    virtual ~Force()=default;
};
}
#endif
