#include <animation/property.h>
#include <view/view.h>
namespace cdroid{

Property::Property(const std::string&name){
    mName = name;
}

float Property::get(void* t){
    return .0;
}

void Property::set(void* object, float value){
}

const std::string Property::getName()const{
    return mName;
}

////////////////////////////////////////////////////////////////////////////
class ALPHA:public Property{
public:
    ALPHA():Property("alpha"){
    }

    float get(void* object){
        return ((View*)object)->getAlpha();
    }
    
    void set(void* object, float value){
        ((View*)object)->setAlpha(value);
    }
};

class TRANSLATION_X:public Property{
public:
    TRANSLATION_X():Property("translationX"){
    }

    void setValue(void* object, float value) {
        ((View*)object)->setTranslationX(value);
    }

    float get(void* object) {
        return ((View*)object)->getTranslationX();
    }
};
class TRANSLATION_Y:public Property{
public:
    TRANSLATION_Y():Property("translationY"){
    }

    void setValue(void* object, float value) {
        ((View*)object)->setTranslationY(value);
    }

    float get(void* object) {
        return ((View*)object)->getTranslationY();
    }
};

class XX:public Property{
public:
    XX():Property("x"){
    }

    void setValue(void* object, float value) {
        ((View*)object)->setX(value);
    }

    float get(void* object) {
        return ((View*)object)->getX();
    }
};

class YY:public Property{
public:
    YY():Property("y"){
    }

    void setValue(void* object, float value) {
        ((View*)object)->setY(value);
    }

    float get(void* object) {
        return ((View*)object)->getY();
    }
};

class ROTATION:public Property{
public:
    ROTATION():Property("rotation"){
    }

    void setValue(void* object, float value) {
        ((View*)object)->setRotation(value);
    }

    float get(void* object) {
        return ((View*)object)->getRotation();
    }
};
class ROTATIONX:public Property{
public:
    ROTATIONX():Property("rotationX"){
    }

    void setValue(void* object, float value) {
        ((View*)object)->setRotationX(value);
    }

    float get(void* object) {
        return ((View*)object)->getRotationX();
    }
};
class ROTATIONY:public Property{
public:
    ROTATIONY():Property("rotationY"){
    }

    void setValue(void* object, float value) {
        ((View*)object)->setRotationY(value);
    }

    float get(void* object) {
        return ((View*)object)->getRotationY();
    }
};

class SCALEX:public Property{
public:
    SCALEX():Property("scaleX"){
    }

    void setValue(void* object, float value) {
        ((View*)object)->setScaleX(value);
    }

    float get(void* object) {
        return ((View*)object)->getScaleX();
    }
};
class SCALEY:public Property{
public:
    SCALEY():Property("scaleY"){
    }

    void setValue(void* object, float value) {
        ((View*)object)->setScaleY(value);
    }

    float get(void* object) {
        return ((View*)object)->getScaleY();
    }
};

}/*endof namespace*/

