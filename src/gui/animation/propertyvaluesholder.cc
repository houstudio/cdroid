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
    mProperty = nullptr;
}

PropertyValuesHolder::PropertyValuesHolder(Property*prop){
    mProperty = prop;
}

PropertyValuesHolder::PropertyValuesHolder(const std::string&name){
    mPropertyName = name;
    mProperty = Property::propertyFromName(name);;
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
       mDataSource[i].emplace<int>(values.at(i));
    if(values.size()==1)
        mDataSource[1].emplace<int>(values[0]);
}

void PropertyValuesHolder::setValues(const std::vector<uint32_t>&values){
    mDataSource.resize(std::max(values.size(),size_t(2)));
    mDataSource.clear();
    for(size_t i = 0;i < values.size();i++)
       mDataSource[i].emplace<uint32_t>(values.at(i));
    if(values.size()==1)
        mDataSource[1].emplace<uint32_t>(values[0]);
}

void PropertyValuesHolder::setValues(const std::vector<float>&values){
    mDataSource.resize(std::max(values.size(),size_t(2)));
    mDataSource.clear();
    for(size_t i = 0;i < values.size();i++)
       mDataSource[i].emplace<float>(values.at(i));
    if(values.size()==1)
        mDataSource[1].emplace<float>(values[0]);
}

void PropertyValuesHolder::init(){
    if(mEvaluator==nullptr){
        mEvaluator = evaluator;
    }
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
#if 0
void PropertyValuesHolder::setFraction(void*target,float fraction){
    if (mDataSource.size()==0) mAnimateValue=mEvaluator(fraction,mStartValue, mEndValue);
    else if (fraction <= 0.0f) mAnimateValue=mDataSource.front();
    else if (fraction >= 1.0f) mAnimateValue=mDataSource.back();
    else{
        fraction *= mDataSource.size() - 1;
        int lowIndex = std::floor(fraction);
        fraction -= lowIndex;
        mAnimateValue = mEvaluator(fraction, mDataSource[lowIndex], mDataSource[lowIndex + 1]);
    }
    if(mOnPropertyChangedListener)mOnPropertyChangedListener(mPropertyName,target,fraction);
}
#endif
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
        mDataSource[position] = value;
    }else if(mGetter){
        AnimateValue value = mGetter(target,mPropertyName);
        mDataSource[position] = value;
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
