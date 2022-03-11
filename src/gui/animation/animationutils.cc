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

typedef struct{
   Context*context;
   std::string package;
   int index;
   ANIMDATA data[8];
}ANIMPARSERDATA;

static void startAnimation(void *userData, const XML_Char *xname, const XML_Char **satts){
    ANIMPARSERDATA*pd=(ANIMPARSERDATA*)userData;
    AttributeSet attrs(pd->context,pd->package);
    std::string name=xname;
    Animation*anim=nullptr;
    attrs.set(satts);
    if (0==name.compare("set")) {
        anim = new AnimationSet(pd->context, attrs);
        //createAnimationFromXml(c, parser, (AnimationSet)anim, attrs);
    } else if (0==name.compare("alpha")) {
        anim = new AlphaAnimation(pd->context, attrs);
    } else if (0==name.compare("scale")) {
        anim = new ScaleAnimation(pd->context, attrs);
    }  else if (0==name.compare("rotate")) {
        anim = new RotateAnimation(pd->context, attrs);
    }  else if (0==name.compare("translate")) {
        anim = new TranslateAnimation(pd->context, attrs);
    } else if (0==name.compare("cliprect")) {
        anim = new ClipRectAnimation(pd->context, attrs);
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
    ANIMPARSERDATA pd;
    void*parseParams[2];
    pd.index = 0;
    pd.context = context;
    XML_Parser parser=XML_ParserCreate(nullptr);
    XML_SetElementHandler(parser, startAnimation, endAnimation);
    XML_SetUserData(parser,(void*)&pd);
    std::unique_ptr<std::istream>stream=context->getInputStream(resid,&pd.package);
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
    return nullptr;
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
    AttributeSet attrs;
    std::string name=xname;
    Context*ctx;
    attrs.set(satts);
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
        LOGE("Unknown interpolator name: %s",name.c_str());
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
