#ifndef __ANIMATOR_INFLATER_H__
#define __ANIMATOR_INFLATER_H__
#include <animation/objectanimator.h>
#include <animation/statelistanimator.h>
namespace cdroid{
class AnimatorInflater{
private:
    static constexpr int VALUE_TYPE_FLOAT     = 0;
    static constexpr int VALUE_TYPE_INT       = 1;
    static constexpr int VALUE_TYPE_PATH      = 2;
    static constexpr int VALUE_TYPE_COLOR     = 3;
    static constexpr int VALUE_TYPE_UNDEFINED = 4;
    class AnimatorParser;
    friend AnimatorParser;
private:
    static Animator* createAnimatorFromXml(Context*ctx,const std::string&resid);
    static StateListAnimator* createStateListAnimatorFromXml(Context*ctx,const std::string&resid);
    static ObjectAnimator* loadObjectAnimator(Context*ctx,const AttributeSet& attrs);
    static ValueAnimator*  loadValueAnimator(Context*context,const AttributeSet& attrs, ValueAnimator*anim);
public:
    static Animator* loadAnimator(Context* context,const std::string&resid);
    static StateListAnimator* loadStateListAnimator(Context* context,const std::string&resid);
};
}//endof namespace
#endif
