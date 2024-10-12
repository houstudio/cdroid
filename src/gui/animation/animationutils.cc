#include <animation/animationutils.h>
#include <animations.h>
#include <core/systemclock.h>
#include <expat.h>
#include <cdlog.h>

namespace cdroid{

int64_t AnimationUtils::currentAnimationTimeMillis(){
    return SystemClock::uptimeMillis();
}

typedef struct{
   Context*context;
   std::string package;
   Animation*animation;
   std::vector<Animation*>animations;
}ANIMPARSERDATA;

static void startAnimation(void *userData, const XML_Char *xname, const XML_Char **satts){
    ANIMPARSERDATA*pd = (ANIMPARSERDATA*)userData;
    AttributeSet attrs(pd->context,pd->package);
    std::string name = xname;
    attrs.set(satts);
    if (0==name.compare("set")) {
        pd->animation= new AnimationSet(pd->context, attrs);
        //createAnimationFromXml(c, parser, (AnimationSet)anim, attrs);
    } else if (0==name.compare("alpha")) {
        pd->animation = new AlphaAnimation(pd->context, attrs);
    } else if (0==name.compare("scale")) {
        pd->animation = new ScaleAnimation(pd->context, attrs);
    }  else if (0==name.compare("rotate")) {
        pd->animation = new RotateAnimation(pd->context, attrs);
    }  else if (0==name.compare("translate")) {
        pd->animation = new TranslateAnimation(pd->context, attrs);
    } else if (0==name.compare("cliprect")) {
        pd->animation = new ClipRectAnimation(pd->context, attrs);
    } else {
        LOGE("Unknown animation name: %s" ,xname);
    }
    pd->animations.push_back(pd->animation);
}

static void endAnimation(void *userData, const XML_Char *name){
    ANIMPARSERDATA*pd = (ANIMPARSERDATA*)userData;
    Animation*last = pd->animations.back();
    pd->animations.pop_back();
    pd->animation = last;
    if( pd->animations.size()){
        AnimationSet*aset=(AnimationSet*)pd->animations.back();
        aset->addAnimation(last);
    }
}

Animation* AnimationUtils::loadAnimation(Context* context,const std::string&resid){
    size_t rdlen;
    char buf[256];
    ANIMPARSERDATA pd;
    void*parseParams[2];
    pd.context = context;
    pd.animation=nullptr;
    XML_Parser parser = XML_ParserCreateNS(nullptr,' ');
    XML_SetElementHandler(parser, startAnimation, endAnimation);
    XML_SetUserData(parser,(void*)&pd);
    std::unique_ptr<std::istream> stream = context->getInputStream(resid,&pd.package);
    do {
        stream->read(buf,sizeof(buf));
        rdlen = stream->gcount();
        if (XML_Parse(parser, buf,rdlen,!rdlen) == XML_STATUS_ERROR) {
            const char*es = XML_ErrorString(XML_GetErrorCode(parser));
            LOGE("%s at %s:line %ld",es, resid.c_str(),XML_GetCurrentLineNumber(parser));
            XML_ParserFree(parser);
            break;
        }
    } while(rdlen);
    XML_ParserFree(parser);
    return pd.animation;
}

typedef struct{
    Context*context;
    LayoutAnimationController*controller;
    std::string package;
}LACDATA;

static void startAnimationController(void *userData, const XML_Char *xname, const XML_Char **satts){
    LACDATA*pd =(LACDATA*)userData;
    AttributeSet attrs(pd->context,pd->package);
    if(strcmp(xname,"layoutAnimation")==0){
        pd->controller= new  LayoutAnimationController(pd->context, attrs);
    }else if(strcmp(xname,"gridLayoutAnimation")==0){
        pd->controller= new GridLayoutAnimationController(pd->context, attrs);
    }else{
        FATAL("unknown layout animation name %s",xname);
    }
}

LayoutAnimationController* AnimationUtils::loadLayoutAnimation(Context* context,const std::string&resid){
    LACDATA data;
    char buf[256];
    size_t rdlen = 0;
    data.context = context;
    data.controller = nullptr;
    std::unique_ptr<std::istream> stream = context->getInputStream(resid,&data.package);
    if(stream==nullptr)return nullptr;
    XML_Parser parser = XML_ParserCreate(nullptr);
    XML_SetElementHandler(parser, startAnimationController, nullptr);
    XML_SetUserData(parser,&data);
    do {
        stream->read(buf,sizeof(buf));
        rdlen = stream->gcount();
        if (XML_Parse(parser, buf,rdlen,!rdlen) == XML_STATUS_ERROR) {
            const char*es=XML_ErrorString(XML_GetErrorCode(parser));
            LOGE("%s at %s:line %ld",es, resid.c_str(),XML_GetCurrentLineNumber(parser));
            XML_ParserFree(parser);
            return nullptr;
        }
    } while(rdlen);
    XML_ParserFree(parser);
    return data.controller;
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

static void startPolator(void *userData, const XML_Char *xname, const XML_Char **satts){
    Interpolator**interpolator = (Interpolator**)userData;
    AttributeSet attrs;
    std::string name = xname;
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
    size_t rdlen;
    char buf[256];
    Interpolator*interpolator = nullptr;
    XML_Parser parser = XML_ParserCreate(nullptr);
    XML_SetElementHandler(parser, startPolator,nullptr);
    XML_SetUserData(parser,&interpolator);
    std::unique_ptr<std::istream> stream = context->getInputStream(resid);
    do{
        stream->read(buf,sizeof(buf));
        rdlen = stream->gcount();
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
