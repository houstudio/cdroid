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
class ALPHA:public Property{
public:
    ALPHA():Property("alpha"){
    }

    AnimateValue get(void* object){
        LOGD("getAlpha %p,%.3f",object,((View*)object)->getAlpha());
        AnimateValue v =((View*)object)->getAlpha();
        return v;
    }
    
    void set(void* object,const AnimateValue& value){
        LOGD("setAlpha %p->%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setAlpha(GET_VARIANT(value,float));
    }
};

class TRANSLATION_X:public Property{
public:
    TRANSLATION_X():Property("translationX"){
    }

    void setValue(void* object,const AnimateValue& value) {
        LOGD("setTranslationX%p->%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setTranslationX(GET_VARIANT(value,float));
    }

    AnimateValue get(void* object) {
        return ((View*)object)->getTranslationX();
    }
};

class TRANSLATION_Y:public Property{
public:
    TRANSLATION_Y():Property("translationY"){
    }

    void setValue(void* object,const AnimateValue& value) {
        ((View*)object)->setTranslationY(GET_VARIANT(value,float));
    }

    AnimateValue get(void* object) {
        return ((View*)object)->getTranslationY();
    }
};

class XX:public Property{
public:
    XX():Property("x"){
    }

    void setValue(void* object,const AnimateValue& value) {
        ((View*)object)->setX(GET_VARIANT(value,float));
    }

    AnimateValue get(void* object) {
        return ((View*)object)->getX();
    }
};

class YY:public Property{
public:
    YY():Property("y"){
    }

    void setValue(void* object,const AnimateValue& value) {
        ((View*)object)->setY(GET_VARIANT(value,float));
    }

    AnimateValue get(void* object) {
        return ((View*)object)->getY();
    }
};

class ROTATION:public Property{
public:
    ROTATION():Property("rotation"){
    }

    void setValue(void* object,const AnimateValue& value) {
        ((View*)object)->setRotation(GET_VARIANT(value,float));
    }

    AnimateValue get(void* object) {
        return ((View*)object)->getRotation();
    }
};

class ROTATIONX:public Property{
public:
    ROTATIONX():Property("rotationX"){
    }

    void setValue(void* object,const AnimateValue& value) {
        ((View*)object)->setRotationX(GET_VARIANT(value,float));
    }

    AnimateValue get(void* object) {
        return ((View*)object)->getRotationX();
    }
};

class ROTATIONY:public Property{
public:
    ROTATIONY():Property("rotationY"){
    }

    void setValue(void* object,const AnimateValue& value) {
        ((View*)object)->setRotationY(GET_VARIANT(value,float));
    }

    AnimateValue get(void* object) {
        return ((View*)object)->getRotationY();
    }
};

class SCALEX:public Property{
public:
    SCALEX():Property("scaleX"){
    }

    void setValue(void* object,const AnimateValue& value) {
        ((View*)object)->setScaleX(GET_VARIANT(value,float));
    }

    AnimateValue get(void* object) {
        return ((View*)object)->getScaleX();
    }
};

class SCALEY:public Property{
public:
    SCALEY():Property("scaleY"){
    }

    void setValue(void* object,const AnimateValue& value) {
        ((View*)object)->setScaleY(GET_VARIANT(value,float));
    }

    AnimateValue get(void* object) {
        return ((View*)object)->getScaleY();
    }
};

static std::map<const std::string,Property*>props={
    {"alpha",new ALPHA()},
    {"translationX",new TRANSLATION_X()}
};

Property*Property::propertyFromName(const std::string&propertyName){
    auto it = props.find(propertyName);
    if(it!=props.end()){
        LOGD_IF(propertyName.size(),"%s =%p",propertyName.c_str(),it->second);
        return it->second;
    }
    LOGD_IF(propertyName.size(),"%s =nullptr",propertyName.c_str());
    return nullptr;
}

}/*endof namespace*/

