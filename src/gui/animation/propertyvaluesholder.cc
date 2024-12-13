#include <animation/propertyvaluesholder.h>
#include <porting/cdlog.h>

namespace cdroid{

PropertyValuesHolder::PropertyValuesHolder(){
    mProperty = nullptr;
}

PropertyValuesHolder::PropertyValuesHolder(const PropertyValuesHolder&o){
    mPropertyName = o.mPropertyName;
    mDataSource = o.mDataSource;
    mStartValue = o.mStartValue;
    mEndValue = o.mEndValue;
    mAnimateValue= o.mAnimateValue;
    mProperty = o.mProperty;
}

PropertyValuesHolder::PropertyValuesHolder(Property*property){
    mProperty = property;
    if(property)mPropertyName = property->getName();
}

PropertyValuesHolder::PropertyValuesHolder(const std::string&name){
    mPropertyName = name;
    mProperty = Property::fromName(name);
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

void PropertyValuesHolder::setProperty(Property*p){
    mProperty = p;
}

Property*PropertyValuesHolder::getProperty(){
    return mProperty;
}

void PropertyValuesHolder::setPropertyChangedListener(const OnPropertyChangedListener&ls){
    mOnPropertyChangedListener = ls;
}

void PropertyValuesHolder::setupSetterAndGetter(void*target){
    Property*prop = nullptr;
    if(mPropertyName.empty())return;
    prop = Property::fromName(mPropertyName);
    if(prop&&(mGetter==nullptr)){

    }
}

AnimateValue PropertyValuesHolder::evaluator(float fraction, const AnimateValue& from, const AnimateValue& to){
    AnimateValue out;
    switch(from.index()){
    case 0:
        out = (int)((1.f - fraction)*GET_VARIANT(from,int) +  fraction * GET_VARIANT(to,int));
        break;
    case 1:{
        float a = lerp((GET_VARIANT(from,uint32_t)>>24)/255.f,(GET_VARIANT(to,uint32_t)>>24)/255.f,fraction);
        float r = lerp(((GET_VARIANT(from,uint32_t)>>16)&0xFF)/255.f,((GET_VARIANT(to,uint32_t)>>16)&0xFF)/255.f,fraction);
        float g = lerp(((GET_VARIANT(from,uint32_t)>>8)&0xFF)/255.f,((GET_VARIANT(to,uint32_t)>>8)&0xFF)/255.f,fraction);
        float b = lerp((GET_VARIANT(from,uint32_t)&0xFF)/255.f,(GET_VARIANT(to,uint32_t)&0xFF)/255.f,fraction);
        out = ((uint32_t)(a*255.f)<<24)|((uint32_t)(r*255)<<16)|((uint32_t)(g*255)<<8)|((uint32_t)(b*255));
        }break;
    case 2:
        out = GET_VARIANT(from,float) * (1.f - fraction) + GET_VARIANT(to,float) * fraction;
        break;
    default:
        LOGE("NOT_REACHED");
    }
    return out;
}

void PropertyValuesHolder::setValues(const std::vector<int>&values){
    mDataSource.resize(std::max(values.size(),size_t(2)));
    mDataSource.clear();
    for(size_t i=0;i<values.size();i++)
       mDataSource.push_back(values.at(i));
    mAnimateValue = values[0];
    mStartValue= mDataSource[0];
    mEndValue  = mDataSource[mDataSource.size()-1];
}

void PropertyValuesHolder::setValues(const std::vector<uint32_t>&values){
    mDataSource.resize(std::max(values.size(),size_t(2)));
    mDataSource.clear();
    for(size_t i = 0;i < values.size();i++)
       mDataSource.push_back(values.at(i));
    mAnimateValue = values[0];
}

void PropertyValuesHolder::setValues(const std::vector<float>&values){
    mDataSource.resize(std::max(values.size(),size_t(2)));
    mDataSource.clear();
    for(size_t i = 0;i < values.size();i++)
       mDataSource.push_back(values.at(i));
    mAnimateValue = values[0];
    mStartValue= mDataSource[0];
    mEndValue  = mDataSource[mDataSource.size()-1];
}

void PropertyValuesHolder::init(){
    if(mEvaluator==nullptr){
        mEvaluator = evaluator;
    }
    mAnimateValue =mDataSource[0];
    mStartValue= mDataSource[0];
    mEndValue  = mDataSource[mDataSource.size()-1];
}

void PropertyValuesHolder::setEvaluator(TypeEvaluator evaluator){
    mEvaluator = evaluator;
}

void PropertyValuesHolder::calculateValue(float fraction){
    if (mDataSource.size()==0) mAnimateValue=mEvaluator(fraction,mStartValue, mEndValue);
    else if (fraction <= 0.0f) mAnimateValue=mDataSource.front();
    else if (fraction >= 1.0f) mAnimateValue=mDataSource.back();
    else{
        fraction *= mDataSource.size() - 1;
        int lowIndex = std::floor(fraction);
        fraction -= lowIndex;
        mAnimateValue = mEvaluator(fraction, mDataSource[lowIndex], mDataSource[lowIndex + 1]);
    } 
}

const AnimateValue& PropertyValuesHolder::getAnimatedValue()const{
    return mAnimateValue;
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
        /*else
            mDataSource[position] = value;*/
    }else if(mGetter){
        AnimateValue value = mGetter(target,mPropertyName);
        if(mDataSource.size()==1)
            mDataSource.insert(mDataSource.begin()+position,value);
        /*else
            mDataSource[position] = value;*/
    }else{
        LOGE("invalidate arguments mGetter,property must be setted");
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

PropertyValuesHolder* PropertyValuesHolder::ofInt(Property*prop,const std::vector<int>&values){
    PropertyValuesHolder*pch = new PropertyValuesHolder(prop);
    pch->setValues(values);
    return pch;
}

PropertyValuesHolder* PropertyValuesHolder::ofFloat(const std::string&name,const std::vector<float>&values){
    PropertyValuesHolder*pvh = new PropertyValuesHolder(name);
    pvh->setValues(values);
    return pvh;
}

PropertyValuesHolder* PropertyValuesHolder::ofFloat(Property*prop,const std::vector<float>&values){
    PropertyValuesHolder*pvh = new PropertyValuesHolder(prop);
    pvh->setValues(values);
    return pvh;
}

PropertyValuesHolder*PropertyValuesHolder::ofObject(const std::string&propertyName,const std::vector<void*>&values){
    PropertyValuesHolder*pvh = new PropertyValuesHolder(propertyName);
    return pvh;
}

}//endof namespace
