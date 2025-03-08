#ifndef __ANIMATION_UTILS_H__
#define __ANIMATION_UTILS_H__
#include <memory>
#include <core/xmlpullparser.h>
#include <animation/animationset.h>
#include <animation/layoutanimationcontroller.h>
namespace cdroid{

class AnimationUtils{
private:
    static std::unordered_map<std::string,std::shared_ptr<Interpolator>>mInterpolators;
    static Animation* createAnimationFromXml(Context* c, XmlPullParser& parser,AnimationSet* parent,const AttributeSet& attrs);
    static LayoutAnimationController* createLayoutAnimationFromXml(Context* c,XmlPullParser& parser,const AttributeSet& attrs);
    static Interpolator* createInterpolatorFromXml(Context* context,XmlPullParser&);
public:
    static int64_t currentAnimationTimeMillis();
    static Animation* loadAnimation(Context* context,const std::string&id);
    static LayoutAnimationController* loadLayoutAnimation(Context* context,const std::string&id);
    static Animation* makeInAnimation(Context* c, bool fromLeft);
    static Animation* makeOutAnimation(Context* c, bool toRight);
    static Animation* makeInChildBottomAnimation(Context* c);
    static Interpolator* loadInterpolator(Context*,const std::string& id);
};

}//endof namespace

#endif/*__ANIMATION_UTILS_H__*/
