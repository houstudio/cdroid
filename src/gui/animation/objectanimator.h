#pragma once
#include <animation/valueanimator.h>
#include <animation/animationhandler.h>
namespace cdroid{

class ObjectAnimator:public ValueAnimator{
private:
    void* mTarget;
    std::string mPropertyName;
    Property* mProperty;
    bool mAutoCancel;
    ObjectAnimator(void* target,const std::string& propertyName);
    bool hasSameTargetAndProperties(const Animator*anim);
protected:
    void initAnimation()override;
    bool isInitialized()override;
public:
    ObjectAnimator();
    ~ObjectAnimator();
    void setTarget(void*target)override;
    void*getTarget();
    ObjectAnimator&setDuration(long dur)override;

    void setupStartValues()override;
    void setupEndValues()override;
    void animateValue(float fraction)override;

    void setPropertyName(const std::string&propertyName);
    void setProperty(Property* property);
    const std::string getPropertyName()const;
    void setIntValues(const std::vector<int>&)override;
    void setFloatValues(const std::vector<float>&values)override;
    ObjectAnimator*clone()const override;
    static ObjectAnimator* ofInt(void* target,const std::string& propertyName, const std::vector<int>&);
    static ObjectAnimator* ofFloat(void* target,const std::string& propertyName,const std::vector<float>&);
    static ObjectAnimator* ofInt(void*target,Property*prop,const std::vector<int>&);
    static ObjectAnimator* ofFloat(void*target,Property*prop,const std::vector<float>&);
    static ObjectAnimator* ofPropertyValuesHolder(void*target,const std::vector< PropertyValuesHolder*>&values);
    void setAutoCancel(bool);
    bool shouldAutoCancel(const AnimationHandler::AnimationFrameCallback*anim);
};

}
