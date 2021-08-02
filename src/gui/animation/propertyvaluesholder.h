#pragma once

#include <map>
#include <string>
#include <functional>
#include <animation/property.h>

namespace cdroid{

typedef std::function<float (float fraction, float startValue, float endValue)>TypeEvaluator;
class PropertyValuesHolder{
private:
    struct KeyFrame{
       float fraction;
       float value;
    };
    std::string mPropertyName;
    TypeEvaluator mEvaluator;
    //TypeConverter mConverter;
    float mAnimatedValue;
    std::vector<KeyFrame>mKeyFrames;
    //TimeInterpolator mInterpolator;
protected:
    Property* mProperty;
    float getValue(float fraction)const;
public: 
    typedef std::function<void(float)>PropertySetter;
    typedef std::function<float()>PropertyGetter;
private:
    static std::map<const std::string,std::map<const std::string,PropertyGetter>>sGetterPropertyMap;
    static std::map<const std::string,std::map<const std::string,PropertySetter>>sSetterPropertyMap;

public:
    PropertyValuesHolder(const std::string propertyName);
    PropertyValuesHolder(Property*prop);
    static PropertyValuesHolder* ofInt(Property*prop,const std::vector<int>&values);
    static PropertyValuesHolder* ofInt(const std::string&,const std::vector<int>&);
    static PropertyValuesHolder* ofFloat(Property*prop,const std::vector<float>&values);
    static PropertyValuesHolder* ofFloat(const std::string&,const std::vector<float>&);
    void setIntValues(const std::vector<int>&);
    void setFloatValues(const std::vector<float>&); 
    void setPropertyName(const std::string& propertyName);
    const std::string getPropertyName()const;
    void setProperty(Property* property);
    void calculateValue(float fraction);
    float getAnimatedValue()const;
    //void setConverter(TypeConverter converter);
};

}//endof namespace
