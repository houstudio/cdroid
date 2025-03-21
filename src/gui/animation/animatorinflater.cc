#include <animation/animatorset.h>
#include <animation/animatorinflater.h>
#include <animation/animationutils.h>
#include <core/typedvalue.h>
#include <drawables/pathparser.h>
#include <porting/cdlog.h>

namespace cdroid{

Animator* AnimatorInflater::loadAnimator(Context* context,const std::string&resid){
    return loadAnimator(context,resid,1.f);
}

Animator* AnimatorInflater::loadAnimator(Context* context,const std::string&resid,float){
    XmlPullParser parser(context,resid);
    Animator* animator = createAnimatorFromXml(context, parser, 1.f/*pathErrorScale*/);
    if (animator != nullptr) {
        //animator->appendChangingConfigurations(getChangingConfigs(resources, id));
        /*ConstantState<Animator> constantState = animator->createConstantState();
        if (constantState != nullptr) {
            if (DBG_ANIMATOR_INFLATER) {
                Log.d(TAG, "caching animator for res %s",id.c_str());
            }
            animatorCache.put(id, theme, constantState);
            // create a new animator so that cached version is never used by the user
            animator = constantState.newInstance(resources, theme);
        }*/
    }
    return animator;
}

std::unordered_map<std::string,std::shared_ptr<StateListAnimator>>mStateAnimatorMap;

StateListAnimator* AnimatorInflater::loadStateListAnimator(Context* context,const std::string&resid){
    auto it = mStateAnimatorMap.find(resid);
    if(it==mStateAnimatorMap.end()){
        XmlPullParser parser(context,resid);
        StateListAnimator*anim =createStateListAnimatorFromXml(context,parser,AttributeSet());
        it = mStateAnimatorMap.insert({resid,std::shared_ptr<StateListAnimator>(anim)}).first;
    }
    return new StateListAnimator(*it->second);
    //return it->second->createConstantState()->newInstance();
}

Animator* AnimatorInflater::createAnimatorFromXml(Context*context,XmlPullParser& parser,float pixelSize){
    return createAnimatorFromXml(context,parser, AttributeSet(), nullptr, 0,pixelSize);
}

Animator* AnimatorInflater::createAnimatorFromXml(Context*context,XmlPullParser&parser,const AttributeSet& atts,
        AnimatorSet*parent,int sequenceOrdering,float pixelSize){
     Animator* anim = nullptr;
     std::vector<Animator*> childAnims;

    // Make sure we are on a start tag.
    int type=0,depth=0;
    const int innerDepth = parser.getDepth();
    XmlPullParser::XmlEvent event;
    while ((((type = parser.next(event,depth)) != XmlPullParser::END_TAG) || (depth >= innerDepth))
            && (type != XmlPullParser::END_DOCUMENT)) {

        if (type != XmlPullParser::START_TAG) {
            continue;
        }

        std::string name = parser.getName();
        bool gotValues = false;
        if (name.compare("objectAnimator")==0) {
            anim = loadObjectAnimator(context,event.attributes, pixelSize);
        } else if (name.compare("animator")==0) {
            anim = loadAnimator(context, event.attributes, nullptr, pixelSize);
        } else if (name.compare("set")==0) {
            anim = new AnimatorSet();
            //anim->appendChangingConfigurations(a.getChangingConfigurations());
            const int ordering = atts.getInt("ordering",std::unordered_map<std::string,int>{
                    {"together",(int)TOGETHER},{"sequentially",(int)SEQUENTIALLY}}, TOGETHER);
            createAnimatorFromXml(context, parser, event.attributes, (AnimatorSet*) anim, ordering,pixelSize);
        } else if (name.compare("propertyValuesHolder")==0) {
            /*PropertyValuesHolder[] values = loadValues(parser,Xml.asAttributeSet(parser));
            if (values != null && anim != null && (anim instanceof ValueAnimator)) {
                ((ValueAnimator*) anim)->setValues(values);
            }
            gotValues = true;*/
        } else {
            LOGE("Unknown animator name:%s",name.c_str());
        }

        if ((parent != nullptr) && !gotValues) {
            childAnims.push_back(anim);
        }
    }
    if ((parent != nullptr) && childAnims.size()) {
        std::vector<Animator*> animsArray =childAnims;
        if (sequenceOrdering == TOGETHER) {
            parent->playTogether(animsArray);
        } else {
            parent->playSequentially(animsArray);
        }
    }
    return anim;
}

StateListAnimator* AnimatorInflater::createStateListAnimatorFromXml(Context*context,XmlPullParser&parser,const AttributeSet&atts){
    StateListAnimator* stateListAnimator = new StateListAnimator();
    while (true) {
        int depth;
        XmlPullParser::XmlEvent event;
        const int type = parser.next(event,depth);
        const std::string name =parser.getName();
        switch (type) {
        case XmlPullParser::END_DOCUMENT:
        case XmlPullParser::END_TAG:  return stateListAnimator;
        case XmlPullParser::START_TAG:// parse item
            if (name.compare("item")==0) {
                std::vector<int>states;
                Animator* animator = nullptr;
                StateSet::parseState(states,event.attributes);
                std::string animId =event.attributes.getString("animator");
                if(!animId.empty()){
                    animator = loadAnimator(context, animId);
                }else{
                    animator = createAnimatorFromXml(context,parser,event.attributes, nullptr, 0,1.f);
                }

                if (animator == nullptr) {
                    throw std::logic_error("animation state item must have a valid animation");
                }
                stateListAnimator->addState(states, animator);
            }
            break;
        }
    }
}

bool AnimatorInflater::isColorType(int type) {
    return (type >= TypedValue::TYPE_FIRST_COLOR_INT) && (type <= TypedValue::TYPE_LAST_COLOR_INT);
}

int AnimatorInflater::valueTypeFromPropertyName(const std::string& name){
    static const std::unordered_map<std::string,int>valueTypes = {
       {"alpha",(int)Property::FLOAT_CLASS},
       {"bottom",(int)Property::INT_CLASS},
       {"left",(int)Property::INT_CLASS},
       {"elevation",(int)Property::FLOAT_CLASS},
       {"pivotX",(int)Property::FLOAT_CLASS},
       {"pivotY",(int)Property::FLOAT_CLASS},
       {"right",(int)Property::INT_CLASS},
       {"rotation",(int)Property::FLOAT_CLASS},
       {"rotationX",(int)Property::FLOAT_CLASS},
       {"rotationY",(int)Property::FLOAT_CLASS},
       {"scaleX",(int)Property::FLOAT_CLASS},
       {"scaleY",(int)Property::FLOAT_CLASS},
       {"scrollX",(int)Property::INT_CLASS},
       {"scrollY",(int)Property::INT_CLASS},
       {"top",(int)Property::INT_CLASS},
       {"translationX",(int)Property::FLOAT_CLASS},
       {"translationY",(int)Property::FLOAT_CLASS},
       {"translationZ",(int)Property::FLOAT_CLASS},
       {"x",(int)Property::FLOAT_CLASS},
       {"y",(int)Property::FLOAT_CLASS},
       {"z",(int)Property::FLOAT_CLASS},
////////////////////////////////////////////////////////////////
       {"strokeWidth",(int)Property::FLOAT_CLASS},
       {"strokeColor",(int)Property::INT_CLASS},
       {"strokeAlpha",(int)Property::FLOAT_CLASS},
       {"fillColor",(int)Property::INT_CLASS},
       {"fillAlpha",(int)Property::FLOAT_CLASS}
    };
    auto it = valueTypes.find(name);
    if(it != valueTypes.end()) return it->second;
    return TypedValue::TYPE_NULL;
}

int AnimatorInflater::inferValueTypeFromValues(const AttributeSet&atts, const std::string& valueFromId,const std::string& valueToId) {
    bool hasFrom = !valueFromId.empty();
    int fromType = hasFrom ? valueTypeFromPropertyName(valueFromId) : 0;
    bool hasTo = !valueToId.empty();
    int toType = hasTo ? valueTypeFromPropertyName(valueToId) : 0;
    int valueType;
    // Check whether it's color type. If not, fall back to default type (i.e. float type)
    if ((hasFrom && isColorType(fromType)) || (hasTo && isColorType(toType))) {
        valueType = VALUE_TYPE_COLOR;
    } else {
        valueType = Property::FLOAT_CLASS;
    }
    return valueType;
}

PropertyValuesHolder*AnimatorInflater::getPVH(const AttributeSet&atts, int valueType,const std::string& propertyName){
    PropertyValuesHolder* returnValue = nullptr;
    const std::string sFrom = atts.getString("valueFrom");
    const std::string sTo = atts.getString("valueTo");
    const bool hasFrom = !sFrom.empty();
    const bool hasTo   = !sTo.empty();
    const int fromType = valueTypeFromPropertyName(propertyName);
    const int toType = fromType;
    const bool getFloats = (valueType==Property::FLOAT_CLASS)||(fromType==Property::FLOAT_CLASS);

    if (valueType == Property::PATH_CLASS) {
        const std::string fromString = atts.getString("valueFrom");
        const std::string toString = atts.getString("valueTo");
        PathParser::PathData* nodesFrom = fromString.empty() ? nullptr : new PathParser::PathData(fromString);
        PathParser::PathData* nodesTo = toString.empty()  ? nullptr : new PathParser::PathData(toString);

        if (nodesFrom != nullptr || nodesTo != nullptr) {
            if (nodesFrom != nullptr) {
                //TypeEvaluator evaluator = new PathDataEvaluator();
                if (nodesTo != nullptr) {
                    if (!PathParser::canMorph(*nodesFrom, *nodesTo)) {
                        throw std::runtime_error(std::string(" Can't morph from") + fromString + " to " + toString);
                    }
                    //returnValue = PropertyValuesHolder::ofObject(propertyName, evaluator, nodesFrom, nodesTo);
                } else {
                    //returnValue = PropertyValuesHolder::ofObject(propertyName, evaluator, (Object) nodesFrom);
                }
            } else if (nodesTo != nullptr) {
                //TypeEvaluator evaluator = new PathDataEvaluator();
                //returnValue = PropertyValuesHolder::ofObject(propertyName, evaluator, (Object) nodesTo);
            }
        }
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
                if(fromType==Property::INT_CLASS) {/*TypedValue::TYPE_DIMENSION*/
                    valueFrom = atts.getDimension("valueFrom", 0);
                }else{
                    valueFrom = atts.getFloat("valueFrom",0);
                }
                if (hasTo) {
                    if(toType==Property::INT_CLASS)/*TypedValue::TYPE_DIMENSION*/
                        valueTo = atts.getDimension("valueTo", 0);
                    else
                        valueTo = atts.getFloat("valueTo",0);
                    returnValue = PropertyValuesHolder::ofFloat(propertyName,{valueFrom, valueTo});
                } else {
                    returnValue = PropertyValuesHolder::ofFloat(propertyName,{valueFrom});
                }
            } else {
                if(toType==Property::INT_CLASS)/*TypedValue::TYPE_DIMENSION*/
                    valueTo = atts.getDimension("valueTo", 0);
                else
                    valueTo = atts.getFloat("valueTo",0);
                returnValue = PropertyValuesHolder::ofFloat(propertyName, {valueTo});
            }
        } else {
            int valueFrom,valueTo;
            if (hasFrom) {
                if (fromType == Property::INT_CLASS) {/*TypedValue::TYPE_DIMENSION*/
                    valueFrom = (int) atts.getDimension("valueFrom", 0);
                } else if (isColorType(fromType)) {
                    valueFrom = atts.getColor("valueFrom", 0);
                } else {
                    valueFrom = atts.getInt("valueFrom", 0);
                }
                if (hasTo) {
                    if (toType == Property::INT_CLASS) {/*TypedValue::TYPE_DIMENSION*/
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
                    if (toType == Property::INT_CLASS) {/*TypedValue::TYPE_DIMENSION*/
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

void AnimatorInflater::parseAnimatorFromTypeArray(ValueAnimator* anim,const AttributeSet&atts, float pixelSize) {
    const long duration = atts.getInt("duration", 300);
    const long startDelay = atts.getInt("startOffset", 0);

    int valueType = atts.getInt("valueType",std::unordered_map<std::string,int>{
            {"intType", (int)Property::INT_CLASS},
            {"colorType",(int)Property::COLOR_CLASS},
            {"floatType",(int)Property::FLOAT_CLASS},
            {"pathType",(int)Property::PATH_CLASS}
            }, Property::UNDEFINED);

    if (valueType == Property::UNDEFINED) {
        valueType = inferValueTypeFromValues(atts, "valueFrom","valueTo");
    }
    PropertyValuesHolder* pvh = getPVH(atts, valueType,atts.getString("propertyName"));
    if (pvh != nullptr) {
        anim->setValues({pvh});
    }

    anim->setDuration(duration);
    anim->setStartDelay(startDelay);

    if (atts.hasAttribute("repeatCount")) {
        anim->setRepeatCount(atts.getInt("repeatCount", 0));
    }
    if (atts.hasAttribute("repeatMode")) {
        anim->setRepeatMode(atts.getInt("repeatMode",ValueAnimator::RESTART));
    }

    /*if (arrayObjectAnimator != nullptr) {
        setupObjectAnimator(anim, arrayObjectAnimator, valueType, pixelSize);
    }*/
    const std::string propertyName = atts.getString("propertyName");
    if((propertyName.empty()==false)&&dynamic_cast<ObjectAnimator*>(anim)){
       ((ObjectAnimator*)anim)->setPropertyName(propertyName);
    }
}

ObjectAnimator* AnimatorInflater::loadObjectAnimator(Context*ctx,const AttributeSet& atts,float){
    ObjectAnimator*anim = new ObjectAnimator();//propertyName);
    loadAnimator(ctx,atts,anim,1.f);
    return anim;
}

ValueAnimator* AnimatorInflater::loadAnimator(Context*context,const AttributeSet& attrs, ValueAnimator* anim, float pathErrorScale){
    // If anim is not null, then it is an object animator.
    /*if (anim != nullptr) {
        if (theme != null) {
            arrayObjectAnimator = theme.obtainStyledAttributes(attrs,R.styleable.PropertyAnimator, 0, 0);
        } else {
            arrayObjectAnimator = res.obtainAttributes(attrs, R.styleable.PropertyAnimator);
        }
        anim.appendChangingConfigurations(arrayObjectAnimator.getChangingConfigurations());
    }*/

    if (anim == nullptr) {
        anim = new ValueAnimator();
    }
    //anim->appendChangingConfigurations(arrayAnimator.getChangingConfigurations());

    parseAnimatorFromTypeArray(anim,attrs, pathErrorScale);

    const std::string resID = attrs.getString("interpolator");
    if (!resID.empty()) {
        Interpolator* interpolator = AnimationUtils::loadInterpolator(context, resID);
        /*if (interpolator instanceof BaseInterpolator) {
            anim.appendChangingConfigurations(((BaseInterpolator) interpolator).getChangingConfiguration());
        }*/
        anim->setInterpolator(interpolator);
    }

    /*arrayAnimator.recycle();
    if (arrayObjectAnimator != null) {
        arrayObjectAnimator.recycle();
    }*/
    return anim;
}

ValueAnimator*  AnimatorInflater::loadValueAnimator(Context*context,const AttributeSet& atts, ValueAnimator*anim,float){
    const int valueType = atts.getInt("valueType",std::unordered_map<std::string,int>{
            {"intType",(int)Property::INT_CLASS},
            {"floatType",(int)Property::FLOAT_CLASS},
            {"colorType",(int)Property::COLOR_CLASS},
            {"pathType",(int)Property::PATH_CLASS}
        },(int)Property::UNDEFINED);

    const std::string propertyName = atts.getString("propertyName");
    const std::string intpResource = atts.getString("interpolator");
    Interpolator* interpolator= nullptr;
    if(!intpResource.empty()){
        AnimationUtils::loadInterpolator(context,intpResource);
    }
    if(anim==nullptr){
        anim = new ValueAnimator();
    }
    if(interpolator){
        anim->setInterpolator(interpolator);
    }
    anim->setDuration(atts.getInt("duration",300));
    anim->setStartDelay(atts.getInt("startOffset",0));
    anim->setRepeatCount(atts.getInt("repeatCount",0));
    anim->setRepeatMode(atts.getInt("repeatMode",std::unordered_map<std::string,int>{
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
