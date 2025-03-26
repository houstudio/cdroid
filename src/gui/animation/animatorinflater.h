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
    static constexpr int TOGETHER = 0;
    static constexpr int SEQUENTIALLY = 1;
private:
    static Animator* createAnimatorFromXml(Context*ctx,XmlPullParser& parser,float pixelSize);
    static Animator* createAnimatorFromXml(Context*ctx,XmlPullParser&parser,const AttributeSet& atts,
                         AnimatorSet*parent,int sequenceOrdering,float pixelSize);
    static StateListAnimator* createStateListAnimatorFromXml(Context*ctx,XmlPullParser&,const AttributeSet&);
    static int inferValueTypeFromPropertyName(const AttributeSet&atts, const std::string& propertyName);
    static PropertyValuesHolder* getPVH(const AttributeSet&atts, int valueType,const std::string& propertyName);
    static void parseAnimatorFromTypeArray(ValueAnimator* anim,const AttributeSet&atts, float pixelSize);
    static ObjectAnimator* loadObjectAnimator(Context*ctx,const AttributeSet& attrs,float );
    static ValueAnimator* loadValueAnimator(Context*context,const AttributeSet& attrs, ValueAnimator*anim,float);
    static ValueAnimator* loadAnimator(Context*ctx,const AttributeSet& attrs, ValueAnimator* anim, float pathErrorScale);
    static std::vector<PropertyValuesHolder*> loadValues(XmlPullParser& parser,const  AttributeSet& attrs);
public:
    static Animator* loadAnimator(Context* context,const std::string&resid);
    static Animator* loadAnimator(Context* context,const std::string&resid,float pathErrorScale);
    static StateListAnimator* loadStateListAnimator(Context*context,const std::string&resid);
};
}//endof namespace
#endif
