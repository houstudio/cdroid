#include <animation/propertyvaluesholder.h>
namespace cdroid{

std::map<const std::string,std::map<const std::string,PropertyValuesHolder::PropertyGetter>>PropertyValuesHolder::sGetterPropertyMap;
std::map<const std::string,std::map<const std::string,PropertyValuesHolder::PropertySetter>>PropertyValuesHolder::sSetterPropertyMap;

static float FloatEvaluator(float fraction, float startValue, float endValue) {
     return startValue + fraction * (endValue - startValue);
}
PropertyValuesHolder::PropertyValuesHolder(const std::string propertyName){
    mPropertyName = propertyName;
    mEvaluator=FloatEvaluator;
}

PropertyValuesHolder::PropertyValuesHolder(Property*prop){
    mProperty = prop;
    if(prop)
        mPropertyName = prop->getName();
}

void PropertyValuesHolder::setPropertyName(const std::string& propertyName){
    mPropertyName = propertyName;
}

const std::string PropertyValuesHolder::getPropertyName()const{
    return mPropertyName;
}

void PropertyValuesHolder::setProperty(Property* property){
    mProperty=property;
}

PropertyValuesHolder* PropertyValuesHolder::ofInt(Property*prop,const std::vector<int>&values){
    PropertyValuesHolder*pvh = new PropertyValuesHolder(prop);
    pvh->setIntValues(values);
    return pvh;
}

PropertyValuesHolder* PropertyValuesHolder::ofInt(const std::string&name,const std::vector<int>&values){
    PropertyValuesHolder*pvh = new PropertyValuesHolder(name);
    pvh->setIntValues(values);
    return pvh; 
}

PropertyValuesHolder* PropertyValuesHolder::ofFloat(Property*prop,const std::vector<float>&values){
    PropertyValuesHolder*pvh = new PropertyValuesHolder(prop);
    pvh->setFloatValues(values);
    return pvh; 
}

PropertyValuesHolder* ofFloat(const std::string&name,const std::vector<float>&values){
    PropertyValuesHolder*pvh = new PropertyValuesHolder(name);
    pvh->setFloatValues(values);
    return pvh; 
}

void PropertyValuesHolder::setIntValues(const std::vector<int>&values){
    mKeyFrames.resize(values.size());
    for(int i=0;i<values.size();i++){
        KeyFrame&kf=mKeyFrames[i];
        kf.fraction=1.f/values.size();
        kf.value=values[i];
    }
}

void PropertyValuesHolder::setFloatValues(const std::vector<float>&values){
    mKeyFrames.resize(values.size());
    for(int i=0;i<values.size();i++){
        KeyFrame&kf=mKeyFrames[i];
        kf.fraction=1.f/values.size();
        kf.value=values[i];
    }
}

/*void PropertyValuesHolder::setConverter(TypeConverter converter){
    mConverter=converter;
}*/

float PropertyValuesHolder::getValue(float fraction)const{
    const KeyFrame &first = mKeyFrames[0];
    const KeyFrame &last  = mKeyFrames[mKeyFrames.size()-1];
    if(mKeyFrames.size()==2){
        return mEvaluator(fraction,first.value,last.value);
    }
    if(fraction<=.0f){
        const KeyFrame& next = mKeyFrames[1];
        float prevFraction =first.fraction;
        float intervalFraction = (fraction -prevFraction) / (next.fraction- prevFraction);
        return mEvaluator(intervalFraction,first.value,next.value);
    }
    if(fraction>=1.f){
        const KeyFrame& prev = mKeyFrames[mKeyFrames.size() - 2];
        float prevFraction = prev.fraction;
        float intervalFraction = (fraction - prevFraction) / (last.fraction - prevFraction);
        return mEvaluator(intervalFraction, prev.value, last.value);
    }
    const int mNumKeyframes =mKeyFrames.size();
    for (int i = 1; i < mNumKeyframes; ++i) {
        const KeyFrame& prevKeyframe = mKeyFrames[i-1];
        const KeyFrame& nextKeyframe = mKeyFrames[i];
        if (fraction < nextKeyframe.fraction) {
            //TimeInterpolator interpolator = nextKeyframe.getInterpolator();
            float prevFraction = prevKeyframe.fraction;
            float intervalFraction = (fraction - prevFraction) /
                    (nextKeyframe.fraction - prevFraction);
            // Apply interpolator on the proportional duration.
            //if (interpolator != null) intervalFraction = interpolator.getInterpolation(intervalFraction);
            return mEvaluator(intervalFraction, prevKeyframe.value,nextKeyframe.value);
        }
    }
    // shouldn't reach here
    return last.value;
}

void PropertyValuesHolder::calculateValue(float fraction){
    mAnimatedValue=getValue(fraction);
}

float PropertyValuesHolder::getAnimatedValue()const{
    return mAnimatedValue;
}

}

