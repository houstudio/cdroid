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
#include <animation/typeevaluators.h>   // PointFEvaluator
#include <core/pathmeasure.h>           // Path sampling for ofPointF

namespace cdroid{

PropertyValuesHolder::PropertyValuesHolder(){
    mProperty = nullptr;
    mKeyframes = nullptr;
    mValueType= Property::UNDEFINED;
    mEvaluator= evaluator;
    LOGD("%p,%s",this,mPropertyName.c_str());
}

PropertyValuesHolder::PropertyValuesHolder(const PropertyValuesHolder&o){
    mPropertyName = o.mPropertyName;
    mKeyframes = o.mKeyframes ? o.mKeyframes->clone() : nullptr;   // AOSP clone(): mKeyframes.clone()
    mAnimateValue= o.mAnimateValue;
    mProperty = o.mProperty;
    mValueType= o.mValueType;
    mEvaluator= o.mEvaluator;
}

PropertyValuesHolder::PropertyValuesHolder(const Property*property){
    mProperty = property;
    mKeyframes = nullptr;
    mValueType= property->getType();
    mEvaluator= evaluator;
    if(property)mPropertyName = property->getName();
}

PropertyValuesHolder::PropertyValuesHolder(const std::string&name){
    mPropertyName = name;
    mValueType= Property::UNDEFINED;
    mProperty = nullptr;
    mKeyframes = nullptr;
    mEvaluator= evaluator;
}

PropertyValuesHolder::~PropertyValuesHolder(){
    //delete mProperty;
    delete mKeyframes;
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

AnimateValue& PropertyValuesHolder::evaluator(float fraction,AnimateValue&out, const AnimateValue& from, const AnimateValue& to){
#if VARIANT_AS_ANIMATEDVAUE
    // Mixed int/float endpoints (property getter float vs XML int literal) lerp
    // in float — numeric promotion instead of bad_variant_access.
    const bool eitherFloat = (from.index() == 1) || (to.index() == 1);
    if (eitherFloat) {
        const float f = (from.index() == 0) ? (float)GET_VARIANT(from,int) : GET_VARIANT(from,float);
        const float t = (to.index()   == 0) ? (float)GET_VARIANT(to,int)   : GET_VARIANT(to,float);
        out = f * (1.f - fraction) + t * fraction;
    } else switch(from.index()){
    case 0:
        out = (int)((1.f - fraction)*GET_VARIANT(from,int) +  fraction * GET_VARIANT(to,int));
        break;
    case 1:
        out = GET_VARIANT(from,float) * (1.f - fraction) + GET_VARIANT(to,float) * fraction;
        break;
    default:
        LOGE("NOT_REACHED");
    }
#else
    if(from.type()==typeid(int)){
        out = (int)((1.f - fraction)*GET_VARIANT(from,int) +  fraction * GET_VARIANT(to,int));
    }else if(from.type()==typeid(float)){
        out = GET_VARIANT(from,float) * (1.f - fraction) + GET_VARIANT(to,float) * fraction;
    }
#endif
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
#if VARIANT_AS_ANIMATEDVAUE
    auto& fromPathData= GET_VARIANT(from,PathParser::PathData);
    auto& toPathData  = GET_VARIANT(to,PathParser::PathData);
    auto& outPathData = GET_VARIANT(out,PathParser::PathData);
    if (!PathParser::interpolatePathData(outPathData, fromPathData, toPathData, fraction)) {
        throw std::runtime_error("Can't interpolate between two incompatible pathData");
    }
#else
    const auto fromPathData = GET_VARIANT(&from,const PathParser::PathData);
    const auto toPathData = GET_VARIANT(&to,const PathParser::PathData);
    auto outPathData = GET_VARIANT(&out,PathParser::PathData);
    if (!PathParser::interpolatePathData(*outPathData, *fromPathData, *toPathData, fraction)) {
        throw std::runtime_error("Can't interpolate between two incompatible pathData");
    }
#endif
    return out;
}

void PropertyValuesHolder::setValues(const std::vector<int>&values){
    delete mKeyframes;
    mKeyframes = KeyframeSet::ofInt(values);
    mValueType = Property::INT_TYPE;
    if(!values.empty()) mAnimateValue = values[0];
}

void PropertyValuesHolder::setValues(const std::vector<float>&values){
    delete mKeyframes;
    mKeyframes = KeyframeSet::ofFloat(values);
    mValueType = Property::FLOAT_TYPE;
    if(!values.empty()) mAnimateValue = values[0];
}

void PropertyValuesHolder::setValues(const std::vector<PathParser::PathData>&values){
    std::vector<AnimateValue> objectValues;
    for(const auto&v:values)objectValues.push_back(v);
    delete mKeyframes;
    mKeyframes = KeyframeSet::ofObject(objectValues);
    mValueType = Property::PATH_TYPE;
    if(!values.empty()) mAnimateValue = values[0];
    mEvaluator= PathDataEvaluator;
}

void PropertyValuesHolder::init(){
    if(mEvaluator==nullptr){
        mEvaluator = evaluator;
    }
    // AOSP PHV.init(): hand the evaluator to the keyframes (they evaluate).
    if(mKeyframes) mKeyframes->setEvaluator(mEvaluator);
}

void PropertyValuesHolder::setEvaluator(TypeEvaluator evaluator){
    mEvaluator = evaluator;
    if(mKeyframes) mKeyframes->setEvaluator(mEvaluator);
}

void PropertyValuesHolder::calculateValue(float fraction){
    mAnimateValue = mKeyframes->getValue(fraction);
}

const AnimateValue& PropertyValuesHolder::getAnimatedValue()const{
    return mAnimateValue;
}

void PropertyValuesHolder::getPropertyValues(PropertyValues& values){
    init();
    values.propertyName = mPropertyName;
    values.type = mValueType;
    // AOSP: mKeyframes.getValue(0)/getValue(1) (copy PathData out — its
    // evaluator returns the same mutable object).
    values.startValue = mKeyframes->getValue(0.f);
    if(values.startValue.index()==2/*PathData*/)
        values.startValue = GET_VARIANT(values.startValue,PathParser::PathData);
    values.endValue = mKeyframes->getValue(1.f);
    if(values.endValue.index()==2/*PathData*/)
        values.endValue = GET_VARIANT(values.endValue,PathParser::PathData);
    mAnimateValue = values.startValue;
    // AOSP sets a dataSource closure when the holder carries intermediate
    // values (>2 keyframes / path-sampled). CDROID's AnimatedVectorDrawable
    // consumers do not act on it yet, so it stays unset here (dormant, as
    // before the keyframes migration).
}

void PropertyValuesHolder::setAnimatedValue(void*target){
    if(mProperty!=nullptr){
        AnimateValue value = getAnimatedValue();
        // Numeric coercion at the property seam: an XML int literal ("55") on a
        // float-typed property (pivotY etc.; AOSP generics would throw) crosses
        // over instead of raising bad_variant_access, and float→int likewise.
        if (mProperty->getType() == Property::FLOAT_TYPE && value.index() == 0/*int*/) {
            value = (float)GET_VARIANT(value,int);
        } else if (mProperty->getType() == Property::INT_TYPE && value.index() == 1/*float*/) {
            value = (int)GET_VARIANT(value,float);
        }
        mProperty->set(target,value);
    }else if(mSetter!=0){
        AnimateValue value = getAnimatedValue();
        mSetter(target,mPropertyName,value);
    }
}

// AOSP setupValue: only fill keyframes that carry no value — XML-provided
// start/end values win; path-sampled keyframes are never overwritten by a
// property getter either (this replaces the old mPathBased guard).
void PropertyValuesHolder::setupValue(void*target,int position){
    Keyframe*keyframe = mKeyframes->getKeyframes()[position];
    if(keyframe->hasValue())return;
    if(mProperty){
        keyframe->setValue(mProperty->get(target));
    }else if(mGetter){
        keyframe->setValue(mGetter(target,mPropertyName));
    }
}

// AOSP: an empty keyframe list (PathKeyframes projections) means the values
// are fully defined by the path — start/end setup is skipped entirely.
void PropertyValuesHolder::setupStartValue(void*target){
    std::vector<Keyframe*>& keyframes = mKeyframes->getKeyframes();
    if (!keyframes.empty()) {
        setupValue(target,0);
    }
}

void PropertyValuesHolder::setupEndValue(void*target){
    std::vector<Keyframe*>& keyframes = mKeyframes->getKeyframes();
    if (!keyframes.empty()) {
        setupValue(target,keyframes.size()-1);
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

PropertyValuesHolder* PropertyValuesHolder::ofKeyframes(const std::string&name,const std::vector<Keyframe*>&keyframes){
    PropertyValuesHolder*pvh = new PropertyValuesHolder(name);
    delete pvh->mKeyframes;
    pvh->mKeyframes = KeyframeSet::ofKeyframe(keyframes);
    pvh->mValueType = pvh->mKeyframes->getType();
    pvh->init();
    return pvh;
}

PropertyValuesHolder* PropertyValuesHolder::ofKeyframes(const Property*prop,const std::vector<Keyframe*>&keyframes){
    PropertyValuesHolder*pvh = new PropertyValuesHolder(prop);
    delete pvh->mKeyframes;
    pvh->mKeyframes = KeyframeSet::ofKeyframe(keyframes);
    pvh->init();
    return pvh;
}

PropertyValuesHolder*PropertyValuesHolder::ofKeyframes(const std::string&name,Keyframes*keyframes){
    PropertyValuesHolder*pvh = new PropertyValuesHolder(name);
    delete pvh->mKeyframes;
    pvh->mKeyframes = keyframes;
    pvh->mValueType = keyframes->getType();
    pvh->init();
    return pvh;
}

PropertyValuesHolder*PropertyValuesHolder::ofKeyframes(const Property*prop,Keyframes*keyframes){
    PropertyValuesHolder*pvh = new PropertyValuesHolder(prop);
    delete pvh->mKeyframes;
    pvh->mKeyframes = keyframes;
    pvh->init();
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

// Generic ofObject: caller supplies the evaluator matching the AnimateValue type (Rect/PointF/...).
PropertyValuesHolder*PropertyValuesHolder::ofObject(const Property*prop,TypeEvaluator evaluator,const std::vector<AnimateValue>&values){
    PropertyValuesHolder*pvh = new PropertyValuesHolder(prop);
    delete pvh->mKeyframes;
    pvh->mKeyframes = KeyframeSet::ofObject(values);
    pvh->mEvaluator = evaluator;
    pvh->mAnimateValue = values.front();
    return pvh;
}

// Sample the Path into N+1 PointF keyframes; PointFEvaluator interpolates between neighbours.
// (AOSP uses PathKeyframes with an error-bounded sampling; CDROID keeps the
// uniform N-sample scheme, now expressed as keyframes with values — so
// setupStartValue/setupEndValue never overwrite them, matching AOSP's
// hasValue() behavior without the old mPathBased guard.)
PropertyValuesHolder*PropertyValuesHolder::ofPointF(const Property*prop,const Cairo::RefPtr<cdroid::Path>& path){
    PropertyValuesHolder*pvh = new PropertyValuesHolder(prop);
    PathMeasure measure(path, false);
    const double length = measure.getLength();
    const int N = 32; // fine enough that linear segments approximate curved paths
    std::vector<AnimateValue> points;
    for (int i = 0; i <= N; i++) {
        double pos[2] = {0,0}, tan[2] = {0,0};
        measure.getPosTan(length * i / N, pos, tan);
        PointF p; p.x = (float)pos[0]; p.y = (float)pos[1];
        points.push_back(p);
    }
    pvh->mKeyframes = KeyframeSet::ofObject(points);
    pvh->mEvaluator = PointFEvaluator;
    pvh->mAnimateValue = points.front();
    return pvh;
}

}//endof namespace
