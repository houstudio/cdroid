#include <animation/propertyvaluesholder.h>
namespace cdroid{

PropertyValuesHolder::PropertyValuesHolder(){
    mProperty=nullptr;
}

PropertyValuesHolder::PropertyValuesHolder(Property*prop){
    mProperty=prop;
}

PropertyValuesHolder::PropertyValuesHolder(const std::string&name){
    mPropertyName=name;
    mProperty=nullptr;
}

PropertyValuesHolder::~PropertyValuesHolder(){
    delete mProperty;
}

void PropertyValuesHolder::setPropertyName(const std::string& propertyName){
    mPropertyName=propertyName;
}

const std::string PropertyValuesHolder::getPropertyName()const{
    return mPropertyName;
}

void PropertyValuesHolder::setProperty(Property*p){
    mProperty =p;
}

Property*PropertyValuesHolder::getProperty(){
    return mProperty;
}

void PropertyValuesHolder::evaluate(Variant& out, const Variant& from, const Variant& to,
           float fraction) const{
    switch(from.index()){
    case 0:
        out = (1.f - fraction)*from.get<int>() +  fraction * to.get<int>();
        break;
    case 1:{
        float a=lerp((from.get<uint32_t>()>>24)/255.f,(to.get<uint32_t>()>>24)/255.f,fraction);
        float r=lerp(((from.get<uint32_t>()>>16)&0xFF)/255.f,((to.get<uint32_t>()>>16)&0xFF)/255.f,fraction);
        float g=lerp(((from.get<uint32_t>()>>8)&0xFF)/255.f,((to.get<uint32_t>()>>8)&0xFF)/255.f,fraction);
        float b=lerp((from.get<uint32_t>()&0xFF)/255.f,(to.get<uint32_t>()&0xFF)/255.f,fraction);
        out=((uint32_t)(a*255.f)<<24)|((uint32_t)(r*255)<<16)|((uint32_t)(g*255)<<8)|((uint32_t)(b*255));
        }break;
    case 2:
        out = from.get<float>() * (1 - fraction) + to.get<float>() * fraction;
        break;
    }
}


void PropertyValuesHolder::setValues(const std::vector<int>&values){
    mDataSource.resize(values.size());
    for(size_t i=0;i<values.size();i++)
       mDataSource[i].emplace<int>(values.at(i));
}

void PropertyValuesHolder::setValues(const std::vector<uint32_t>&values){
    mDataSource.resize(values.size());
    for(size_t i=0;i<values.size();i++)
       mDataSource[i].emplace<uint32_t>(values.at(i));
}

void PropertyValuesHolder::setValues(const std::vector<float>&values){
    mDataSource.resize(values.size());
    for(size_t i=0;i<values.size();i++)
       mDataSource[i].emplace<float>(values.at(i));
}

void PropertyValuesHolder::setFraction(void*target,float fraction){
    if (mDataSource.size()==0) evaluate(mAnimateValue, mStartValue, mEndValue, fraction);
    else if (fraction <= 0.0f) mAnimateValue=mDataSource.front();
    else if (fraction >= 1.0f) mAnimateValue=mDataSource.back();
    else{
        fraction *= mDataSource.size() - 1;
        int lowIndex = std::floor(fraction);
        fraction -= lowIndex;
        evaluate(mAnimateValue, mDataSource[lowIndex], mDataSource[lowIndex + 1], fraction);
    }
    if(mProperty)mProperty->set(target,fraction);
}
const Variant& PropertyValuesHolder::getAnimatedValue()const{
    return mAnimateValue;
}

PropertyValuesHolder* PropertyValuesHolder::ofInt(const std::string&name,const std::vector<int>&values){
    PropertyValuesHolder*ip=new PropertyValuesHolder(name);
    ip->setValues(values);
    return ip;
}

PropertyValuesHolder* PropertyValuesHolder::ofInt(Property*prop,const std::vector<int>&values){
    PropertyValuesHolder*ip=new PropertyValuesHolder(prop);
    ip->setValues(values);
    return ip; 
}

PropertyValuesHolder* PropertyValuesHolder::ofFloat(const std::string&name,const std::vector<float>&values){
    PropertyValuesHolder*fp=new PropertyValuesHolder(name);
    fp->setValues(values);
    return fp;
}

PropertyValuesHolder* PropertyValuesHolder::ofFloat(Property*prop,const std::vector<float>&values){
    PropertyValuesHolder*fp=new PropertyValuesHolder(prop);
    fp->setValues(values);
    return fp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}//endof namespace
