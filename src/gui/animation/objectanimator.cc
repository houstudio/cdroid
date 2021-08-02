#include <animation/objectanimator.h>
#include <stdarg.h>

namespace cdroid{

void ObjectAnimator::setAutoCancel(bool cancel){
}

ObjectAnimator* ObjectAnimator::ofInt(void* target,const std::string& propertyName, int count,...){
    ObjectAnimator*anim=new ObjectAnimator();
    va_list ap;
    std::vector<int>values;
    va_start(ap,count);
    for(int i=0;i<count;i++){
        values.push_back(va_arg(ap,int));
    }
    va_end(ap);
    anim->setIntValues(values);
    return anim;
}

ObjectAnimator* ObjectAnimator::ofFloat(void* target,const std::string& propertyName, int count,...){
    ObjectAnimator*anim=new ObjectAnimator();
    va_list ap;
    std::vector<float>values;
    va_start(ap,count);
    for(int i=0;i<count;i++){
        values.push_back(va_arg(ap,int));
    }
    va_end(ap);
    anim->setFloatValues(values);
    return anim;
}

ObjectAnimator* ObjectAnimator::ofInt(void*target,Property*prop,int count,...){
    ObjectAnimator*anim=new ObjectAnimator();
    return anim;
}

ObjectAnimator*ObjectAnimator::ofFloat(void*target,Property*prop,int count,...){
    ObjectAnimator*anim=new ObjectAnimator();
    return anim;
}

}
