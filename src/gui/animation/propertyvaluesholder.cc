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

PropertyValuesHolder* PropertyValuesHolder::ofInt(const std::string&name,const std::vector<int>&values){
    IntPropertyValuesHolder*ip=new IntPropertyValuesHolder(name);
    ip->setValues(values);
    return ip;
}

PropertyValuesHolder* PropertyValuesHolder::ofInt(Property*prop,const std::vector<int>&values){
    IntPropertyValuesHolder*ip=new IntPropertyValuesHolder(prop);
    ip->setValues(values);
    return ip; 
}

PropertyValuesHolder* PropertyValuesHolder::ofFloat(const std::string&name,const std::vector<float>&values){
    FloatPropertyValuesHolder*fp=new FloatPropertyValuesHolder(name);
    fp->setValues(values);
    return fp;
}

PropertyValuesHolder* PropertyValuesHolder::ofFloat(Property*prop,const std::vector<float>&values){
    FloatPropertyValuesHolder*fp=new FloatPropertyValuesHolder(prop);
    fp->setValues(values);
    return fp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IntPropertyValuesHolder::IntPropertyValuesHolder():PropertyValuesHolderImpl<int>(){
    mEvaluator=new IntEvaluator();
}

IntPropertyValuesHolder::IntPropertyValuesHolder(const std::string&name):PropertyValuesHolderImpl<int>(name){
    mEvaluator=new IntEvaluator();
}

IntPropertyValuesHolder::IntPropertyValuesHolder(Property*prop):PropertyValuesHolderImpl<int>(prop){
    mEvaluator=new IntEvaluator();
}

FloatPropertyValuesHolder::FloatPropertyValuesHolder():PropertyValuesHolderImpl<float>(){
    mEvaluator=new FloatEvaluator();
}

FloatPropertyValuesHolder::FloatPropertyValuesHolder(const std::string&name):PropertyValuesHolderImpl<float>(name){
    mEvaluator=new FloatEvaluator();
}

FloatPropertyValuesHolder::FloatPropertyValuesHolder(Property*prop):PropertyValuesHolderImpl<float>(prop){
    mEvaluator=new FloatEvaluator();
}


}//endof namespace
