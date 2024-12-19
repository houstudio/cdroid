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
    ALPHA():Property("alpha"){}
    AnimateValue get(void* object)override{
        LOGV("%p alpha=%.3f",object,((View*)object)->getAlpha());
        AnimateValue v =((View*)object)->getAlpha();
        return v;
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p alpha=%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setAlpha(GET_VARIANT(value,float));
    }
};

class BACKGROUND_COLOR:public Property{
private:
    ColorDrawable*cd;
public:
    BACKGROUND_COLOR():Property("backgroundColor"){
        cd = nullptr;
    }
    AnimateValue get(void* object)override{
        Drawable*d = ((View*)object)->getBackground();
        cd = dynamic_cast<ColorDrawable*>(d);
        LOGV("%p backgroundColor=%x",object,cd->getColor());
        return cd->getColor();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p color=%.3f",object,GET_VARIANT(value,float));
        cd->mutae()->setColor(GET_VARIANT(value,int));
    }
};

class ELEVATION:public Property{
public:
    ELEVATION():Property("elevation"){}
    AnimateValue get(void* object)override{
        LOGV("%p left=%d",object,((View*)object)->getElevation());
        AnimateValue v =((View*)object)->getElevation();
        return v;
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p left=%d",object,GET_VARIANT(value,float));
        ((View*)object)->setElevation(GET_VARIANT(value,float));
    }
};

class LEFT:public Property{
public:
    LEFT():Property("left"){}
    AnimateValue get(void* object)override{
        LOGV("%p left=%d",object,((View*)object)->getLeft());
        AnimateValue v =((View*)object)->getLeft();
        return v;
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p left=%d",object,GET_VARIANT(value,int));
        ((View*)object)->setLeft(GET_VARIANT(value,int));
    }
};

class TOP:public Property{
public:
    TOP():Property("top"){}
    AnimateValue get(void* object)override{
        LOGV("%p top=%d",object,((View*)object)->getTop());
        AnimateValue v =((View*)object)->getTop();
        return v;
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p top=%d",object,GET_VARIANT(value,int));
        ((View*)object)->setTop(GET_VARIANT(value,int));
    }
};

class RIGHT:public Property{
public:
    RIGHT():Property("right"){}
    AnimateValue get(void* object)override{
        LOGV("%p right=%d",object,((View*)object)->getRight());
        AnimateValue v =((View*)object)->getRight();
        return v;
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p right=%d",object,GET_VARIANT(value,int));
        ((View*)object)->setRight(GET_VARIANT(value,int));
    }
};

class BOTTOM:public Property{
public:
    BOTTOM():Property("bottom"){}

    AnimateValue get(void* object)override{
        LOGV("%p bottom=%d",object,((View*)object)->getBottom());
        AnimateValue v =((View*)object)->getBottom();
        return v;
    }

    void set(void* object,const AnimateValue& value)override{
        LOGV("%p bottom=%d",object,GET_VARIANT(value,int));
        ((View*)object)->setBottom(GET_VARIANT(value,int));
    }
};

class TRANSLATION_X:public Property{
public:
    TRANSLATION_X():Property("translationX"){}
    AnimateValue get(void* object) {
        return ((View*)object)->getTranslationX();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p translationX=%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setTranslationX(GET_VARIANT(value,float));
    }
};

class TRANSLATION_Y:public Property{
public:
    TRANSLATION_Y():Property("translationY"){}
    AnimateValue get(void* object) {
        return ((View*)object)->getTranslationY();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p translationY=%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setTranslationY(GET_VARIANT(value,float));
    }
};

class TRANSLATION_Z:public Property{
public:
    TRANSLATION_Z():Property("translationZ"){}
    AnimateValue get(void* object) {
        return ((View*)object)->getTranslationZ();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p translationZ=%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setTranslationZ(GET_VARIANT(value,float));
    }
};

class XX:public Property{
public:
    XX():Property("x"){}
    AnimateValue get(void* object) {
        return ((View*)object)->getX();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p X%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setX(GET_VARIANT(value,float));
    }

};

class YY:public Property{
public:
    YY():Property("y"){}
    AnimateValue get(void* object) {
        return ((View*)object)->getY();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p Y=%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setY(GET_VARIANT(value,float));
    }
};

class ZZ:public Property{
public:
    ZZ():Property("z"){}
    AnimateValue get(void* object) {
        return ((View*)object)->getZ();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p Z=%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setZ(GET_VARIANT(value,float));
    }
};

class PIVOT_X:public Property{
public:
    PIVOT_X():Property("pivotX"){}
    AnimateValue get(void* object) {
        return ((View*)object)->getPivotX();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p pivotX=%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setPivotX(GET_VARIANT(value,float));
    }
};

class PIVOT_Y:public Property{
public:
    PIVOT_Y():Property("pivotY"){}
    AnimateValue get(void* object) {
        return ((View*)object)->getPivotY();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p pivotX=%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setPivotY(GET_VARIANT(value,float));
    }
};

class ROTATION:public Property{
public:
    ROTATION():Property("rotation"){}
    AnimateValue get(void* object) {
        return ((View*)object)->getRotation();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p rotation=%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setRotation(GET_VARIANT(value,float));
    }
};

class ROTATION_X:public Property{
public:
    ROTATION_X():Property("rotationX"){}
    AnimateValue get(void* object) {
        return ((View*)object)->getRotationX();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p rotationX=%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setRotationX(GET_VARIANT(value,float));
    }
};

class ROTATION_Y:public Property{
public:
    ROTATION_Y():Property("rotationY"){}
    AnimateValue get(void* object) {
        return ((View*)object)->getRotationY();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p rotationY=%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setRotationY(GET_VARIANT(value,float));
    }
};

class SCALE_X:public Property{
public:
    SCALE_X():Property("scaleX"){}
    AnimateValue get(void* object) {
        return ((View*)object)->getScaleX();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p scaleX=%f",object,GET_VARIANT(value,float));
        ((View*)object)->setScaleX(GET_VARIANT(value,float));
    }
};

class SCALE_Y:public Property{
public:
    SCALE_Y():Property("scaleY"){}
    AnimateValue get(void* object) {
        return ((View*)object)->getScaleY();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p scaleY=%f",GET_VARIANT(value,float));
        ((View*)object)->setScaleY(GET_VARIANT(value,float));
    }
};

class SCROLL_X:public Property{
public:
    SCROLL_X():Property("scrollX"){}
    AnimateValue get(void* object) {
        return ((View*)object)->getScrollX();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p scrollX=%d",object,GET_VARIANT(value,int));
        ((View*)object)->setScrollX(GET_VARIANT(value,int));
    }
};

class SCROLL_Y:public Property{
public:
    SCROLL_Y():Property("scrollY"){}
    AnimateValue get(void* object) {
        return ((View*)object)->getScrollY();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p scaleY=%d",GET_VARIANT(value,int));
        ((View*)object)->setScrollY(GET_VARIANT(value,int));
    }
};

static std::map<const std::string,Property*>props={
    {"alpha",new ALPHA()},
    {"bottom",new BOTTOM()},
    {"elevation",new ELEVATION()},
    {"left",new LEFT()},
    {"pivotX",new PIVOT_X()},
    {"pivotY",new PIVOT_Y()},
    {"right",new RIGHT()},
    {"rotation" ,new ROTATION()},
    {"rotationX",new ROTATION_X()},
    {"rotationY",new ROTATION_Y()},
    {"scaleX",new SCALE_X()},
    {"scaleY",new SCALE_Y()},
    {"scrollX",new SCROLL_X()},
    {"scrollY",new SCROLL_Y()},
    {"top",new TOP()},
    {"translationX",new TRANSLATION_X()},
    {"translationY",new TRANSLATION_Y()},
    {"translationZ",new TRANSLATION_Z()},
    {"x",new XX()},
    {"y",new YY()},
    {"z",new ZZ()}
};

Property*Property::fromName(const std::string&propertyName){
    auto it = props.find(propertyName);
    if(it!=props.end()){
        return it->second;
    }
    LOGD_IF(!propertyName.empty(),"%s =nullptr",propertyName.c_str());
    return nullptr;
}

}/*endof namespace*/

