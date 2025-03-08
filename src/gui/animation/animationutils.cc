#include <animation/animationutils.h>
#include <animations.h>
#include <core/systemclock.h>
#include <porting/cdlog.h>

namespace cdroid{

std::unordered_map<std::string,std::shared_ptr<Interpolator>>AnimationUtils::mInterpolators;

int64_t AnimationUtils::currentAnimationTimeMillis(){
    return SystemClock::uptimeMillis();
}

Animation* AnimationUtils::loadAnimation(Context* context,const std::string&resid){
    Animation*anim = nullptr;
    int type,depth;
    XmlPullParser::XmlEvent event;
    XmlPullParser parser(context,resid);
    return createAnimationFromXml(context,parser,nullptr,event.attributes);
}

Animation* AnimationUtils::createAnimationFromXml(Context* c, XmlPullParser& parser,AnimationSet* parent,const AttributeSet& attrs){
    // Make sure we are on a start tag.
    int type;
    Animation* anim = nullptr;
    XmlPullParser::XmlEvent event;
    const int depth = parser.getDepth();

    while (((type=parser.next(event)) != XmlPullParser::END_TAG || parser.getDepth() > depth)
           && type != XmlPullParser::END_DOCUMENT) {

        if (type != XmlPullParser::START_TAG) {
            continue;
        }
        std::string  name = parser.getName();
        if (name.compare("set")==0) {
            anim = new AnimationSet(c, event.attributes);
            createAnimationFromXml(c, parser, (AnimationSet*)anim, event.attributes);
        } else if (name.compare("alpha")==0) {
            anim = new AlphaAnimation(c, event.attributes);
        } else if (name.compare("scale")==0) {
            anim = new ScaleAnimation(c, event.attributes);
        }  else if (name.compare("rotate")==0) {
            anim = new RotateAnimation(c, event.attributes);
        }  else if (name.compare("translate")==0) {
            anim = new TranslateAnimation(c, event.attributes);
        } else if (name.compare("cliprect")==0) {
            anim = new ClipRectAnimation(c, event.attributes);
        } else {
            LOGW("Unknown animation name:%s",name.c_str());
        }
        if (parent != nullptr) {
            parent->addAnimation(anim);
        }
    }
    return anim;
}

LayoutAnimationController* AnimationUtils::loadLayoutAnimation(Context* context,const std::string&resid){
    int type,depth;
    XmlPullParser::XmlEvent event;
    XmlPullParser parser(context,resid);
    return createLayoutAnimationFromXml(context,parser,event.attributes);
}

LayoutAnimationController* AnimationUtils::createLayoutAnimationFromXml(Context* c,
        XmlPullParser& parser,const AttributeSet& attrs){
    int type,depth;
    XmlPullParser::XmlEvent event;
    const int innerDepth = parser.getDepth();
    LayoutAnimationController* controller = nullptr;
    while (((type = parser.next(event,depth)) != XmlPullParser::END_TAG || innerDepth > depth)
            && type != XmlPullParser::END_DOCUMENT) {

        if (type != XmlPullParser::START_TAG) {
            continue;
        }

        const std::string name = parser.getName();

        if (name.compare("layoutAnimation")==0) {
            controller = new LayoutAnimationController(c, event.attributes);
        } else if (name.compare("gridLayoutAnimation")==0) {
            controller = new GridLayoutAnimationController(c, event.attributes);
        } else {
            LOGE("Unknown layout animation name:%s ",name.c_str());
        }
    }

    return controller;
}

Animation* AnimationUtils::makeInAnimation(Context* c, bool fromLeft){
    Animation*a = loadAnimation(c,fromLeft?"cdroid:anim/slide_in_left.xml":"cdroid:anim/slide_in_right.xml");
    a->setInterpolator(new DecelerateInterpolator());
    a->setStartTime(currentAnimationTimeMillis());
    return a;
}

Animation* AnimationUtils::makeOutAnimation(Context* c, bool toRight){
    Animation*a = loadAnimation(c,toRight?"cdroid:anim/slide_out_right.xml":"cdroid:anim/slide_out_left.xml");
    a->setInterpolator(new AccelerateInterpolator());
    a->setStartTime(currentAnimationTimeMillis());
    return a;
}

Animation* AnimationUtils::makeInChildBottomAnimation(Context* c){
    Animation*a = loadAnimation(c,"cdroid:anim/slide_in_child_bottom.xml");
    a->setInterpolator(new AccelerateInterpolator());
    a->setStartTime(currentAnimationTimeMillis());
    return a;
}

Interpolator* AnimationUtils::loadInterpolator(Context*context,const std::string& id){
    XmlPullParser parser(context,id);
    return createInterpolatorFromXml(context, parser);
}

Interpolator* AnimationUtils::createInterpolatorFromXml(Context* context,XmlPullParser&parser){
    int type,depth;
    const int innerDepth = parser.getDepth();
    BaseInterpolator*interpolator = nullptr;
    XmlPullParser::XmlEvent event;
    while(((type = parser.next(event,depth)) != XmlPullParser::END_TAG || innerDepth > depth)
                && type != XmlPullParser::END_DOCUMENT){
        if (type != XmlPullParser::START_TAG) {
            continue;
        }

        const std::string name = parser.getName();
        const AttributeSet& attrs = event.attributes;
        if (0==name.compare("linearInterpolator")) {
            interpolator = new LinearInterpolator();
        } else if (0==name.compare("accelerateInterpolator")) {
            interpolator = new AccelerateInterpolator(context, attrs);
        } else if (0==name.compare("decelerateInterpolator")) {
            interpolator = new DecelerateInterpolator(context, attrs);
        } else if (0==name.compare("accelerateDecelerateInterpolator")) {
            interpolator = new AccelerateDecelerateInterpolator();
        } else if (0==name.compare("cycleInterpolator")) {
            interpolator = new CycleInterpolator(context, attrs);
        } else if (0==name.compare("anticipateInterpolator")) {
            interpolator = new AnticipateInterpolator(context,attrs);
        } else if (0==name.compare("overshootInterpolator")) {
            interpolator = new OvershootInterpolator(context, attrs);
        } else if (0==name.compare("anticipateOvershootInterpolator")) {
            interpolator = new AnticipateOvershootInterpolator(context,attrs);
        } else if (0==name.compare("bounceInterpolator")) {
            interpolator = new BounceInterpolator();
        } else if (0==name.compare("pathInterpolator")) {
            interpolator = new PathInterpolator(context,attrs);
        } else {
            LOGE("Unknown interpolator name: %s",name.c_str());
        }
    }
    return interpolator;
}

}
