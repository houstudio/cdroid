#include <animation/animationutils.h>
#include <animations.h>
#include <core/systemclock.h>
#include <expat.h>
#include <cdlog.h>

namespace cdroid{

long AnimationUtils::currentAnimationTimeMillis(){
    return SystemClock::uptimeMillis();
}

typedef struct{
   Animation*anim;
   AttributeSet attr;
}ANIMDATA;

static void startAnimation(void *userData, const XML_Char *xname, const XML_Char **satts){
    void** parseParams=(void**)userData;
    int*index=(int*)parseParams[0];
    ANIMDATA*ads=(ANIMDATA*)parseParams[1];
    Context*c=(Context*)parseParams[2];
    AttributeSet attrs=AttributeSet(satts);
    std::string name=xname;
    Animation*anim=nullptr;
    if (0==name.compare("set")) {
        anim = new AnimationSet(c, attrs);
        //createAnimationFromXml(c, parser, (AnimationSet)anim, attrs);
    } else if (0==name.compare("alpha")) {
        anim = new AlphaAnimation(c, attrs);
    } else if (0==name.compare("scale")) {
        anim = new ScaleAnimation(c, attrs);
    }  else if (0==name.compare("rotate")) {
        anim = new RotateAnimation(c, attrs);
    }  else if (0==name.compare("translate")) {
        anim = new TranslateAnimation(c, attrs);
    } else if (0==name.compare("cliprect")) {
        anim = new ClipRectAnimation(c, attrs);
    } else {
        LOGE("Unknown animation name: %s" ,xname);
    }
}

static void endAnimation(void *userData, const XML_Char *name){
}

Animation* AnimationUtils::loadAnimation(Context* context,const std::string&resid){
    int rdlen;
    char buf[256];
    int index=0;
    ANIMDATA ads[8];
    void*parseParams[2];
    parseParams[0]=&index;
    parseParams[1]=(void*)ads;
    parseParams[2]=context;
    XML_Parser parser=XML_ParserCreate(nullptr);
    XML_SetElementHandler(parser, startAnimation, endAnimation);
    XML_SetUserData(parser,(void*)parseParams);
    std::unique_ptr<std::istream>stream=context->getInputStream(resid);
    do {
        stream->read(buf,sizeof(buf));
        rdlen=stream->gcount();
        if (XML_Parse(parser, buf,rdlen,!rdlen) == XML_STATUS_ERROR) {
            const char*es=XML_ErrorString(XML_GetErrorCode(parser));
            LOGE("%s at %s:line %ld",es, resid.c_str(),XML_GetCurrentLineNumber(parser));
            XML_ParserFree(parser);
            break;
        }
    } while(rdlen);
    XML_ParserFree(parser);
    return nullptr;
}

LayoutAnimationController* AnimationUtils::loadLayoutAnimation(Context* context,const std::string&resid){
}

Animation* AnimationUtils::makeInAnimation(Context* c, bool fromLeft){
    Animation*a=loadAnimation(c,fromLeft?"cdroid:anim/slide_in_left.xml":"cdroid:anim/slide_in_right.xml");
    a->setInterpolator(new DecelerateInterpolator());
    a->setStartTime(currentAnimationTimeMillis());
    return a;
}

Animation* AnimationUtils::makeOutAnimation(Context* c, bool toRight){
    Animation*a=loadAnimation(c,toRight?"cdroid:anim/slide_out_right.xml":"cdroid:anim/slide_out_left.xml");
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

static void startPolator(void *userData, const XML_Char *xname, const XML_Char **satts){
    Interpolator**interpolator=(Interpolator**)userData;
    AttributeSet attrs(satts);
    std::string name=xname;
    Context*ctx;
    if (0==name.compare("linearInterpolator")) {
        *interpolator = new LinearInterpolator();
    } else if (0==name.compare("accelerateInterpolator")) {
        *interpolator = new AccelerateInterpolator(ctx, attrs);
    } else if (0==name.compare("decelerateInterpolator")) {
        *interpolator = new DecelerateInterpolator(ctx, attrs);
    } else if (0==name.compare("accelerateDecelerateInterpolator")) {
        *interpolator = new AccelerateDecelerateInterpolator();
    } else if (0==name.compare("cycleInterpolator")) {
        *interpolator = new CycleInterpolator(ctx, attrs);
    } else if (0==name.compare("anticipateInterpolator")) {
        *interpolator = new AnticipateInterpolator(ctx,attrs);
    } else if (0==name.compare("overshootInterpolator")) {
        *interpolator = new OvershootInterpolator(ctx, attrs);
    } else if (0==name.compare("anticipateOvershootInterpolator")) {
        *interpolator = new AnticipateOvershootInterpolator(ctx,attrs);
    } else if (0==name.compare("bounceInterpolator")) {
        *interpolator = new BounceInterpolator();
    } else if (0==name.compare("pathInterpolator")) {
        *interpolator = new PathInterpolator(ctx,attrs);
    } else {
        LOGE("Unknown interpolator name: %s",name);
    }
}

Interpolator* AnimationUtils::loadInterpolator(Context* context,const std::string&resid){
    int rdlen;
    char buf[256];
    Interpolator*interpolator=nullptr;
    XML_Parser parser=XML_ParserCreate(nullptr);
    XML_SetElementHandler(parser, startPolator,nullptr);
    XML_SetUserData(parser,&interpolator);
    std::unique_ptr<std::istream>stream=context->getInputStream(resid);
    do{
        stream->read(buf,sizeof(buf));
        rdlen=stream->gcount();
        if (XML_Parse(parser, buf,rdlen,!rdlen) == XML_STATUS_ERROR) {
            const char*es=XML_ErrorString(XML_GetErrorCode(parser));
            LOGE("%s at %s:line %ld",es, resid.c_str(),XML_GetCurrentLineNumber(parser));
            XML_ParserFree(parser); 
            return nullptr;
        }
    }while(rdlen);
    XML_ParserFree(parser);
    return interpolator;
}

}
