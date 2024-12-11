#include <animation/property.h>
#include <view/view.h>
namespace cdroid{

Property::Property(const std::string&name){
    mName = name;
}

AnimateValue Property::get(void* t){
    AnimateValue v=0.f;
    return v;
}

void Property::set(void* object,const AnimateValue& value){
}

const std::string Property::getName()const{
    return mName;
}

////////////////////////////////////////////////////////////////////////////
class LEFT:public Property{
public:
    LEFT():Property("left"){}

    AnimateValue get(void* object)override{
        LOGD("left %p,%d",object,((View*)object)->getLeft());
        AnimateValue v =((View*)object)->getLeft();
        return v;
    }

    void set(void* object,const AnimateValue& value)override{
        LOGD("left %p->%d",object,GET_VARIANT(value,int));
        ((View*)object)->setLeft(GET_VARIANT(value,int));
    }
};

class TOP:public Property{
public:
    TOP():Property("top"){}

    AnimateValue get(void* object)override{
        LOGD("top %p,%d",object,((View*)object)->getTop());
        AnimateValue v =((View*)object)->getTop();
        return v;
    }

    void set(void* object,const AnimateValue& value)override{
        LOGD("top %p->%d",object,GET_VARIANT(value,int));
        ((View*)object)->setTop(GET_VARIANT(value,int));
    }
};

class RIGHT:public Property{
public:
    RIGHT():Property("right"){}

    AnimateValue get(void* object)override{
        LOGD("right %p,%d",object,((View*)object)->getRight());
        AnimateValue v =((View*)object)->getLeft();
        return v;
    }

    void set(void* object,const AnimateValue& value)override{
        LOGD("right %p->%d",object,GET_VARIANT(value,int));
        ((View*)object)->setRight(GET_VARIANT(value,int));
    }
};

class BOTTOM:public Property{
public:
    BOTTOM():Property("bottom"){}

    AnimateValue get(void* object)override{
        LOGD("bottom %p,%d",object,((View*)object)->getBottom());
        AnimateValue v =((View*)object)->getBottom();
        return v;
    }

    void set(void* object,const AnimateValue& value)override{
        LOGD("bottom %p->%d",object,GET_VARIANT(value,int));
        ((View*)object)->setBottom(GET_VARIANT(value,int));
    }
};

class ALPHA:public Property{
public:
    ALPHA():Property("alpha"){}

    AnimateValue get(void* object)override{
        LOGV("alpha %p,%.3f",object,((View*)object)->getAlpha());
        AnimateValue v =((View*)object)->getAlpha();
        return v;
    }
    
    void set(void* object,const AnimateValue& value)override{
        LOGV("alpha %p->%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setAlpha(GET_VARIANT(value,float));
    }
};

class TRANSLATION_X:public Property{
public:
    TRANSLATION_X():Property("translationX"){}

    void set(void* object,const AnimateValue& value)override{
        LOGV("translationX %p->%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setTranslationX(GET_VARIANT(value,float));
    }

    AnimateValue get(void* object) {
        return ((View*)object)->getTranslationX();
    }
};

class TRANSLATION_Y:public Property{
public:
    TRANSLATION_Y():Property("translationY"){}

    void set(void* object,const AnimateValue& value)override{
        LOGV("translationY %p->%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setTranslationY(GET_VARIANT(value,float));
    }

    AnimateValue get(void* object) {
        return ((View*)object)->getTranslationY();
    }
};

class TRANSLATION_Z:public Property{
public:
    TRANSLATION_Z():Property("translationZ"){}
    void set(void* object,const AnimateValue& value)override{
        LOGV("translationZ %p->%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setTranslationZ(GET_VARIANT(value,float));
    }

    AnimateValue get(void* object) {
        return ((View*)object)->getTranslationZ();
    }
};

class XX:public Property{
public:
    XX():Property("x"){}

    void set(void* object,const AnimateValue& value)override{
        LOGV("X %p->%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setX(GET_VARIANT(value,float));
    }

    AnimateValue get(void* object) {
        return ((View*)object)->getX();
    }
};

class YY:public Property{
public:
    YY():Property("y"){}

    void set(void* object,const AnimateValue& value)override{
        LOGV("Y %p->%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setY(GET_VARIANT(value,float));
    }

    AnimateValue get(void* object) {
        return ((View*)object)->getY();
    }
};

class ZZ:public Property{
public:
    ZZ():Property("z"){}
    void set(void* object,const AnimateValue& value)override{
        LOGV("Z %p->%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setZ(GET_VARIANT(value,float));
    }

    AnimateValue get(void* object) {
        return ((View*)object)->getZ();
    }
};

class ROTATION:public Property{
public:
    ROTATION():Property("rotation"){}

    void set(void* object,const AnimateValue& value)override{
        LOGV("rotation %p->%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setRotation(GET_VARIANT(value,float));
    }

    AnimateValue get(void* object) {
        return ((View*)object)->getRotation();
    }
};

class ROTATION_X:public Property{
public:
    ROTATION_X():Property("rotationX"){}

    void set(void* object,const AnimateValue& value)override{
        LOGV("rotationX %p->%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setRotationX(GET_VARIANT(value,float));
    }

    AnimateValue get(void* object) {
        return ((View*)object)->getRotationX();
    }
};

class ROTATION_Y:public Property{
public:
    ROTATION_Y():Property("rotationY"){}

    void set(void* object,const AnimateValue& value)override{
        LOGV("rotationY %p->%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setRotationY(GET_VARIANT(value,float));
    }

    AnimateValue get(void* object) {
        return ((View*)object)->getRotationY();
    }
};

class SCALE_X:public Property{
public:
    SCALE_X():Property("scaleX"){}

    void set(void* object,const AnimateValue& value)override{
        LOGV("scaleX %p-->%f",object,GET_VARIANT(value,float));
        ((View*)object)->setScaleX(GET_VARIANT(value,float));
    }

    AnimateValue get(void* object) {
        return ((View*)object)->getScaleX();
    }
};

class SCALE_Y:public Property{
public:
    SCALE_Y():Property("scaleY"){}

    void set(void* object,const AnimateValue& value)override{
        LOGV("scaleY %p->%f",GET_VARIANT(value,float));
        ((View*)object)->setScaleY(GET_VARIANT(value,float));
    }

    AnimateValue get(void* object) {
        return ((View*)object)->getScaleY();
    }
};

static std::map<const std::string,Property*>props={
    {"left",new LEFT()},
    {"top",new TOP()},
    {"right",new RIGHT()},
    {"bottom",new BOTTOM()},
    {"alpha",new ALPHA()},
    {"rotation" ,new ROTATION()},
    {"rotationX",new ROTATION_X()},
    {"rotationY",new ROTATION_Y()},
    {"scaleX",new SCALE_X()},
    {"scaleY",new SCALE_Y()},
    {"translationX",new TRANSLATION_X()},
    {"translationY",new TRANSLATION_Y()},
    {"translationZ",new TRANSLATION_Z()},
    {"x",new XX()},
    {"y",new YY()},
    {"z",new ZZ()}
};

Property*Property::propertyFromName(const std::string&propertyName){
    auto it = props.find(propertyName);
    if(it!=props.end()){
        LOGV_IF(propertyName.size(),"%s =%p",propertyName.c_str(),it->second);
        return it->second;
    }
    LOGV_IF(propertyName.size(),"%s =nullptr",propertyName.c_str());
    return nullptr;
}

}/*endof namespace*/

