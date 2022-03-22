#include <animation/animatorset.h>
#include <animation/animatorinflater.h>
#include <expat.h>

namespace cdroid{

Animator* AnimatorInflater::loadAnimator(Context* context,const std::string&resid){
    return createAnimatorFromXml(context,resid);
}

StateListAnimator* AnimatorInflater::loadStateListAnimator(Context* context,const std::string&resid){
    return createStateListAnimatorFromXml(context,resid);
}

typedef struct{
    Context*context;
    std::vector<Animator*>animators;
    std::string package;
    Animator*result;
    std::function<ObjectAnimator*(Context*,const AttributeSet&)>fnObject;
    std::function<ValueAnimator*(Context*,const AttributeSet&,ValueAnimator*)>fnAnimator;
}ParseData;

static void startElement(void *userData, const XML_Char *name, const XML_Char **satts){
    ParseData*pd =(ParseData*)userData;
    AttributeSet atts;
    atts.setContext(pd->context,pd->package);
    atts.set(satts);
    Animator*anim = nullptr;
    if(strcmp(name,"objectAnimator")==0){
        anim = pd->fnObject(pd->context,atts);
    }else if(strcmp(name,"animator")==0){
        anim = pd->fnAnimator(pd->context,atts,nullptr);
    }else if(strcmp(name,"set")==0){
        anim = new AnimatorSet();
    }else if(strcmp(name,"propertyValuesHolder")==0){
    }else{
        LOGE("Unknown animator name:%s",name);
    }
    if(pd->animators.empty())
        pd->result = anim;
    pd->animators.push_back(anim);
}

static void endElement(void *userData, const XML_Char *name){
    ParseData*pd =(ParseData*)userData;
 
}

Animator* AnimatorInflater::createAnimatorFromXml(Context*ctx,const std::string&resid){
    ParseData pd;
    int len;
    char buf[128];
    XML_Parser parser=XML_ParserCreateNS(nullptr,' ');
    pd.context = ctx;
    pd.result  = nullptr;
    pd.fnObject  = AnimatorInflater::loadObjectAnimator;
    pd.fnAnimator= AnimatorInflater::loadValueAnimator;
    std::unique_ptr<std::istream>stream=ctx->getInputStream(resid,&pd.package);
    XML_SetUserData(parser,&pd);
    XML_SetElementHandler(parser, startElement, endElement);
    do {
        stream->read(buf,sizeof(buf));
        len=stream->gcount();
        if (XML_Parse(parser, buf,len,len==0) == XML_STATUS_ERROR) {
            const char*es=XML_ErrorString(XML_GetErrorCode(parser));
            LOGE("%s at line %ld",es, XML_GetCurrentLineNumber(parser));
            XML_ParserFree(parser);
            return nullptr;
        }
    } while(len!=0);
    XML_ParserFree(parser);
    return pd.result;
}

StateListAnimator* AnimatorInflater::createStateListAnimatorFromXml(Context*ctx,const std::string&resid){
    return nullptr;
}

ObjectAnimator* AnimatorInflater::loadObjectAnimator(Context*ctx,const AttributeSet& atts){
    ObjectAnimator*anim = new ObjectAnimator();
    loadValueAnimator(ctx,atts,anim);
    return anim;
}

ValueAnimator*  AnimatorInflater::loadValueAnimator(Context*context,const AttributeSet& atts, ValueAnimator*anim){
    if(anim==nullptr)
        anim = new ValueAnimator();
    anim->setDuration(atts.getInt("duration",300));
    anim->setStartDelay(atts.getInt("startDelay",0));
    anim->setRepeatCount(atts.getInt("repeatCount",0));
    anim->setRepeatMode(atts.getInt("repeatMode",std::map<const std::string,int>{
        {"restart" , (int)ValueAnimator::RESTART},
        {"reverse" , (int)ValueAnimator::REVERSE},
        {"infinite", (int)ValueAnimator::INFINITE}
    },ValueAnimator::RESTART));

    std::vector<float>values;
    if(atts.hasAttribute("valueFrom")){
        float f=atts.getFloat("valueFrom",.0f);
        values.push_back(f);
    }
    if(atts.hasAttribute("valueTo")){
        float f=atts.getFloat("valueTo",.0f); 
        values.push_back(f);
    }
    anim->setFloatValues(values);
    return anim;
}

}
