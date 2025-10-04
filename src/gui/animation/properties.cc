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

AnimateValue Property::get(void* t) const{
    AnimateValue v=0.f;
    return v;
}

void Property::set(void* object,const AnimateValue& value) const{
}

int Property::getType()const{
    return mType;
}

const std::string Property::getName()const{
    return mName;
}

////////////////////////////////////////////////////////////////////////////

#define DEFINE_FLOATPROPERTY(PROPNAME, METHOD, PROJ)        \
namespace {                                                 \
class prop_##PROJ : public Property {                       \
public:                                                     \
    prop_##PROJ() : Property(PROPNAME,FLOAT_TYPE) {}        \
    void set(void*obj,const AnimateValue& v)const override{ \
        ((View*)obj)->set##METHOD(GET_VARIANT(v,float));    \
    }                                                       \
    AnimateValue get(void*obj)const override                \
            { return ((View*)obj)->get##METHOD(); }         \
};                                                          \
static const prop_##PROJ PROJ;        \
}

#define DEFINE_INTPROPERTY(PROPNAME, METHOD, PROJ)          \
namespace {                                                 \
class prop_##PROJ : public Property {                       \
public:                                                     \
    prop_##PROJ() : Property(PROPNAME,INT_TYPE) {}          \
    void set(void*obj,const AnimateValue& v)const override{ \
        ((View*)obj)->set##METHOD(GET_VARIANT(v,int));      \
    }                                                       \
    AnimateValue get(void*obj)const override                \
            { return ((View*)obj)->get##METHOD(); }         \
};                                                          \
static const prop_##PROJ PROJ;        \
}

DEFINE_FLOATPROPERTY("elevation",Elevation,ELEVATION);
DEFINE_FLOATPROPERTY("pivotX",PivotX,PIVOT_X);
DEFINE_FLOATPROPERTY("pivotY",PivotY,PIVOT_Y);

DEFINE_INTPROPERTY("left",Left,LEFT);
DEFINE_INTPROPERTY("top",Top,TOP);
DEFINE_INTPROPERTY("right",Right,RIGHT);
DEFINE_INTPROPERTY("bottom",Bottom,BOTTOM);
DEFINE_INTPROPERTY("scrollX",ScrollX,SCROLL_X);
DEFINE_INTPROPERTY("scrollY",ScrollY,SCROLL_Y);

namespace {

class __BACKGROUND_COLOR:public Property{
public:
    __BACKGROUND_COLOR():Property("backgroundColor",COLOR_TYPE){}
    AnimateValue get(void* object)const override{
        ColorDrawable*cd = dynamic_cast<ColorDrawable*>(((View*)object)->getBackground());
        LOGV("%p backgroundColor=%x",object,cd->getColor());
        return cd->getColor();
    }
    void set(void* object,const AnimateValue& value)const override{
        ColorDrawable*cd = dynamic_cast<ColorDrawable*>(((View*)object)->getBackground());
        LOGV("%p color=%.3f",object,GET_VARIANT(value,int));
        cd->mutate()->setColor(GET_VARIANT(value,int));
    }
};
static const __BACKGROUND_COLOR BACKGROUND_COLOR;

}
static std::unordered_map<std::string,const Property*>props={
    {"alpha",View::ALPHA},
    {"bottom",&BOTTOM},
    {"backgroundColor",&BACKGROUND_COLOR},
    {"elevation",&ELEVATION},
    {"left",&LEFT},
    {"pivotX",&PIVOT_X},
    {"pivotY",&PIVOT_Y},
    {"right" ,&RIGHT},
    {"rotation" ,View::ROTATION},
    {"rotationX",View::ROTATION_X},
    {"rotationY",View::ROTATION_Y},
    {"scaleX" ,View::SCALE_X},
    {"scaleY" ,View::SCALE_Y},
    {"scrollX",&SCROLL_X},
    {"scrollY",&SCROLL_Y},
    {"top",&TOP},
    {"translationX",View::TRANSLATION_X},
    {"translationY",View::TRANSLATION_Y},
    {"translationZ",View::TRANSLATION_Z},
    {"x",View::X},
    {"y",View::Y},
    {"z",View::Z}
};

Property*Property::fromName(const std::string&propertyName){
    auto it = props.find(propertyName);
    if(propertyName.empty())
        return nullptr;
    if(it!=props.end()){
        return (Property*)it->second;
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
        props.insert({propertyName,prop});
        return true;
    }
    return false;
}

}/*endof namespace*/

