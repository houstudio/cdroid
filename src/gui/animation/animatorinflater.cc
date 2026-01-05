/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <animation/animatorset.h>
#include <animation/animatorinflater.h>
#include <animation/animationutils.h>
#include <core/typedvalue.h>
#include <drawable/pathparser.h>
#include <porting/cdlog.h>

namespace cdroid{

Animator* AnimatorInflater::loadAnimator(Context* context,const std::string&resid){
    return loadAnimator(context,resid,1.f);
}
#if 1
Animator* AnimatorInflater::loadAnimator(Context* context,const std::string&resid,float){
    XmlPullParser parser(context,resid);
    Animator* animator = createAnimatorFromXml(context, parser, 1.f/*pathErrorScale*/);
    return animator;
}

static std::unordered_map<std::string,std::shared_ptr<StateListAnimator>>mStateAnimatorMap;
StateListAnimator* AnimatorInflater::loadStateListAnimator(Context* context,const std::string&resid){
    auto it = mStateAnimatorMap.find(resid);
    if(it==mStateAnimatorMap.end()){
        XmlPullParser parser(context,resid);
        const AttributeSet& attrs = parser;
        StateListAnimator*anim =createStateListAnimatorFromXml(context,parser,attrs);
        it = mStateAnimatorMap.insert({resid,std::shared_ptr<StateListAnimator>(anim)}).first;
    }
    return new StateListAnimator(*it->second);
}
#else
static std::unordered_map<std::string,std::shared_ptr<ConstantState<Animator*>>>mAnimatorCache;
Animator* AnimatorInflater::loadAnimator(Context* context,const std::string&resid,float){
    XmlPullParser parser(context,resid);
    Animator* animator = nullptr;
    auto itc = mAnimatorCache.find(resid);
    if(itc!= mAnimatorCache.end()){
        animator=itc->second->newInstance();
    } else{
        animator = createAnimatorFromXml(context, parser, 1.f/*pathErrorScale*/);
        if (animator != nullptr) {
            auto  constantState = animator->createConstantState();
            if (constantState != nullptr) {
                LOGD("caching animator for res %s",resid.c_str());
                mAnimatorCache.insert({resid, constantState});
                // create a new animator so that cached version is never used by the user
                animator = constantState->newInstance();//resources, theme);
            }
        }
    }
    return animator;
}

static std::unordered_map<std::string,std::shared_ptr<ConstantState<StateListAnimator*>>>mStateAnimatorMap;
StateListAnimator* AnimatorInflater::loadStateListAnimator(Context* context,const std::string&resid){
    auto itc = mStateAnimatorMap.find(resid);
    StateListAnimator* animator =nullptr;
    if(itc!=mStateAnimatorMap.end()){
        animator = itc->second->newInstance();
        LOGD("load %s from StateAnimatorCache",resid.c_str());
    }else{
        XmlPullParser parser(context,resid);
        const AttributeSet attrs(&parser);
        animator =createStateListAnimatorFromXml(context,parser,attrs);
        auto constantState = animator->createConstantState();
        mStateAnimatorMap.insert({resid,constantState});
    }
    return animator;//new StateListAnimator(*it->second);
}
#endif
Animator* AnimatorInflater::createAnimatorFromXml(Context*context,XmlPullParser& parser,float pixelSize){
    const AttributeSet& attrs = parser;
    return createAnimatorFromXml(context,parser, attrs, nullptr, 0,pixelSize);
}

Animator* AnimatorInflater::createAnimatorFromXml(Context*context,XmlPullParser&parser,const AttributeSet& attrs,
        AnimatorSet*parent,int sequenceOrdering,float pixelSize){
     Animator* anim = nullptr;
     std::vector<Animator*> childAnims;

    // Make sure we are on a start tag.
    int type = 0,depth = 0;
    const int innerDepth = parser.getDepth()+1;
    while ((((type = parser.next()) != XmlPullParser::END_TAG) || (parser.getDepth() >= innerDepth))
            && (type != XmlPullParser::END_DOCUMENT) && (type != XmlPullParser::BAD_DOCUMENT) ) {

        if (type != XmlPullParser::START_TAG) {
            continue;
        }

        std::string name = parser.getName();
        bool gotValues = false;
        if (name.compare("objectAnimator")==0) {
            anim = loadObjectAnimator(context,attrs, pixelSize);
        } else if (name.compare("animator")==0) {
            anim = loadAnimator(context, attrs, nullptr, pixelSize);
        } else if (name.compare("set")==0) {
            anim = new AnimatorSet();
            const int ordering = attrs.getInt("ordering",std::unordered_map<std::string,int>{
                    {"together",(int)TOGETHER},{"sequentially",(int)SEQUENTIALLY}}, TOGETHER);
            createAnimatorFromXml(context, parser, attrs, (AnimatorSet*) anim, ordering,pixelSize);
        } else if (name.compare("propertyValuesHolder")==0) {
            std::vector<PropertyValuesHolder*>values = loadValues(parser,attrs);
            if (values.size() && (dynamic_cast<ValueAnimator*>(anim))) {
                ((ValueAnimator*) anim)->setValues(values);
            }
            gotValues = true;
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

StateListAnimator* AnimatorInflater::createStateListAnimatorFromXml(Context*context,XmlPullParser&parser,const AttributeSet&attrs){
    StateListAnimator* stateListAnimator = new StateListAnimator();
    while (true) {
        const int type = parser.next();
        const std::string name =parser.getName();
        switch (type) {
        case XmlPullParser::END_DOCUMENT:
        case XmlPullParser::END_TAG:  return stateListAnimator;
        case XmlPullParser::START_TAG:// parse item
            if (name.compare("item")==0) {
                std::vector<int>states;
                Animator* animator = nullptr;
                StateSet::parseState(states,attrs);
                std::string animId = attrs.getString("animator");
                if(!animId.empty()){
                    animator = loadAnimator(context, animId);
                }else{
                    animator = createAnimatorFromXml(context,parser,attrs, nullptr, 0,1.f);
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
 
std::vector<PropertyValuesHolder*> AnimatorInflater::loadValues(XmlPullParser& parser,const  AttributeSet& attrs){
    std::vector<PropertyValuesHolder*> values;
    int type = XmlPullParser::START_TAG;
    while ((type != XmlPullParser::END_TAG) && (type != XmlPullParser::END_DOCUMENT)) {
        if (type != XmlPullParser::START_TAG) {
            type = parser.next();
            continue;
        }
        std::string name = parser.getName();
        if (name.compare("propertyValuesHolder")==0) {
            const std::string propertyName = attrs.getString("propertyName");
            const int valueType = attrs.getInt("valueType",std::unordered_map<std::string,int>{
                 {"intType", (int)Property::INT_TYPE},    {"colorType",(int)Property::COLOR_TYPE},
                 {"floatType",(int)Property::FLOAT_TYPE}, {"pathType",(int)Property::PATH_TYPE}},
                 Property::UNDEFINED);
            LOGD("propertyValuesHolder.%s type=%d",propertyName.c_str(),valueType);
            PropertyValuesHolder* pvh = loadPvh(parser, propertyName, valueType);
            if (pvh == nullptr) {
                pvh = getPVH(attrs, valueType,propertyName);
            }
            if (pvh != nullptr) {
                values.push_back(pvh);
            }
        }
        type = parser.next();

    }
    return values;
}

PropertyValuesHolder* AnimatorInflater::loadPvh(XmlPullParser& parser,const std::string& propertyName, int valueType){
    int type;
    PropertyValuesHolder* value = nullptr;
#if 0
    ArrayList<Keyframe> keyframes = null;

    while ((type = parser.next()) != XmlPullParser::END_TAG &&
            type != XmlPullParser::END_DOCUMENT) {
        const std::string name = parser.getName();
        if (name.compare("keyframe")==0) {
            if (valueType == VALUE_TYPE_UNDEFINED) {
                valueType = inferValueTypeOfKeyframe(res, theme, Xml.asAttributeSet(parser));
            }
            Keyframe keyframe = loadKeyframe(res, theme, Xml.asAttributeSet(parser), valueType);
            if (keyframe != null) {
                if (keyframes == null) {
                    keyframes = new ArrayList<Keyframe>();
                }
                keyframes.add(keyframe);
            }
            parser.next();
        }
    }

    int count;
    if (keyframes != null && (count = keyframes.size()) > 0) {
        // make sure we have keyframes at 0 and 1
        // If we have keyframes with set fractions, add keyframes at start/end
        // appropriately. If start/end have no set fractions:
        // if there's only one keyframe, set its fraction to 1 and add one at 0
        // if >1 keyframe, set the last fraction to 1, the first fraction to 0
        Keyframe firstKeyframe = keyframes.get(0);
        Keyframe lastKeyframe = keyframes.get(count - 1);
        float endFraction = lastKeyframe.getFraction();
        if (endFraction < 1) {
            if (endFraction < 0) {
                lastKeyframe.setFraction(1);
            } else {
                keyframes.add(keyframes.size(), createNewKeyframe(lastKeyframe, 1));
                ++count;
            }
        }
        float startFraction = firstKeyframe.getFraction();
        if (startFraction != 0) {
            if (startFraction < 0) {
                firstKeyframe.setFraction(0);
            } else {
                keyframes.add(0, createNewKeyframe(firstKeyframe, 0));
                ++count;
            }
        }
        Keyframe[] keyframeArray = new Keyframe[count];
        keyframes.toArray(keyframeArray);
        for (int i = 0; i < count; ++i) {
            Keyframe keyframe = keyframeArray[i];
            if (keyframe.getFraction() < 0) {
                if (i == 0) {
                    keyframe.setFraction(0);
                } else if (i == count - 1) {
                    keyframe.setFraction(1);
                } else {
                    // figure out the start/end parameters of the current gap
                    // in fractions and distribute the gap among those keyframes
                    int startIndex = i;
                    int endIndex = i;
                    for (int j = startIndex + 1; j < count - 1; ++j) {
                        if (keyframeArray[j].getFraction() >= 0) {
                            break;
                        }
                        endIndex = j;
                    }
                    float gap = keyframeArray[endIndex + 1].getFraction() -
                            keyframeArray[startIndex - 1].getFraction();
                    distributeKeyframes(keyframeArray, gap, startIndex, endIndex);
                }
            }
        }
        value = PropertyValuesHolder.ofKeyframe(propertyName, keyframeArray);
        if (valueType == VALUE_TYPE_COLOR) {
            value.setEvaluator(ArgbEvaluator.getInstance());
        }
    }
#endif
    return value;
}

static const std::unordered_map<std::string,int>valueTypes = {
    {"alpha",(int)Property::FLOAT_TYPE},
    {"bottom",(int)Property::INT_TYPE},
    {"left",(int)Property::INT_TYPE},
    {"elevation",(int)Property::FLOAT_TYPE},
    {"pivotX",(int)Property::FLOAT_TYPE},
    {"pivotY",(int)Property::FLOAT_TYPE},
    {"right",(int)Property::INT_TYPE},
    {"rotation",(int)Property::FLOAT_TYPE},
    {"rotationX",(int)Property::FLOAT_TYPE},
    {"rotationY",(int)Property::FLOAT_TYPE},
    {"scaleX",(int)Property::FLOAT_TYPE},
    {"scaleY",(int)Property::FLOAT_TYPE},
    {"scrollX",(int)Property::INT_TYPE},
    {"scrollY",(int)Property::INT_TYPE},
    {"top",(int)Property::INT_TYPE},

    {"translateX",(int)Property::FLOAT_TYPE},
    {"translateY",(int)Property::FLOAT_TYPE},

    {"translationX",(int)Property::FLOAT_TYPE},
    {"translationY",(int)Property::FLOAT_TYPE},
    {"translationZ",(int)Property::FLOAT_TYPE},
    {"x",(int)Property::FLOAT_TYPE},
    {"y",(int)Property::FLOAT_TYPE},
    {"z",(int)Property::FLOAT_TYPE},
////////////////////////////////////////////////////////////////
    {"strokeWidth",(int)Property::FLOAT_TYPE},
    {"strokeColor",(int)Property::COLOR_TYPE},
    {"strokeAlpha",(int)Property::FLOAT_TYPE},
    {"fillColor",(int)Property::COLOR_TYPE},
    {"fillAlpha",(int)Property::FLOAT_TYPE},
    {"pathData",(int)Property::PATH_TYPE},
    {"trimPathStart",(int)Property::FLOAT_TYPE},
    {"trimPathEnd",(int)Property::FLOAT_TYPE},
    {"trimPathOffset",(int)Property::FLOAT_TYPE}
};

int AnimatorInflater::inferValueTypeFromPropertyName(const AttributeSet&atts, const std::string& propertyName) {
    const int valueType = atts.getInt("valueType",std::unordered_map<std::string,int>{
         {"intType", (int)Property::INT_TYPE},
         {"colorType",(int)Property::COLOR_TYPE},
         {"floatType",(int)Property::FLOAT_TYPE},
         {"pathType",(int)Property::PATH_TYPE}
         }, Property::UNDEFINED);
    if(valueType==Property::UNDEFINED){
        auto it = valueTypes.find(propertyName);
        if(it != valueTypes.end()) return it->second;
        return Property::UNDEFINED;
    }
    return valueType;
}

PropertyValuesHolder*AnimatorInflater::getPVH(const AttributeSet&atts, int valueType,const std::string& propertyName){
    PropertyValuesHolder* returnValue = nullptr;
    const std::string sFrom = atts.getString("valueFrom");
    const std::string sTo = atts.getString("valueTo");
    const bool hasFrom = !sFrom.empty();
    const bool hasTo   = !sTo.empty();
    const int fromType = inferValueTypeFromPropertyName(atts,propertyName);
    const int toType = fromType;
    const bool getFloats = (valueType==Property::FLOAT_TYPE)||(fromType==Property::FLOAT_TYPE);

    if (valueType == Property::PATH_TYPE) {
        const std::string fromString = atts.getString("valueFrom");
        const std::string toString = atts.getString("valueTo");
        PathParser::PathData nodesFrom = fromString.empty() ? PathParser::PathData() : PathParser::PathData(fromString);
        PathParser::PathData nodesTo = toString.empty()  ? PathParser::PathData() : PathParser::PathData(toString);

        if (fromString.size() || toString.size()) {
            if (fromString.size()) {
                PathParser::PathData nodesFrom(fromString);
                if (toString.size()) {
                    PathParser::PathData nodesTo(toString);
                    if (!PathParser::canMorph(nodesFrom, nodesTo)) {
                        throw std::runtime_error(std::string(" Can't morph from") + fromString + " to " + toString);
                    }
                    returnValue = PropertyValuesHolder::ofObject(propertyName, {nodesFrom, nodesTo});
                } else {
                    returnValue = PropertyValuesHolder::ofObject(propertyName, {nodesFrom});
                }
            } else if (toString.size()) {
                PathParser::PathData nodesTo(toString);
                returnValue = PropertyValuesHolder::ofObject(propertyName,{nodesTo});
            }
            if(returnValue)returnValue->setEvaluator(PropertyValuesHolder::PathDataEvaluator);
        }
    } else {
        TypeEvaluator evaluator = nullptr;
        // Integer and float value types are handled here.
        if ((fromType == Property::COLOR_TYPE)||(toType==Property::COLOR_TYPE)) {
            // special case for colors: ignore valueType and get ints
            evaluator = PropertyValuesHolder::ArgbEvaluator;
        }
        if (getFloats) {
            float valueFrom,valueTo;
            if (hasFrom) {
                if(fromType==Property::INT_TYPE) {/*TypedValue::TYPE_DIMENSION*/
                    valueFrom = atts.getDimension("valueFrom", 0);
                }else{
                    valueFrom = atts.getFloat("valueFrom",0);
                }
                if (hasTo) {
                    if(toType==Property::INT_TYPE)/*TypedValue::TYPE_DIMENSION*/
                        valueTo = atts.getDimension("valueTo", 0);
                    else
                        valueTo = atts.getFloat("valueTo",0);
                    returnValue = PropertyValuesHolder::ofFloat(propertyName,{valueFrom, valueTo});
                } else {
                    returnValue = PropertyValuesHolder::ofFloat(propertyName,{valueFrom});
                }
            } else {
                if(toType==Property::INT_TYPE)/*TypedValue::TYPE_DIMENSION*/
                    valueTo = atts.getDimension("valueTo", 0);
                else
                    valueTo = atts.getFloat("valueTo",0);
                returnValue = PropertyValuesHolder::ofFloat(propertyName, {valueTo});
            }
        } else {
            int valueFrom,valueTo;
            if (hasFrom) {
                if (fromType == Property::INT_TYPE) {/*TypedValue::TYPE_DIMENSION*/
                    valueFrom = (int) atts.getDimension("valueFrom", 0);
                } else if (fromType==Property::COLOR_TYPE) {
                    valueFrom = atts.getColor("valueFrom", 0);
                } else {
                    valueFrom = atts.getInt("valueFrom", 0);
                }
                if (hasTo) {
                    if (toType == Property::INT_TYPE) {/*TypedValue::TYPE_DIMENSION*/
                        valueTo = (int) atts.getDimension("valueTo", 0);
                    } else if (toType==Property::COLOR_TYPE) {
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
                    if (toType == Property::INT_TYPE) {/*TypedValue::TYPE_DIMENSION*/
                        valueTo = (int) atts.getDimension("valueTo", 0);
                    } else if (toType==Property::COLOR_TYPE) {
                        valueTo = atts.getColor("valueTo", 0);
                    } else {
                        valueTo = atts.getInt("valueTo", 0);
                    }
                    returnValue = PropertyValuesHolder::ofInt(propertyName, {valueTo});
                }
            }
        }
        if (returnValue != nullptr && evaluator != nullptr) {
            returnValue->setEvaluator(evaluator);
        }
    }
    return returnValue;
}

void AnimatorInflater::parseAnimatorFromTypeArray(ValueAnimator* anim,const AttributeSet&atts, float pixelSize) {
    const long duration = atts.getInt("duration", 300);
    const long startDelay = atts.getInt("startOffset", 0);
    const std::string propertyName = atts.getString("propertyName");

    const int valueType = inferValueTypeFromPropertyName(atts,propertyName);

    PropertyValuesHolder* pvh = getPVH(atts, valueType,propertyName);
    if (pvh != nullptr) {
        anim->setValues({pvh});
    }

    anim->setDuration(duration);
    anim->setStartDelay(startDelay);

    if (atts.hasAttribute("repeatCount")) {
        anim->setRepeatCount(atts.getInt("repeatCount", ValueAnimator::INFINITE));
    }
    if (atts.hasAttribute("repeatMode")) {
        anim->setRepeatMode(atts.getInt("repeatMode",std::unordered_map<std::string,int>{
                    {"restart",(int)ValueAnimator::RESTART},
                    {"reverse",(int)ValueAnimator::REVERSE}
            },ValueAnimator::RESTART));
    }

    /*if (arrayObjectAnimator != nullptr) {
        setupObjectAnimator(anim, arrayObjectAnimator, valueType, pixelSize);
    }*/
    if((propertyName.empty()==false)&&dynamic_cast<ObjectAnimator*>(anim)){
       ((ObjectAnimator*)anim)->setPropertyName(propertyName);
    }
}

TypeEvaluator AnimatorInflater::setupAnimatorForPath(ValueAnimator* anim,const AttributeSet&arrayAnimator){
    TypeEvaluator evaluator = nullptr;
    const std::string fromString = arrayAnimator.getString("valueFrom");
    const std::string toString = arrayAnimator.getString("valueTo");

    if (!fromString.empty()) {//pathDataFrom != null) {
        PathParser::PathData pathDataFrom (fromString);
        if (!toString.empty()) {//pathDataTo != null) {
            PathParser::PathData pathDataTo(toString);
            //anim->setObjectValues(pathDataFrom, pathDataTo);
            if (!PathParser::canMorph(pathDataFrom, pathDataTo)) {
                throw std::runtime_error(//arrayAnimator.getPositionDescription()
                        " Can't morph from " + fromString + " to " + toString);
            }
        } else {
            //anim->setObjectValues((Object)pathDataFrom);
        }
        //evaluator = new PathDataEvaluator();
    } else if (!toString.empty()){//pathDataTo != null) {
        PathParser::PathData pathDataTo(toString);
        //anim->setObjectValues((Object)pathDataTo);
        //evaluator = new PathDataEvaluator();
    }

    LOGV_IF(evaluator!=nullptr,"create a new PathDataEvaluator here");

    return evaluator;
}

void AnimatorInflater::setupObjectAnimator(ValueAnimator* anim, const AttributeSet&arrayObjectAnimator,int valueType, float pixelSize){
    ObjectAnimator* oa = (ObjectAnimator*) anim;
    std::string pathData = arrayObjectAnimator.getString("pathData");
    // Path can be involved in an ObjectAnimator in the following 3 ways:
    // 1) Path morphing: the property to be animated is pathData, and valueFrom and valueTo
    //    are both of pathType. valueType = pathType needs to be explicitly defined.
    // 2) A property in X or Y dimension can be animated along a path: the property needs to be
    //    defined in propertyXName or propertyYName attribute, the path will be defined in the
    //    pathData attribute. valueFrom and valueTo will not be necessary for this animation.
    // 3) PathInterpolator can also define a path (in pathData) for its interpolation curve.
    // Here we are dealing with case 2:
    if (!pathData.empty()) {
        std::string propertyXName = arrayObjectAnimator.getString("propertyXName");
        std::string propertyYName = arrayObjectAnimator.getString("propertyYName");

        if (valueType == VALUE_TYPE_PATH || valueType == VALUE_TYPE_UNDEFINED) {
            // When pathData is defined, we are in case #2 mentioned above. ValueType can only
            // be float type, or int type. Otherwise we fallback to default type.
            valueType = VALUE_TYPE_FLOAT;
        }
#if 0
        if (propertyXName.empty() && propertyYName.empty()) {
            throw std::runtime_error(//arrayObjectAnimator.getPositionDescription()
                    " propertyXName or propertyYName is needed for PathData");
        } else {
            auto path = PathParser::createPathFromPathData(pathData);
            float error = 0.5f * pixelSize; // max half a pixel error
            PathKeyframes keyframeSet = KeyframeSet.ofPath(path, error);
            Keyframes xKeyframes;
            Keyframes yKeyframes;
            if (valueType == VALUE_TYPE_FLOAT) {
                xKeyframes = keyframeSet.createXFloatKeyframes();
                yKeyframes = keyframeSet.createYFloatKeyframes();
            } else {
                xKeyframes = keyframeSet.createXIntKeyframes();
                yKeyframes = keyframeSet.createYIntKeyframes();
            }
            PropertyValuesHolder* x = nullptr;
            PropertyValuesHolder* y = nullptr;
            if (!propertyXName.empty()) {
                x = PropertyValuesHolder::ofKeyframes(propertyXName, xKeyframes);
            }
            if (!propertyYName.empty()) {
                y = PropertyValuesHolder::ofKeyframes(propertyYName, yKeyframes);
            }
            if (x == nullptr) {
                oa->setValues({y});
            } else if (y == nullptr) {
                oa->setValues({x});
            } else {
                oa->setValues({x, y});
            }
        }
#endif
    } else {
        std::string propertyName = arrayObjectAnimator.getString("propertyName");
        oa->setPropertyName(propertyName);
    }
}

ObjectAnimator* AnimatorInflater::loadObjectAnimator(Context*ctx,const AttributeSet& atts,float){
    ObjectAnimator*anim = new ObjectAnimator();
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
    return anim;
}

ValueAnimator*  AnimatorInflater::loadValueAnimator(Context*context,const AttributeSet& atts, ValueAnimator*anim,float){
    const int valueType = atts.getInt("valueType",std::unordered_map<std::string,int>{
            {"intType",(int)Property::INT_TYPE},
            {"floatType",(int)Property::FLOAT_TYPE},
            {"colorType",(int)Property::COLOR_TYPE},
            {"pathType",(int)Property::PATH_TYPE}
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
