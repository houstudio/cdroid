/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <animation/property.h>
#include <view/view.h>

namespace cdroid{

Property::Property(const std::string&name){
    mName = name;
    mType = UNDEFINED;
}

Property::Property(const std::string&name,int type){
    mName = name;
    mType = type;
}

AnimateValue Property::get(void* t){
    AnimateValue v=0.f;
    return v;
}

void Property::set(void* object,const AnimateValue& value){
}

int Property::getType()const{
    return mType;
}

const std::string Property::getName()const{
    return mName;
}

////////////////////////////////////////////////////////////////////////////
namespace {
class ALPHA:public Property{
public:
    ALPHA():Property("alpha",FLOAT_TYPE){}
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
    BACKGROUND_COLOR():Property("backgroundColor",COLOR_TYPE){
        cd = nullptr;
    }
    AnimateValue get(void* object)override{
        Drawable*d = ((View*)object)->getBackground();
        cd = dynamic_cast<ColorDrawable*>(d);
        LOGV("%p backgroundColor=%x",object,cd->getColor());
        return cd->getColor();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p color=%.3f",object,GET_VARIANT(value,int));
        cd->mutate()->setColor(GET_VARIANT(value,int));
    }
};

class ELEVATION:public Property{
public:
    ELEVATION():Property("elevation",FLOAT_TYPE){}
    AnimateValue get(void* object)override{
        LOGV("%p elevation=%.3f",object,((View*)object)->getElevation());
        AnimateValue v =((View*)object)->getElevation();
        return v;
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p elevation=%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setElevation(GET_VARIANT(value,float));
    }
};

class LEFT:public Property{
public:
    LEFT():Property("left",INT_TYPE){}
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
    TOP():Property("top",INT_TYPE){}
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
    RIGHT():Property("right",INT_TYPE){}
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
    BOTTOM():Property("bottom",INT_TYPE){}

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
    TRANSLATION_X():Property("translationX",FLOAT_TYPE){}
    AnimateValue get(void* object) override{
        LOGV("translationX=%f",((View*)object)->getTranslationX());
        return ((View*)object)->getTranslationX();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p translationX=%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setTranslationX(GET_VARIANT(value,float));
    }
};

class TRANSLATION_Y:public Property{
public:
    TRANSLATION_Y():Property("translationY",FLOAT_TYPE){}
    AnimateValue get(void* object) override{
        return ((View*)object)->getTranslationY();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p translationY=%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setTranslationY(GET_VARIANT(value,float));
    }
};

class TRANSLATION_Z:public Property{
public:
    TRANSLATION_Z():Property("translationZ",FLOAT_TYPE){}
    AnimateValue get(void* object) override{
        return ((View*)object)->getTranslationZ();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p translationZ=%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setTranslationZ(GET_VARIANT(value,float));
    }
};

class XX:public Property{
public:
    XX():Property("x",FLOAT_TYPE){}
    AnimateValue get(void* object) override{
        return ((View*)object)->getX();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p X%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setX(GET_VARIANT(value,float));
    }

};

class YY:public Property{
public:
    YY():Property("y",FLOAT_TYPE){}
    AnimateValue get(void* object) override{
        return ((View*)object)->getY();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p Y=%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setY(GET_VARIANT(value,float));
    }
};

class ZZ:public Property{
public:
    ZZ():Property("z",FLOAT_TYPE){}
    AnimateValue get(void* object) override{
        return ((View*)object)->getZ();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p Z=%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setZ(GET_VARIANT(value,float));
    }
};

class PIVOT_X:public Property{
public:
    PIVOT_X():Property("pivotX",FLOAT_TYPE){}
    AnimateValue get(void* object) override{
        return ((View*)object)->getPivotX();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p pivotX=%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setPivotX(GET_VARIANT(value,float));
    }
};

class PIVOT_Y:public Property{
public:
    PIVOT_Y():Property("pivotY",FLOAT_TYPE){}
    AnimateValue get(void* object) override{
        return ((View*)object)->getPivotY();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p pivotY=%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setPivotY(GET_VARIANT(value,float));
    }
};

class ROTATION:public Property{
public:
    ROTATION():Property("rotation",FLOAT_TYPE){}
    AnimateValue get(void* object) override{
        return ((View*)object)->getRotation();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p rotation=%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setRotation(GET_VARIANT(value,float));
    }
};

class ROTATION_X:public Property{
public:
    ROTATION_X():Property("rotationX",FLOAT_TYPE){}
    AnimateValue get(void* object) override{
        return ((View*)object)->getRotationX();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p rotationX=%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setRotationX(GET_VARIANT(value,float));
    }
};

class ROTATION_Y:public Property{
public:
    ROTATION_Y():Property("rotationY",FLOAT_TYPE){}
    AnimateValue get(void* object) override{
        return ((View*)object)->getRotationY();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p rotationY=%.3f",object,GET_VARIANT(value,float));
        ((View*)object)->setRotationY(GET_VARIANT(value,float));
    }
};

class SCALE_X:public Property{
public:
    SCALE_X():Property("scaleX",FLOAT_TYPE){}
    AnimateValue get(void* object) override{
        return ((View*)object)->getScaleX();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p scaleX=%f",object,GET_VARIANT(value,float));
        ((View*)object)->setScaleX(GET_VARIANT(value,float));
    }
};

class SCALE_Y:public Property{
public:
    SCALE_Y():Property("scaleY",FLOAT_TYPE){}
    AnimateValue get(void* object) override{
        return ((View*)object)->getScaleY();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p scaleY=%f",GET_VARIANT(value,float));
        ((View*)object)->setScaleY(GET_VARIANT(value,float));
    }
};

class SCROLL_X:public Property{
public:
    SCROLL_X():Property("scrollX",INT_TYPE){}
    AnimateValue get(void* object) override{
        return ((View*)object)->getScrollX();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p scrollX=%d",object,GET_VARIANT(value,int));
        ((View*)object)->setScrollX(GET_VARIANT(value,int));
    }
};

class SCROLL_Y:public Property{
public:
    SCROLL_Y():Property("scrollY",INT_TYPE){}
    AnimateValue get(void* object) override{
        return ((View*)object)->getScrollY();
    }
    void set(void* object,const AnimateValue& value)override{
        LOGV("%p scaleY=%d",GET_VARIANT(value,int));
        ((View*)object)->setScrollY(GET_VARIANT(value,int));
    }
};
}
static std::unordered_map<std::string,std::shared_ptr<Property>>props={
    {"alpha",std::make_shared<ALPHA>()},
    {"bottom",std::make_shared<BOTTOM>()},
    {"backgroundColor",std::make_shared<BACKGROUND_COLOR>()},
    {"elevation",std::make_shared<ELEVATION>()},
    {"left",std::make_shared<LEFT>()},
    {"pivotX",std::make_shared<PIVOT_X>()},
    {"pivotY",std::make_shared<PIVOT_Y>()},
    {"right" ,std::make_shared<RIGHT>()},
    {"rotation" ,std::make_shared<ROTATION>()},
    {"rotationX",std::make_shared<ROTATION_X>()},
    {"rotationY",std::make_shared<ROTATION_Y>()},
    {"scaleX" ,std::make_shared<SCALE_X>()},
    {"scaleY" ,std::make_shared<SCALE_Y>()},
    {"scrollX",std::make_shared<SCROLL_X>()},
    {"scrollY",std::make_shared<SCROLL_Y>()},
    {"top",std::make_shared<TOP>()},
    {"translationX",std::make_shared<TRANSLATION_X>()},
    {"translationY",std::make_shared<TRANSLATION_Y>()},
    {"translationZ",std::make_shared<TRANSLATION_Z>()},
    {"x",std::make_shared<XX>()},
    {"y",std::make_shared<YY>()},
    {"z",std::make_shared<ZZ>()}
};

Property*Property::fromName(const std::string&propertyName){
    auto it = props.find(propertyName);
    if(propertyName.empty())
        return nullptr;
    if(it!=props.end()){
        return it->second.get();
    }
    LOGD_IF(!propertyName.empty(),"%s =nullptr",propertyName.c_str());
    return nullptr;
}

Property*Property::fromName(const std::string&className,const std::string&propertyName){
    if(className.empty()||className.compare("View")==0)return fromName(propertyName);
    return nullptr;
}

bool Property::reigsterProperty(const std::string&propertyName,Property*prop){
    auto it = props.find(propertyName);
    if(it==props.end()){
        std::shared_ptr<Property>ptr(prop);
        props.insert({propertyName,ptr});
        return true;
    }
    return false;
}

}/*endof namespace*/

