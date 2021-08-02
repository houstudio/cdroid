#pragma once
#include <animation/valueanimator.h>

namespace cdroid{

class ObjectAnimator:public ValueAnimator{
public:
    static ObjectAnimator*ofInt(void* target,const std::string& propertyName, int,...);
    static ObjectAnimator*ofFloat(void* target,const std::string& propertyName, int,...);
    static ObjectAnimator*ofInt(void*target,Property*prop,int,...);
    static ObjectAnimator*ofFloat(void*target,Property*prop,int,...);

    void setAutoCancel(bool cancel);
};

}
