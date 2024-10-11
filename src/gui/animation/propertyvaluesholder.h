#pragma once

#include <map>
#include <string>
#include <functional>
#include <vector>
#include <cmath>
#include <iostream>
#include <cdtypes.h>
#include <core/color.h>
#include <core/variant.h>
//reference:
//http://androidxref.com/9.0.0_r3/xref/frameworks/base/libs/hwui/PropertyValuesHolder.h
#if variant_CPP17_OR_GREATER
   #define GET_VARIANT(vt,type) std::get<type>(vt)
#else
   #define GET_VARIANT(vt,type) vt.get<type>()
#endif
namespace cdroid{
typedef nonstd::variant<int,uint32_t,float>AnimateValue;

inline constexpr float lerp(float fromValue, float toValue, float fraction) {
    return float(fromValue * (1.f - fraction) + toValue * fraction);
}
inline int lerp(int startValue, int endValue, float fraction) {
    return int(startValue + std::round(fraction * (endValue - startValue)));
}

class Property{
private:
    std::string mName;
public:
    Property(const std::string&name){
        mName=name;
    }
    virtual float get(void* t){return .0;};
    virtual void set(void* object, float value){};
    const std::string getName()const{return mName;}
};

class PropertyValuesHolder{
protected:
    std::string mPropertyName;
    Property*mProperty;
    std::vector<AnimateValue>mDataSource;
    AnimateValue mStartValue;
    AnimateValue mEndValue;
    AnimateValue mAnimateValue;
    virtual void evaluate(AnimateValue& out, const AnimateValue& from, const AnimateValue& to, float fraction)const;
public:
    PropertyValuesHolder();
    PropertyValuesHolder(const PropertyValuesHolder&);
    virtual ~PropertyValuesHolder();
    PropertyValuesHolder(Property*prop);
    PropertyValuesHolder(const std::string&name);
    void setPropertyName(const std::string& propertyName);
    const std::string getPropertyName()const;
    void setProperty(Property*p);
    Property*getProperty();
    
    void setValues(const std::vector<int>&values);
    void setValues(const std::vector<uint32_t>&values);
    void setValues(const std::vector<float>&values);
    virtual void setFraction(void*target,float fraction);
    const AnimateValue& getAnimatedValue()const;

    static PropertyValuesHolder*ofInt(const std::string&name,const std::vector<int>&);
    static PropertyValuesHolder*ofInt(Property*,const std::vector<int>&);
    static PropertyValuesHolder*ofFloat(const std::string&name,const std::vector<float>&);
    static PropertyValuesHolder*ofFloat(Property*prop,const std::vector<float>&);
};

typedef PropertyValuesHolder  IntPropertyValuesHolder;
typedef PropertyValuesHolder  FloatPropertyValuesHolder;

}//endof namespace
