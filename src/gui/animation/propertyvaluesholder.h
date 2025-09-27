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
#include <drawables/pathparser.h>
//reference:
//http://androidxref.com/9.0.0_r3/xref/frameworks/base/libs/hwui/PropertyValuesHolder.h
namespace cdroid{

using TypeEvaluator = AnimateValue&(*)(float fraction,AnimateValue&out,const AnimateValue&startValue,const AnimateValue&endValue);
class PropertyValuesHolder{
public:
    friend class ValueAnimator;
    using PropertySetter = std::function<void(void*target,const std::string&prop,AnimateValue&v)>;
    using PropertyGetter = std::function<AnimateValue(void*target,const std::string&prop)>;
    using OnPropertyChangedListener = std::function<void(const std::string&,void*target,float)>;
    class PropertyValues {
    public:
        int type;
        std::string propertyName;
        AnimateValue startValue;//It seems only used for PathParser::PathData
        AnimateValue endValue;//
        using DataSource = std::function<AnimateValue(float fraction)>;
        DataSource dataSource;
    };
    static AnimateValue& ArgbEvaluator(float fraction,AnimateValue& out,const AnimateValue& from, const AnimateValue& to);
    static AnimateValue& PathDataEvaluator(float fraction,AnimateValue& out,const AnimateValue& from, const AnimateValue& to);
protected:
    int mValueType;
    std::string mPropertyName;
    const Property*mProperty;
    OnPropertyChangedListener mOnPropertyChangedListener;
    PropertyGetter mGetter;
    PropertySetter mSetter;
    TypeEvaluator mEvaluator;
    std::vector<AnimateValue>mDataSource;
    AnimateValue mAnimateValue;
    void setupValue(void*target,int);
    void init();
    static AnimateValue& evaluator(float fraction,AnimateValue&out,const AnimateValue& from, const AnimateValue& to);
    void calculateValue(float fraction);
public:
    PropertyValuesHolder();
    PropertyValuesHolder(const PropertyValuesHolder&);
    virtual ~PropertyValuesHolder();
    PropertyValuesHolder(const Property*prop);
    PropertyValuesHolder(const std::string&name);
    void setPropertyName(const std::string& propertyName);
    const std::string getPropertyName()const;
    void setProperty(Property*p);
    const Property*getProperty()const;
    int getValueType()const;
    void setPropertyChangedListener(const OnPropertyChangedListener&);
    
    void setValues(const std::vector<int>&values);
    void setValues(const std::vector<float>&values);
    void setValues(const std::vector<PathParser::PathData>&values);
    void getPropertyValues(PropertyValues&values);
    const AnimateValue& getAnimatedValue()const;

    void setEvaluator(TypeEvaluator evaluator);
    void setAnimatedValue(void*target);
    void setupStartValue(void*target);
    void setupEndValue(void*target);
    void setupSetterAndGetter(void*target);

    static PropertyValuesHolder*ofInt(const std::string&name,const std::vector<int>&);
    static PropertyValuesHolder*ofInt(const Property*,const std::vector<int>&);
    static PropertyValuesHolder*ofFloat(const std::string&name,const std::vector<float>&);
    static PropertyValuesHolder*ofFloat(const Property*prop,const std::vector<float>&);
    static PropertyValuesHolder*ofObject(const std::string&propertyName,const std::vector<void*>&);
    static PropertyValuesHolder*ofObject(const Property*prop,const std::vector<PathParser::PathData>&);
    static PropertyValuesHolder*ofObject(const std::string&propertyName,const std::vector<PathParser::PathData>&);
};

typedef PropertyValuesHolder  IntPropertyValuesHolder;
typedef PropertyValuesHolder  FloatPropertyValuesHolder;

}//endof namespace
#endif/*__PROPERTY_VALUES_HOLDER_H__*/
