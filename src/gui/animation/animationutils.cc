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
    XmlPullParser parser(context,resid);
    const AttributeSet attrs(&parser);
    return createAnimationFromXml(context,parser,nullptr,attrs);
}

Animation* AnimationUtils::createAnimationFromXml(Context* c, XmlPullParser& parser,AnimationSet* parent,const AttributeSet& attrs){
    // Make sure we are on a start tag.
    int type;
    Animation* anim = nullptr;
    const int depth = parser.getDepth();

    while (((type=parser.next()) != XmlPullParser::END_TAG || parser.getDepth() > depth)
           && (type != XmlPullParser::END_DOCUMENT) && (type!=XmlPullParser::BAD_DOCUMENT) ) {

        if (type != XmlPullParser::START_TAG) {
            continue;
        }
        std::string  name = parser.getName();
        if (name.compare("set")==0) {
            anim = new AnimationSet(c, attrs);
            createAnimationFromXml(c, parser, (AnimationSet*)anim, attrs);
        } else if (name.compare("alpha")==0) {
            anim = new AlphaAnimation(c, attrs);
        } else if (name.compare("scale")==0) {
            anim = new ScaleAnimation(c, attrs);
        }  else if (name.compare("rotate")==0) {
            anim = new RotateAnimation(c, attrs);
        }  else if (name.compare("translate")==0) {
            anim = new TranslateAnimation(c, attrs);
        } else if (name.compare("cliprect")==0) {
            anim = new ClipRectAnimation(c, attrs);
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
    XmlPullParser parser(context,resid);
    const AttributeSet attrs(&parser);;
    return createLayoutAnimationFromXml(context,parser,attrs);
}

LayoutAnimationController* AnimationUtils::createLayoutAnimationFromXml(Context* c,
        XmlPullParser& parser,const AttributeSet& attrs){
    int type;
    const int depth = parser.getDepth();
    LayoutAnimationController* controller = nullptr;
    while (((type = parser.next()) != XmlPullParser::END_TAG || parser.getDepth()>depth)
            && (type != XmlPullParser::END_DOCUMENT) && (type!=XmlPullParser::BAD_DOCUMENT) ) {

        if (type != XmlPullParser::START_TAG) {
            continue;
        }

        const std::string name = parser.getName();
        if (name.compare("layoutAnimation")==0) {
            controller = new LayoutAnimationController(c, attrs);
        } else if (name.compare("gridLayoutAnimation")==0) {
            controller = new GridLayoutAnimationController(c, attrs);
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
    int type;
    const int depth = parser.getDepth();
    BaseInterpolator*interpolator = nullptr;
    const AttributeSet attrs(&parser);
    while(((type = parser.next()) != XmlPullParser::END_TAG || parser.getDepth() > depth)
                && type != XmlPullParser::END_DOCUMENT){
        if (type != XmlPullParser::START_TAG) {
            continue;
        }

        const std::string name = parser.getName();
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
