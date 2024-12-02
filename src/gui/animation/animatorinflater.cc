#include <animation/animatorset.h>
#include <animation/animatorinflater.h>
#include <expat.h>
#include <cdlog.h>

namespace cdroid{

Animator* AnimatorInflater::loadAnimator(Context* context,const std::string&resid){
    return createAnimatorFromXml(context,resid);
}

StateListAnimator* AnimatorInflater::loadStateListAnimator(Context* context,const std::string&resid){
    return createStateListAnimatorFromXml(context,resid);
}

typedef struct{
    std::string name;
    Animator*animator;
    std::vector<int>state;
    AttributeSet atts;
}AnimNode;
typedef struct{
    Context*context;
    std::vector<AnimNode>items;
    std::string package;
    Animator*animator;
    StateListAnimator*statelistAnimator;
}ParseData;

class AnimatorInflater::AnimatorParser{
public:
static void startElement(void *userData, const XML_Char *name, const XML_Char **satts){
    ParseData*pd =(ParseData*)userData;
    AnimNode an;
    AttributeSet atts;
    an.name = name;
    an.animator = nullptr;
    an.atts.setContext(pd->context,pd->package);
    an.atts.set(satts);
    if(strcmp(name,"selector")==0){
        pd->statelistAnimator = new StateListAnimator();
    }if(strcmp(name,"item")==0){
        StateSet::parseState(an.state,an.atts);
    }else if(strcmp(name,"objectAnimator")==0){
        an.animator = AnimatorInflater::loadObjectAnimator(pd->context,an.atts);
    }else if(strcmp(name,"animator")==0){
        an.animator = AnimatorInflater::loadValueAnimator(pd->context,an.atts,nullptr);
    }else if(strcmp(name,"set")==0){
        an.animator = new AnimatorSet();
    }
    LOGD("name:%s",(std::string(pd->items.size()*2,' ')+name).c_str());
    pd->items.push_back(an);
}

static void endElement(void *userData, const XML_Char *name){
    ParseData*pd =(ParseData*)userData;
    pd->items.pop_back();
    if(strcmp(name,"item")==0){
    }else if(strcmp(name,"objectAnimator")==0){
    }else if(strcmp(name,"animator")==0){
    }else if((strcmp(name,"set")==0)&&pd->items.size()){
        AnimNode an=(AnimNode)pd->items.back();
        AnimatorSet*aset=(AnimatorSet*)an.animator;
        //aset->playTogether(pd->animators);
    }
}
};
Animator* AnimatorInflater::createAnimatorFromXml(Context*ctx,const std::string&resid){
    ParseData pd;
    int len;
    char buf[128];
    XML_Parser parser=XML_ParserCreateNS(nullptr,' ');
    pd.context = ctx;
    pd.animator  = nullptr;
    pd.statelistAnimator = nullptr;
    std::unique_ptr<std::istream>stream=ctx->getInputStream(resid,&pd.package);
    XML_SetUserData(parser,&pd);
    XML_SetElementHandler(parser, AnimatorParser::startElement, AnimatorParser::endElement);
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
    return pd.animator;
}

StateListAnimator* AnimatorInflater::createStateListAnimatorFromXml(Context*ctx,const std::string&resid){
    ParseData pd;
    int len;
    char buf[128];
    XML_Parser parser=XML_ParserCreateNS(nullptr,' ');
    pd.context = ctx;
    pd.animator  = nullptr;
    pd.statelistAnimator = nullptr;
    std::unique_ptr<std::istream>stream=ctx->getInputStream(resid,&pd.package);
    XML_SetUserData(parser,&pd);
    XML_SetElementHandler(parser, AnimatorParser::startElement, AnimatorParser::endElement);
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
    return pd.statelistAnimator;
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
