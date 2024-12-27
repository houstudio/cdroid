#include <animation/animatorset.h>
#include <animation/animatorinflater.h>
#include <animation/animationutils.h>
#include <core/typedvalue.h>
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
        std::string propertyName;
        an.name = name;
        an.animator = nullptr;
        an.atts.setContext(pd->context,pd->package);
        an.atts.set(satts);
        if(strcmp(name,"selector")==0){
            pd->statelistAnimator = new StateListAnimator();
        }if(strcmp(name,"item")==0){
            StateSet::parseState(an.state,an.atts);
        }else if(strcmp(name,"objectAnimator")==0){
            propertyName = an.atts.getString("propertyName");
            an.animator = AnimatorInflater::loadObjectAnimator(pd->context,an.atts);
            pd->mChildren.push_back(an.animator);
        }else if(strcmp(name,"set")==0){
            an.animator = new AnimatorSet();
            pd->mChildren.clear();
        }
        if( (pd->items.size()==0) && (pd->statelistAnimator==nullptr) )
            pd->animator = an.animator;

        LOGD("%s %s animator=%p",(std::string(pd->items.size()*4,' ')+name).c_str(),propertyName.c_str(),an.animator);
        pd->items.push_back(an);
    }

    static void __endElement(void *userData, const XML_Char *name){
        AnimatorParseData*pd =(AnimatorParseData*)userData;
        AnimNode back = pd->items.back();
        if(strcmp(name,"item")==0){
            pd->statelistAnimator->addState(back.state,back.animator);
        }else if(strcmp(name,"set")==0){
            const int together = back.atts.getInt("ordering",std::map<const std::string,int>{
                 {"together",0}, {"sequentially",1}},0);
            AnimatorSet*aset = (AnimatorSet*)back.animator;
            if(together==0) aset->playTogether(pd->mChildren);
            else aset->playSequentially(pd->mChildren);
            if(pd->items.size()>1){
                AnimNode* parent = pd->fromTop(pd->statelistAnimator?-2:-1);
                parent->animator=back.animator;
            }
        }
        pd->items.pop_back();
        if(pd->items.size()&&back.animator){
            AnimNode* parent = pd->fromTop(-1);
            if(parent->name.compare("item")==0){
                parent->animator=back.animator;
            }
        }
        LOGD("%s",(std::string(pd->items.size()*4,' ')+name).c_str());
    }
};

Animator* AnimatorInflater::createAnimatorFromXml(Context*ctx,const std::string&resid){
    std::streamsize len;
    char buf[128];
    AnimatorParseData pd;
    pd.context = ctx;
    pd.animator  = nullptr;
    pd.statelistAnimator = nullptr;
    std::unique_ptr<std::istream>stream = ctx->getInputStream(resid,&pd.package);
    if(stream ==nullptr){
        return nullptr;
    }
    XML_Parser parser = XML_ParserCreateNS(nullptr,' ');
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
    return pd.animator;
}

StateListAnimator* AnimatorInflater::createStateListAnimatorFromXml(Context*ctx,const std::string&resid){
    int len;
    char buf[128];
    AnimatorParseData pd;
    pd.context = ctx;
    pd.animator  = nullptr;
    pd.statelistAnimator = nullptr;
    std::unique_ptr<std::istream>stream = ctx->getInputStream(resid,&pd.package);
    if(stream ==nullptr){
        return nullptr;
    }
    XML_Parser parser = XML_ParserCreateNS(nullptr,' ');
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

bool AnimatorInflater::isColorType(int type) {
    return (type >= TypedValue::TYPE_FIRST_COLOR_INT) && (type <= TypedValue::TYPE_LAST_COLOR_INT);
}

int AnimatorInflater::valueTypeFromPropertyName(const std::string& name){
    static const std::map<const std::string,int>valueTypes = {
       {"alpha",(int)VALUE_TYPE_FLOAT},
       {"bottom",(int)VALUE_TYPE_INT},
       {"left",(int)VALUE_TYPE_INT},
       {"elevation",(int)VALUE_TYPE_FLOAT},
       {"pivotX",(int)VALUE_TYPE_FLOAT},
       {"pivotY",(int)VALUE_TYPE_FLOAT},
       {"right",(int)VALUE_TYPE_INT},
       {"rotation",(int)VALUE_TYPE_FLOAT},
       {"rotationX",(int)VALUE_TYPE_FLOAT},
       {"rotationY",(int)VALUE_TYPE_FLOAT},
       {"scaleX",(int)VALUE_TYPE_FLOAT},
       {"scaleY",(int)VALUE_TYPE_FLOAT},
       {"scrollX",(int)VALUE_TYPE_INT},
       {"scrollY",(int)VALUE_TYPE_INT},
       {"top",(int)VALUE_TYPE_INT},
       {"translationX",(int)VALUE_TYPE_FLOAT},
       {"translationY",(int)VALUE_TYPE_FLOAT},
       {"translationZ",(int)VALUE_TYPE_FLOAT},
       {"x",(int)VALUE_TYPE_FLOAT},
       {"y",(int)VALUE_TYPE_FLOAT},
       {"z",(int)VALUE_TYPE_FLOAT},
    };
    auto it = valueTypes.find(name);
    if(it != valueTypes.end()) return it->second;
    return TypedValue::TYPE_NULL;
}

PropertyValuesHolder*AnimatorInflater::getPVH(const AttributeSet&atts, int valueType,const std::string& propertyName){
    PropertyValuesHolder* returnValue = nullptr;
    const std::string sFrom = atts.getString("valueFrom");
    const std::string sTo = atts.getString("valueTo");
    const bool hasFrom = !sFrom.empty();
    const bool hasTo   = !sTo.empty();
    const int fromType = valueTypeFromPropertyName(propertyName);
    const int toType = fromType;
    const bool getFloats = (valueType==VALUE_TYPE_FLOAT)||(fromType==VALUE_TYPE_FLOAT);

    if (valueType == VALUE_TYPE_PATH) {
        std::string fromString = atts.getString("valueFrom");
        std::string toString = atts.getString("valueTo");
        /*PathParser.PathData nodesFrom = fromString == null ? null : new PathParser.PathData(fromString);
        PathParser.PathData nodesTo = toString == null  ? null : new PathParser.PathData(toString);

        if (nodesFrom != null || nodesTo != null) {
            if (nodesFrom != null) {
                TypeEvaluator evaluator = new PathDataEvaluator();
                if (nodesTo != null) {
                    if (!PathParser.canMorph(nodesFrom, nodesTo)) {
                        throw std::runtime_error(std::string(" Can't morph from") + fromString + " to " + toString);
                    }
                    returnValue = PropertyValuesHolder::ofObject(propertyName, evaluator, nodesFrom, nodesTo);
                } else {
                    returnValue = PropertyValuesHolder::ofObject(propertyName, evaluator, (Object) nodesFrom);
                }
            } else if (nodesTo != null) {
                TypeEvaluator evaluator = new PathDataEvaluator();
                returnValue = PropertyValuesHolder::ofObject(propertyName, evaluator, (Object) nodesTo);
            }
        }*/
    } else {
        /*TypeEvaluator evaluator = nullptr;
        // Integer and float value types are handled here.
        if (valueType == VALUE_TYPE_COLOR) {
            // special case for colors: ignore valueType and get ints
            evaluator = ArgbEvaluator.getInstance();
        }*/
        if (getFloats) {
            float valueFrom,valueTo;
            if (hasFrom) {
                if(fromType==VALUE_TYPE_INT) {/*TypedValue::TYPE_DIMENSION*/
                    valueFrom = atts.getDimension("valueFrom", 0);
                }else{
                    valueFrom = atts.getFloat("valueFrom",0);
                }
                if (hasTo) {
                    if(toType==VALUE_TYPE_INT)/*TypedValue::TYPE_DIMENSION*/
                        valueTo = atts.getDimension("valueTo", 0);
                    else
                        valueTo = atts.getFloat("valueTo",0);
                    returnValue = PropertyValuesHolder::ofFloat(propertyName,{valueFrom, valueTo});
                } else {
                    returnValue = PropertyValuesHolder::ofFloat(propertyName,{valueFrom});
                }
            } else {
                if(toType==VALUE_TYPE_INT)/*TypedValue::TYPE_DIMENSION*/
                    valueTo = atts.getDimension("valueTo", 0);
                else
                    valueTo = atts.getFloat("valueTo",0);
                returnValue = PropertyValuesHolder::ofFloat(propertyName, {valueTo});
            }
        } else {
            int valueFrom,valueTo;
            if (hasFrom) {
                if (fromType == VALUE_TYPE_INT) {/*TypedValue::TYPE_DIMENSION*/
                    valueFrom = (int) atts.getDimension("valueFrom", 0);
                } else if (isColorType(fromType)) {
                    valueFrom = atts.getColor("valueFrom", 0);
                } else {
                    valueFrom = atts.getInt("valueFrom", 0);
                }
                if (hasTo) {
                    if (toType == VALUE_TYPE_INT) {/*TypedValue::TYPE_DIMENSION*/
                        valueTo = (int) atts.getDimension("valueTo", 0);
                    } else if (isColorType(toType)) {
                        valueTo = atts.getColor("valueTo", 0);
                    } else {
                        valueTo = atts.getInt("valueTo", 0);
                    }
                    returnValue = PropertyValuesHolder::ofInt(propertyName, {valueFrom, valueTo});
                } else {
                    returnValue = PropertyValuesHolder::ofInt(propertyName, {valueFrom});
                }
            } else {
                if (hasTo) {
                    if (toType == VALUE_TYPE_INT) {/*TypedValue::TYPE_DIMENSION*/
                        valueTo = (int) atts.getDimension("valueTo", 0);
                    } else if (isColorType(toType)) {
                        valueTo = atts.getColor("valueTo", 0);
                    } else {
                        valueTo = atts.getInt("valueTo", 0);
                    }
                    returnValue = PropertyValuesHolder::ofInt(propertyName, {valueTo});
                }
            }
        }
        /*if (returnValue != null && evaluator != null) {
            returnValue.setEvaluator(evaluator);
        }*/
    }
    return returnValue;
}

ObjectAnimator* AnimatorInflater::loadObjectAnimator(Context*ctx,const AttributeSet& atts){
    const std::string propertyName = atts.getString("propertyName");
    ObjectAnimator*anim = new ObjectAnimator(nullptr,propertyName);
    loadValueAnimator(ctx,atts,anim);
    return anim;
}

ValueAnimator*  AnimatorInflater::loadValueAnimator(Context*context,const AttributeSet& atts, ValueAnimator*anim){
    const int valueType = atts.getInt("valueType",std::map<const std::string,int>{
            {"floatType",(int)VALUE_TYPE_FLOAT},  {"intType",(int)VALUE_TYPE_INT},
            {"colorType",(int)VALUE_TYPE_COLOR},  {"pathType",(int)VALUE_TYPE_PATH}
        },(int)VALUE_TYPE_UNDEFINED);

    const std::string propertyName = atts.getString("propertyName");
    const std::string intpResource = atts.getString("interpolator");
    Interpolator* interpolator= AnimationUtils::loadInterpolator(context,intpResource);
    if(anim==nullptr){
        anim = new ValueAnimator();
    }
    if(interpolator){
        anim->setInterpolator(interpolator);
    }
    anim->setDuration(atts.getInt("duration",300));
    anim->setStartDelay(atts.getInt("startOffset",0));
    anim->setRepeatCount(atts.getInt("repeatCount",0));
    anim->setRepeatMode(atts.getInt("repeatMode",std::map<const std::string,int>{
        {"restart" , (int)ValueAnimator::RESTART},
        {"reverse" , (int)ValueAnimator::REVERSE},
        {"infinite", (int)ValueAnimator::INFINITE}
    },ValueAnimator::RESTART));

    PropertyValuesHolder*pvh = getPVH(atts,valueType,propertyName);
    if(pvh)
        anim->setValues({pvh});
    return anim;
}

}
