#pragma once

#include <map>
#include <string>
#include <functional>
#include <vector>
#include <cmath>
#include <iostream>
#include <core/color.h>
#include <core/variant.h>
#include <animation/property.h>
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

class PropertyValuesHolder{
public:
    using PropertySetter = std::function<void(const std::string&prop,AnimateValue&v)>;
    using PropertyGetter = std::function<AnimateValue(const std::string&prop)>;
    using OnPropertyChangedListener = std::function<void(const std::string&,void*target,float)>;
protected:
    std::string mPropertyName;
    Property*mProperty;
    OnPropertyChangedListener mOnPropertyChangedListener;
    std::vector<AnimateValue>mDataSource;
    AnimateValue mStartValue;
    AnimateValue mEndValue;
    AnimateValue mAnimateValue;
    void setupValue(void*target);
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
    void setPropertyChangedListener(const OnPropertyChangedListener&);
    
    void setValues(const std::vector<int>&values);
    void setValues(const std::vector<uint32_t>&values);
    void setValues(const std::vector<float>&values);
    virtual void setFraction(void*target,float fraction);
    const AnimateValue& getAnimatedValue()const;

    void setAnimatedValue(void*target);
    void setupStartValue(void*target);
    void setupEndValue(void*target);

    static PropertyValuesHolder*ofInt(const std::string&name,const std::vector<int>&);
    static PropertyValuesHolder*ofInt(Property*,const std::vector<int>&);
    static PropertyValuesHolder*ofFloat(const std::string&name,const std::vector<float>&);
    static PropertyValuesHolder*ofFloat(Property*prop,const std::vector<float>&);
    static PropertyValuesHolder*ofObject(const std::string&propertyName,const std::vector<void*>&);
};

typedef PropertyValuesHolder  IntPropertyValuesHolder;
typedef PropertyValuesHolder  FloatPropertyValuesHolder;

}//endof namespace
