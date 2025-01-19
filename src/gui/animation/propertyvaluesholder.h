#ifndef __PROPERTY_VALUES_HOLDER_H__
#define __PROPERTY_VALUES_HOLDER_H__

#include <string>
#include <functional>
#include <vector>
#include <cmath>
#include <iostream>
#include <core/color.h>
#include <core/variant.h>
#include <unordered_map>
#include <animation/property.h>
//reference:
//http://androidxref.com/9.0.0_r3/xref/frameworks/base/libs/hwui/PropertyValuesHolder.h
namespace cdroid{

using TypeEvaluator = std::function<AnimateValue(float fraction,AnimateValue&startValue,AnimateValue&endValue)>;
class PropertyValuesHolder{
public:
    friend class ValueAnimator;
    using PropertySetter = std::function<void(void*target,const std::string&prop,AnimateValue&v)>;
    using PropertyGetter = std::function<AnimateValue(void*target,const std::string&prop)>;
    using OnPropertyChangedListener = std::function<void(const std::string&,void*target,float)>;
protected:
    std::string mPropertyName;
    Property*mProperty;
    OnPropertyChangedListener mOnPropertyChangedListener;
    PropertyGetter mGetter;
    PropertySetter mSetter;
    TypeEvaluator mEvaluator;
    std::vector<AnimateValue>mDataSource;
    AnimateValue mAnimateValue;
    void setupValue(void*target,int);
    void init();
    static AnimateValue evaluator(float fraction,const AnimateValue& from, const AnimateValue& to);
    void calculateValue(float fraction);
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
    const AnimateValue& getAnimatedValue()const;

    void setEvaluator(TypeEvaluator evaluator);
    void setAnimatedValue(void*target);
    void setupStartValue(void*target);
    void setupEndValue(void*target);
    void setupSetterAndGetter(void*target);

    static PropertyValuesHolder*ofInt(const std::string&name,const std::vector<int>&);
    static PropertyValuesHolder*ofInt(Property*,const std::vector<int>&);
    static PropertyValuesHolder*ofFloat(const std::string&name,const std::vector<float>&);
    static PropertyValuesHolder*ofFloat(Property*prop,const std::vector<float>&);
    static PropertyValuesHolder*ofObject(const std::string&propertyName,const std::vector<void*>&);
};

typedef PropertyValuesHolder  IntPropertyValuesHolder;
typedef PropertyValuesHolder  FloatPropertyValuesHolder;

}//endof namespace
#endif/*__PROPERTY_VALUES_HOLDER_H__*/
