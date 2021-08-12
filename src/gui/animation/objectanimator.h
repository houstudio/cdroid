#pragma once
#include <animation/valueanimator.h>
#include <animation/animationhandler.h>
namespace cdroid{

class ObjectAnimator:public ValueAnimator{
private:
    bool mAutoCancel;
public:
    static ObjectAnimator*ofInt(void* target,const std::string& propertyName, int,...);
    static ObjectAnimator*ofFloat(void* target,const std::string& propertyName, int,...);
    static ObjectAnimator*ofInt(void*target,Property*prop,int,...);
    static ObjectAnimator*ofFloat(void*target,Property*prop,int,...);

    void setAutoCancel(bool);
    bool shouldAutoCancel(const AnimationHandler::AnimationFrameCallback&anim);
};

}
