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
    std::vector<Animator*>mChildren;
    StateListAnimator*statelistAnimator;
    AnimNode*fromTop(int pos){
        int size =(int)items.size();
        if(size+pos<0||pos>0)return nullptr;
        return &items.at(size+pos);
    }
}AnimatorParseData;

class AnimatorInflater::AnimatorParser{
public:
    static void startElement(void *userData, const XML_Char *name, const XML_Char **satts){
        AnimatorParseData*pd =(AnimatorParseData*)userData;
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
            pd->mChildren.clear();
        }
        LOGD("%s",(std::string(pd->items.size()*4,' ')+name).c_str());
        pd->items.push_back(an);
    }

    static void __endElement(void *userData, const XML_Char *name){
        AnimatorParseData*pd =(AnimatorParseData*)userData;
        AnimNode back = pd->items.back();
        if(strcmp(name,"item")==0){
            pd->statelistAnimator->addState(back.state,back.animator);
        }else if((strcmp(name,"objectAnimator")==0)||(strcmp(name,"animator")==0)){
            AnimNode*parent = pd->fromTop(-2);
            if(parent->name.compare("set")==0){
                pd->mChildren.push_back(back.animator);
            }
        }else if((strcmp(name,"set")==0)&&pd->items.size()){
            AnimNode* parent = pd->fromTop(-2);
            if(parent->name.compare("item")){
                const int together = back.atts.getInt("ordering",std::map<const std::string,int>{
                    {"together",0}, {"sequentially",1}},0);
                AnimatorSet*aset = (AnimatorSet*)parent->animator;
                if(together==0)
                    aset->playTogether(pd->mChildren);
                else
                    aset->playSequentially(pd->mChildren);
            }else{
                parent->animator=back.animator;
            }
        }
        pd->items.pop_back();
        LOGD("%s",(std::string(pd->items.size()*4,' ')+name).c_str());
    }
};

Animator* AnimatorInflater::createAnimatorFromXml(Context*ctx,const std::string&resid){
    int len;
    char buf[128];
    XML_Parser parser = XML_ParserCreateNS(nullptr,' ');
    AnimatorParseData pd;
    pd.context = ctx;
    pd.animator  = nullptr;
    pd.statelistAnimator = nullptr;
    std::unique_ptr<std::istream>stream = ctx->getInputStream(resid,&pd.package);
    XML_SetUserData(parser,&pd);
    XML_SetElementHandler(parser, AnimatorParser::startElement, AnimatorParser::__endElement);
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
    int len;
    char buf[128];
    XML_Parser parser = XML_ParserCreateNS(nullptr,' ');
    AnimatorParseData pd;
    pd.context = ctx;
    pd.animator  = nullptr;
    pd.statelistAnimator = nullptr;
    std::unique_ptr<std::istream>stream=ctx->getInputStream(resid,&pd.package);
    XML_SetUserData(parser,&pd);
    XML_SetElementHandler(parser, AnimatorParser::startElement, AnimatorParser::__endElement);
    do {
        stream->read(buf,sizeof(buf));
        len = stream->gcount();
        if (XML_Parse(parser, buf,len,len==0) == XML_STATUS_ERROR) {
            const char*es = XML_ErrorString(XML_GetErrorCode(parser));
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
