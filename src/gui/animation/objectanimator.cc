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
#include <stdarg.h>
#include <porting/cdlog.h>
#include <animation/objectanimator.h>

namespace cdroid{

ObjectAnimator::ObjectAnimator():ValueAnimator(){
    mTarget = nullptr;
    mProperty = nullptr;
    mAutoCancel = false;
}

ObjectAnimator::ObjectAnimator(const ObjectAnimator&anim):ValueAnimator(anim){
    mTarget = anim.mTarget;
    mProperty = anim.mProperty;
    mAutoCancel = anim.mAutoCancel;
}

ObjectAnimator::~ObjectAnimator(){
    //delete mProperty;
}

ObjectAnimator::ObjectAnimator(void* target,const std::string& propertyName)
  :ObjectAnimator(){
    setTarget(target);
    setPropertyName(propertyName);
}

void ObjectAnimator::initAnimation(){
    if(!mInitialized){
        void* target = getTarget();
        for(auto value:mValues){
            value->setupSetterAndGetter(target);
            value->setupStartValue(target);
        }
        ValueAnimator::initAnimation();
    }
}

ObjectAnimator&ObjectAnimator::setDuration(int64_t duration){
    ValueAnimator::setDuration(duration);
    return *this;
}

bool ObjectAnimator::isInitialized(){
    return mInitialized;
}

void ObjectAnimator::setTarget(void*target){
    const void*oldTarget = getTarget();
    if(oldTarget!=target){
        if(isStarted()){
            cancel();
        }
        mTarget = target;
        mInitialized=false;
    }
}

void* ObjectAnimator::getTarget(){
    return mTarget;
}

void ObjectAnimator::setPropertyName(const std::string& propertyName){
    if(mValuesMap.size()){
        PropertyValuesHolder* valuesHolder = mValues.at(0);
        const std::string oldName = valuesHolder->getPropertyName();
        valuesHolder->setPropertyName(propertyName);
        auto it2= mValuesMap.find(oldName);
        if(it2!=mValuesMap.end())
            mValuesMap.erase(it2);
        mValuesMap.insert({propertyName,valuesHolder});
    }
    mPropertyName= propertyName;
    mInitialized = false;
}

void ObjectAnimator::setProperty(const Property* property){
    if (!mValues.empty()) {
        PropertyValuesHolder* valuesHolder = mValues[0];
        const std::string oldName = valuesHolder->getPropertyName();
        valuesHolder->setProperty(property);
        auto it = mValuesMap.find(oldName);
        if(it != mValuesMap.end()){
            mValuesMap.erase(it);
        }
        mValuesMap.insert(std::unordered_map<std::string,PropertyValuesHolder*>::value_type(mPropertyName,valuesHolder));
    }
    if (mProperty != nullptr) {
        mPropertyName = property->getName();
    }
    mProperty = property;
    // New property/values/target should cause re-initialization prior to starting
    mInitialized = false; 
}

const std::string ObjectAnimator::getPropertyName()const{
    std::string propertyName;
    if(!mPropertyName.empty())
        return mPropertyName;
    else if(mProperty){
        return mProperty->getName();
    }else if(mValues.size()){
        for(auto v:mValues){
            if(!propertyName.empty())
               propertyName+=",";
            propertyName+=v->getPropertyName();
        }
    }
    return propertyName;
}

void ObjectAnimator::setIntValues(const std::vector<int>&values){
    if (mValues.size() == 0) {
        // No values yet - this animator is being constructed piecemeal. Init the values with
        // whatever the current propertyName is
        if (mProperty) {
            setValues({PropertyValuesHolder::ofInt(mProperty, values)});
        } else {
            setValues({PropertyValuesHolder::ofInt(mPropertyName, values)});
        }
    } else {
        ValueAnimator::setIntValues(values);
    }
}

void ObjectAnimator::setFloatValues(const std::vector<float>&values){
    if (mValues.size() == 0) {
        // No values yet - this animator is being constructed piecemeal. Init the values with
        // whatever the current propertyName is
        if (mProperty) {
            setValues({PropertyValuesHolder::ofFloat(mProperty, values)});
        } else {
            setValues({PropertyValuesHolder::ofFloat(mPropertyName, values)});
        }
    } else {
        ValueAnimator::setFloatValues(values);
    }
}

void ObjectAnimator::setAutoCancel(bool cancel){
    mAutoCancel = cancel;
}

bool ObjectAnimator::hasSameTargetAndProperties(const Animator*anim){
    if (dynamic_cast<const ObjectAnimator*>(anim)) {
        std::vector<PropertyValuesHolder*>&theirValues = ((ObjectAnimator*) anim)->getValues();
        if (((ObjectAnimator*) anim)->getTarget() == getTarget() && mValues.size() == theirValues.size()) {
            for (int i = 0; i < mValues.size(); ++i) {
                PropertyValuesHolder* pvhMine = mValues[i];
                PropertyValuesHolder* pvhTheirs = theirValues[i];
                if (pvhMine->getPropertyName().empty() ||
                    pvhMine->getPropertyName()!=pvhTheirs->getPropertyName()) {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

void ObjectAnimator::start(){
    AnimationHandler::getInstance().autoCancelBasedOn(this);
    ValueAnimator::start();
}

bool ObjectAnimator::shouldAutoCancel(const AnimationHandler::AnimationFrameCallback*anim){
    if (anim == nullptr) {
        return false;
    }

    if (dynamic_cast<const ObjectAnimator*>(anim)) {
         ObjectAnimator* objAnim = (ObjectAnimator*) anim;
         if (objAnim->mAutoCancel && hasSameTargetAndProperties(objAnim)) {
             return true;
         }
    }
    return false;    
}

void ObjectAnimator::setupStartValues() {
    initAnimation();

    void* target = getTarget();
    if (target != nullptr) {
        size_t numValues = mValues.size();
        for (size_t i = 0; i < numValues; ++i) {
            mValues[i]->setupStartValue(target);
        }
    }
}

void ObjectAnimator::setupEndValues() {
    initAnimation();
    void* target = getTarget();
    if (target != nullptr) {
        size_t numValues = mValues.size();
        for (size_t i = 0; i < numValues; ++i) {
            mValues[i]->setupEndValue(target);
        }
    }
}

void ObjectAnimator::animateValue(float fraction){
    void* target = getTarget();
    if ((mTarget != nullptr) && (target == nullptr)) {
        // We lost the target reference, cancel and clean up. Note: we allow null target if the
        /// target has never been set.
        cancel();
        return;
    }

    ValueAnimator::animateValue(fraction);
    size_t numValues = mValues.size();
    for (size_t i = 0; i < numValues; ++i) {
        mValues[i]->setAnimatedValue(target);
    }
}

ObjectAnimator*ObjectAnimator::clone()const{
    ObjectAnimator*anim = new ObjectAnimator(*this);
    return anim;
}

ObjectAnimator* ObjectAnimator::ofInt(void* target,const std::string& propertyName,const std::vector<int>&values){
    ObjectAnimator*anim = new ObjectAnimator(target,propertyName);
    anim->setIntValues(values);
    return anim;
}

ObjectAnimator* ObjectAnimator::ofFloat(void* target,const std::string& propertyName, const std::vector<float>&values){
    ObjectAnimator*anim = new ObjectAnimator(target,propertyName);
    anim->setFloatValues(values);
    return anim;
}

ObjectAnimator* ObjectAnimator::ofPropertyValuesHolder(void*target,const std::vector<PropertyValuesHolder*>&values){
    ObjectAnimator*anim = new ObjectAnimator();
    anim->setTarget(target);
    anim->setValues(values);
    return anim;
}

ObjectAnimator* ObjectAnimator::ofInt(void*target,const Property*prop,const std::vector<int>&values){
    ObjectAnimator*anim = new ObjectAnimator();
    anim->setTarget(target);
    anim->setProperty(prop);
    anim->setIntValues(values);
    return anim;
}

ObjectAnimator* ObjectAnimator::ofFloat(void*target,const Property*prop,const std::vector<float>&values){
    ObjectAnimator*anim = new ObjectAnimator();
    anim->setTarget(target);
    anim->setProperty(prop);
    anim->setFloatValues(values);
    return anim;
}

std::string ObjectAnimator::toString()const{
    std::string str =std::string("ObjectAnimator@")+std::to_string((long)this);
    str+=":"+mPropertyName;
    return str;
}

}
