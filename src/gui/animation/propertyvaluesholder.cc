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
#include <animation/propertyvaluesholder.h>
#include <porting/cdlog.h>

namespace cdroid{

PropertyValuesHolder::PropertyValuesHolder(){
    mProperty = nullptr;
    mValueType= Property::UNDEFINED;
    mEvaluator= evaluator;
    LOGD("%p,%s",this,mPropertyName.c_str());
}

PropertyValuesHolder::PropertyValuesHolder(const PropertyValuesHolder&o){
    mPropertyName = o.mPropertyName;
    mDataSource = o.mDataSource;
    mAnimateValue= o.mAnimateValue;
    mProperty = o.mProperty;
    mValueType= o.mValueType;
    mEvaluator= o.mEvaluator;
}

PropertyValuesHolder::PropertyValuesHolder(const Property*property){
    mProperty = property;
    mValueType= property->getType();
    mEvaluator= evaluator;
    if(property)mPropertyName = property->getName();
}

PropertyValuesHolder::PropertyValuesHolder(const std::string&name){
    mPropertyName = name;
    mValueType= Property::UNDEFINED;
    mProperty = nullptr;
    mEvaluator= evaluator;
}

PropertyValuesHolder::~PropertyValuesHolder(){
    //delete mProperty;
}

void PropertyValuesHolder::setPropertyName(const std::string& propertyName){
    mPropertyName = propertyName;
}

const std::string PropertyValuesHolder::getPropertyName()const{
    return mPropertyName;
}

void PropertyValuesHolder::setProperty(const Property*p){
    mProperty = p;
    mValueType= p?p->getType():Property::UNDEFINED;
}

const Property*PropertyValuesHolder::getProperty()const{
    return mProperty;
}

int PropertyValuesHolder::getValueType()const{
    return mValueType;
}

void PropertyValuesHolder::setPropertyChangedListener(const OnPropertyChangedListener&ls){
    mOnPropertyChangedListener = ls;
}

void PropertyValuesHolder::setupSetterAndGetter(void*target){
    if(mPropertyName.empty())return;
    if(mProperty==nullptr){
        mProperty = Property::fromName(mPropertyName);
        mValueType= mProperty->getType();
    }
}

static int lerp(int startValue, int endValue, float fraction) {
    return int(startValue + std::round(fraction * (endValue - startValue)));
}

AnimateValue& PropertyValuesHolder::evaluator(float fraction,AnimateValue&out, const AnimateValue& from, const AnimateValue& to){
    switch(from.index()){
    case 0:
        out = (int)((1.f - fraction)*GET_VARIANT(from,int) +  fraction * GET_VARIANT(to,int));
        break;
    case 1:
        out = GET_VARIANT(from,float) * (1.f - fraction) + GET_VARIANT(to,float) * fraction;
        break;
    default:
        LOGE("NOT_REACHED");
    }
    return out;
}

AnimateValue& PropertyValuesHolder::ArgbEvaluator(float fraction,AnimateValue& out,const AnimateValue&from,const AnimateValue&to){
    const uint32_t fromArgb = (uint32_t)GET_VARIANT(from,int32_t);
    const uint32_t toArgb = (uint32_t)GET_VARIANT(to,int32_t);
    const float startA = ((fromArgb >> 24) & 0xff) / 255.0f;
    const float startR = ((fromArgb >> 16) & 0xff) / 255.0f;
    const float startG = ((fromArgb >>  8) & 0xff) / 255.0f;
    const float startB = ( fromArgb        & 0xff) / 255.0f;

    const float endA = ((toArgb >> 24) & 0xff) / 255.0f;
    const float endR = ((toArgb >> 16) & 0xff) / 255.0f;
    const float endG = ((toArgb >>  8) & 0xff) / 255.0f;
    const float endB = ( toArgb        & 0xff) / 255.0f;

    const float a = startA + fraction * (endA - startA);
    const float r = startR + fraction * (endR - startR);
    const float g = startG + fraction * (endG - startG);
    const float b = startB + fraction * (endB - startB);
    uint32_t color = ((uint32_t)(a*255.f)<<24)|((uint32_t)(r*255)<<16)|((uint32_t)(g*255)<<8)|((uint32_t)(b*255));
    out = int32_t(color);
    return out;
}

AnimateValue& PropertyValuesHolder::PathDataEvaluator(float fraction,AnimateValue& out,const AnimateValue&from,const AnimateValue&to){
    auto& fromPathData= GET_VARIANT(from,PathParser::PathData);
    auto& toPathData  = GET_VARIANT(to,PathParser::PathData);
    auto& outPathData = GET_VARIANT(out,PathParser::PathData);
    if (!PathParser::interpolatePathData(outPathData, fromPathData, toPathData, fraction)) {
        throw std::runtime_error("Can't interpolate between two incompatible pathData");
    }
    return out;
}

void PropertyValuesHolder::setValues(const std::vector<int>&values){
    //mDataSource.resize(std::max(values.size(),size_t(2)));
    mDataSource.clear();
    mValueType = Property::INT_TYPE;
    for(size_t i = 0;i < values.size();i++)
       mDataSource.push_back(values.at(i));
    mAnimateValue = values[0];
}

void PropertyValuesHolder::setValues(const std::vector<float>&values){
    //mDataSource.resize(std::max(values.size(),size_t(2)));
    mDataSource.clear();
    mValueType = Property::FLOAT_TYPE;
    for(size_t i = 0;i < values.size();i++)
       mDataSource.push_back(values.at(i));
    mAnimateValue = values[0];
}

void PropertyValuesHolder::setValues(const std::vector<PathParser::PathData>&values){
    mDataSource.clear();
    mValueType = Property::PATH_TYPE;
    for(size_t i = 0;i < values.size();i++)
       mDataSource.push_back(values.at(i));
    mAnimateValue = values[0];
    mEvaluator= PathDataEvaluator;
}

void PropertyValuesHolder::init(){
    if(mEvaluator==nullptr){
        mEvaluator = evaluator;
    }
    mAnimateValue =mDataSource[0];
}

void PropertyValuesHolder::setEvaluator(TypeEvaluator evaluator){
    mEvaluator = evaluator;
}

void PropertyValuesHolder::calculateValue(float fraction){
    if (fraction <= 0.0f) mAnimateValue = mDataSource.front();
    else if (fraction >= 1.0f) mAnimateValue = mDataSource.back();
    else {
        fraction *= mDataSource.size() - 1;
        const int lowIndex = std::floor(fraction);
        fraction -= lowIndex;
        (*mEvaluator)(fraction, mAnimateValue,mDataSource[lowIndex], mDataSource[lowIndex + 1]);
    } 
}

const AnimateValue& PropertyValuesHolder::getAnimatedValue()const{
    return mAnimateValue;
}

void PropertyValuesHolder::getPropertyValues(PropertyValues& values){
    init();
    values.propertyName = mPropertyName;
    values.type = mValueType;
    values.startValue = mDataSource[0];
    values.endValue = mDataSource[mDataSource.size()-1];
    mAnimateValue = mDataSource[0];
#if 0
    values.startValue = mKeyframes.getValue(0);
    if (values.startValue instanceof PathParser::PathData) {
        // PathData evaluator returns the same mutable PathData object when query fraction,
        // so we have to make a copy here.
        values.startValue = new PathParser::PathData((PathParser::PathData) values.startValue);
    }
    values.endValue = mKeyframes.getValue(1);
    if (values.endValue instanceof PathParser::PathData) {
        // PathData evaluator returns the same mutable PathData object when query fraction,
        // so we have to make a copy here.
        values.endValue = new PathParser::PathData((PathParser::PathData) values.endValue);
    }
    // TODO: We need a better way to get data out of keyframes.
    if (mKeyframes instanceof PathKeyframes.FloatKeyframesBase
            || mKeyframes instanceof PathKeyframes.IntKeyframesBase
            || (mKeyframes.getKeyframes() != null && mKeyframes.getKeyframes().size() > 2)) {
        // When a pvh has more than 2 keyframes, that means there are intermediate values in
        // addition to start/end values defined for animators. Another case where such
        // intermediate values are defined is when animator has a path to animate along. In
        // these cases, a data source is needed to capture these intermediate values.
        values.getValueAtFraction=[this](float fraction)->AnimateValue {
            calculateValue(fraction);
            return mAnimateValue;
            //return mKeyframes.getValue(fraction);
        };
    } else {
        values.dataSource = nullptr;
    }
#endif
}

void PropertyValuesHolder::setAnimatedValue(void*target){
    if(mProperty!=nullptr){
        mProperty->set(target,getAnimatedValue());
    }else if(mSetter!=0){
        AnimateValue value = getAnimatedValue();
        mSetter(target,mPropertyName,value);
    }
}

void PropertyValuesHolder::setupValue(void*target,int position){
    if(mProperty){
        AnimateValue value = mProperty->get(target);
        if(mDataSource.size()==1)
            mDataSource.insert(mDataSource.begin()+position,value);
        else
            mDataSource[position] = value;
    }else if(mGetter){
        AnimateValue value = mGetter(target,mPropertyName);
        if(mDataSource.size()==1)
            mDataSource.insert(mDataSource.begin()+position,value);
        else
            mDataSource[position] = value;
    }
}

void PropertyValuesHolder::setupStartValue(void*target){
    if(!mDataSource.empty()){
        setupValue(target,0);
    }
}

void PropertyValuesHolder::setupEndValue(void*target){
    if(!mDataSource.empty()){
        setupValue(target,mDataSource.size()-1);
    }
}

PropertyValuesHolder* PropertyValuesHolder::ofInt(const std::string&name,const std::vector<int>&values){
    PropertyValuesHolder*pvh = new PropertyValuesHolder(name);
    pvh->setValues(values);
    return pvh;
}

PropertyValuesHolder* PropertyValuesHolder::ofInt(const Property*prop,const std::vector<int>&values){
    PropertyValuesHolder*pvh = new PropertyValuesHolder(prop);
    pvh->setValues(values);
    return pvh;
}

PropertyValuesHolder* PropertyValuesHolder::ofFloat(const std::string&name,const std::vector<float>&values){
    PropertyValuesHolder*pvh = new PropertyValuesHolder(name);
    pvh->setValues(values);
    return pvh;
}

PropertyValuesHolder* PropertyValuesHolder::ofFloat(const Property*prop,const std::vector<float>&values){
    PropertyValuesHolder*pvh = new PropertyValuesHolder(prop);
    pvh->setValues(values);
    return pvh;
}

PropertyValuesHolder*PropertyValuesHolder::ofObject(const std::string&propertyName,const std::vector<void*>&values){
    PropertyValuesHolder*pvh = new PropertyValuesHolder(propertyName);
    //pvh->setValues(values);
    return pvh;
}

PropertyValuesHolder*PropertyValuesHolder::ofObject(const Property*prop,const std::vector<PathParser::PathData>&values){
    PropertyValuesHolder*pvh = new PropertyValuesHolder(prop);
    pvh->setValues(values);
    return pvh;
}

PropertyValuesHolder*PropertyValuesHolder::ofObject(const std::string&propertyName,const std::vector<PathParser::PathData>&values){
    PropertyValuesHolder*pvh = new PropertyValuesHolder(propertyName);
    pvh->setValues(values);
    return pvh;
}

}//endof namespace
