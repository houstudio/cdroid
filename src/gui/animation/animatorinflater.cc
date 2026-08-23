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
#include <core/context.h>
#include <core/typedarray.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
using namespace cdroid::internal;

namespace cdroid{

// AOSP AnimatorInflater private obtainAttributes(res, theme, set, attrs).
std::unique_ptr<TypedArray> AnimatorInflater::obtainAttributes(Context*ctx,const Resources::Theme* theme,
        const AttributeSet& set,const uint32_t* attrs){
    if (theme) return theme->obtainStyledAttributes(&set, attrs);
    return ctx->obtainStyledAttributes(set, attrs);
}

Animator* AnimatorInflater::loadAnimator(Context* context,const std::string&resid){
    return loadAnimator(context,resid,1.f);
}
Animator* AnimatorInflater::loadAnimator(Context* context,const std::string&resid,float pathErrorScale){
    // AOSP loadAnimator(Context, id) → loadAnimator(res, context.getTheme(), id).
    Resources::Theme theme = context->getTheme();
    return loadAnimator(context, &theme, resid, pathErrorScale);
}

Animator* AnimatorInflater::loadAnimator(Context* context,const Resources::Theme* theme,
        const std::string&resid,float pathErrorScale){
    XmlPullParser parser(context,resid);
    return createAnimatorFromXml(context, theme, parser, pathErrorScale);
}

Animator* AnimatorInflater::loadAnimator(Context* context,int resid){
    return loadAnimator(context,resid,1.f);
}

Animator* AnimatorInflater::loadAnimator(Context* context,int resid,float){
    if (resid == 0) return nullptr;  // AOSP: 0 → null
    // AOSP loadAnimator(Resources, Theme, id, pathErrorScale): the
    // ConfigurationBoundResourceCache on ResourcesImpl serves hits as
    // newInstance() — the cached source animator is never handed out.
    Resources& res = context->getResources();
    Resources::Theme theme = context->getTheme();
    Animator* animator = res.obtainCachedAnimator(resid, theme._engineHandle());
    if (animator != nullptr) return animator;
    XmlPullParser parser(context,resid);
    animator = createAnimatorFromXml(context, &theme, parser, 1.f);
    if (animator != nullptr) {
        // AOSP appends getChangingConfigs(resources, id) so entries self-invalidate
        // via needNewResources; CDROID clears the whole cache on configuration
        // change (ResourcesImpl::updateConfiguration), making per-entry configs
        // unnecessary. createConstantState() transfers ownership of the parsed
        // animator to the constant state (AOSP relies on GC).
        const auto constantState = animator->createConstantState();
        if (constantState != nullptr) {
            res.cacheAnimator(resid, theme._engineHandle(), constantState);
            // create a new animator so that cached version is never used by the user
            animator = constantState->newInstance();
        }
    }
    return animator;
}

Animator* AnimatorInflater::loadAnimator(Context* context,const Resources::Theme* theme,int resid,float pathErrorScale){
    if (resid == 0) return nullptr;  // AOSP: 0 → null
    XmlPullParser parser(context,resid);
    return createAnimatorFromXml(context, theme, parser, pathErrorScale);
}

StateListAnimator* AnimatorInflater::loadStateListAnimator(Context* context,const std::string&resid){
    // String-resid entry (CDROID extension; AOSP loads by @AnimatorRes int id):
    // no caching — the themed animator cache is keyed by int resource id.
    XmlPullParser parser(context,resid);
    const AttributeSet& attrs = parser;
    Resources::Theme theme = context->getTheme();
    return createStateListAnimatorFromXml(context,&theme,parser,attrs);
}

StateListAnimator* AnimatorInflater::loadStateListAnimator(Context* context,int resid){
    if (resid == 0) return nullptr;  // AOSP: 0 → null
    // AOSP loadStateListAnimator(Context, id): ConfigurationBoundResourceCache
    // on ResourcesImpl; hits come back as newInstance() (a clone).
    Resources& res = context->getResources();
    Resources::Theme theme = context->getTheme();
    StateListAnimator* animator = res.obtainCachedStateListAnimator(resid, theme._engineHandle());
    if (animator != nullptr) return animator;
    XmlPullParser parser(context, resid);
    const AttributeSet& attrs = parser;
    animator = createStateListAnimatorFromXml(context, &theme, parser, attrs);
    if (animator != nullptr) {
        // changing-configs per entry unnecessary — see loadAnimator(Context, int).
        const auto constantState = animator->createConstantState();
        if (constantState != nullptr) {
            res.cacheStateListAnimator(resid, theme._engineHandle(), constantState);
            // return a clone so that the animator in constant state is never used.
            animator = constantState->newInstance();
        }
    }
    return animator;
}
Animator* AnimatorInflater::createAnimatorFromXml(Context*context,const Resources::Theme* theme,XmlPullParser& parser,float pixelSize){
    const AttributeSet& attrs = parser;
    return createAnimatorFromXml(context,theme,parser, attrs, nullptr, 0,pixelSize);
}

Animator* AnimatorInflater::createAnimatorFromXml(Context*context,const Resources::Theme* theme,XmlPullParser&parser,const AttributeSet& attrs,
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
            anim = loadObjectAnimator(context,theme,attrs, pixelSize);
        } else if (name.compare("animator")==0) {
            anim = loadAnimator(context, theme, attrs, nullptr, pixelSize);
        } else if (name.compare("set")==0) {
            anim = new AnimatorSet();
            auto ta = obtainAttributes(context, theme, attrs, R::styleable::AnimatorSet);
            const int ordering = ta->getInt(R::styleable::AnimatorSet_ordering, TOGETHER);
            createAnimatorFromXml(context, theme, parser, attrs, (AnimatorSet*) anim, ordering,pixelSize);
        } else if (name.compare("propertyValuesHolder")==0) {
            std::vector<PropertyValuesHolder*>values = loadValues(context,theme,parser,attrs);
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

StateListAnimator* AnimatorInflater::createStateListAnimatorFromXml(Context*context,const Resources::Theme* theme,XmlPullParser&parser,const AttributeSet&attrs){
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
                // AOSP: scan the item's attrs for android:animation (@animator/... ref)
                const int attributeCount = (int)attrs.getAttributeCount();
                for (int i = 0; i < attributeCount; i++) {
                    if (attrs.getAttributeNameResource(i) == R::attr::animation) {
                        animator = loadAnimator(context, theme, attrs.getAttributeResourceValue(i, 0), 1.f);
                        break;
                    }
                }
                if (animator == nullptr) {
                    animator = createAnimatorFromXml(context,theme,parser,attrs, nullptr, 0,1.f);
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
 
std::vector<PropertyValuesHolder*> AnimatorInflater::loadValues(Context*ctx,const Resources::Theme* theme,XmlPullParser& parser,const AttributeSet& attrs){
    std::vector<PropertyValuesHolder*> values;
    int type = XmlPullParser::START_TAG;
    while ((type != XmlPullParser::END_TAG) && (type != XmlPullParser::END_DOCUMENT)) {
        if (type != XmlPullParser::START_TAG) {
            type = parser.next();
            continue;
        }
        std::string name = parser.getName();
        if (name.compare("propertyValuesHolder")==0) {
            auto ta = obtainAttributes(ctx, theme, attrs, R::styleable::PropertyValuesHolder);
            const std::string propertyName = ta->getString(R::styleable::PropertyValuesHolder_propertyName);
            const int valueType = ta->getInt(R::styleable::PropertyValuesHolder_valueType, VALUE_TYPE_UNDEFINED);
            LOGD("propertyValuesHolder.%s type=%d",propertyName.c_str(),valueType);
            PropertyValuesHolder* pvh = loadPvh(parser, propertyName, valueType);
            if (pvh == nullptr) {
                pvh = getPVH(ctx, theme, attrs, valueType, propertyName);
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

// (The old propertyName→valueType map was a text-XML shim — binary values
// carry their own type; AOSP infers from the raw TypedValues.)

int AnimatorInflater::inferValueTypeFromType(const TypedValue& tv) {
    // AOSP AnimatorInflater.inferValueTypeFromType, verbatim.
    if (tv.type == TypedValue::TYPE_FLOAT) {
        return VALUE_TYPE_FLOAT;
    } else if (tv.type >= TypedValue::TYPE_FIRST_INT && tv.type <= TypedValue::TYPE_LAST_INT) {
        // Disregard the actual int type for now.
        return VALUE_TYPE_INT;
    } else if (tv.type == TypedValue::TYPE_STRING) {
        return VALUE_TYPE_PATH;
    }
    return VALUE_TYPE_UNDEFINED;
}

int AnimatorInflater::inferValueTypeFromValues(const TypedArray& a, int valueFromId, int valueToId) {
    // AOSP AnimatorInflater.inferValueTypeFromValues: prefer valueFrom's type.
    TypedValue tv;
    if (a.peekValue(valueFromId, &tv)) {
        return inferValueTypeFromType(tv);
    }
    if (a.peekValue(valueToId, &tv)) {
        return inferValueTypeFromType(tv);
    }
    return VALUE_TYPE_UNDEFINED;
}

PropertyValuesHolder*AnimatorInflater::getPVH(Context*ctx, const Resources::Theme* theme,const AttributeSet&atts, int valueType,const std::string& propertyName){
    // AOSP AnimatorInflater.getPVH: typed face — hasValue presence, getFloat/
    // getInt reads (both coerce across int/float), VALUE_TYPE_* space. The old
    // string probing (sFrom/sTo via getString) lost every typed value.
    auto ta = obtainAttributes(ctx, theme, atts, R::styleable::PropertyValuesHolder);
    PropertyValuesHolder* returnValue = nullptr;
    const bool hasFrom = ta->hasValue(R::styleable::PropertyValuesHolder_valueFrom);
    const bool hasTo   = ta->hasValue(R::styleable::PropertyValuesHolder_valueTo);

    if (valueType == VALUE_TYPE_UNDEFINED) {
        // Infer the value type if unspecified.
        valueType = inferValueTypeFromValues(*ta, R::styleable::PropertyValuesHolder_valueFrom,
                R::styleable::PropertyValuesHolder_valueTo);
        if (valueType == VALUE_TYPE_UNDEFINED) {
            // Not enough information to infer the value type.
            return returnValue;
        }
    }

    const bool getFloats = (valueType == VALUE_TYPE_FLOAT);

    if (valueType == VALUE_TYPE_PATH) {
        const std::string fromString = ta->getString(R::styleable::PropertyValuesHolder_valueFrom);
        const std::string toString = ta->getString(R::styleable::PropertyValuesHolder_valueTo);
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
        if (valueType == VALUE_TYPE_COLOR) {
            // special case for colors: ignore valueType and get ints
            evaluator = PropertyValuesHolder::ArgbEvaluator;
        }
        if (getFloats) {
            float valueFrom = 0, valueTo = 0;
            if (hasFrom) {
                valueFrom = ta->getFloat(R::styleable::PropertyValuesHolder_valueFrom, 0);
            }
            if (hasTo) {
                valueTo = ta->getFloat(R::styleable::PropertyValuesHolder_valueTo, 0);
            }
            if (hasFrom && hasTo) {
                returnValue = PropertyValuesHolder::ofFloat(propertyName, {valueFrom, valueTo});
            } else if (hasFrom) {
                returnValue = PropertyValuesHolder::ofFloat(propertyName, {valueFrom});
            } else if (hasTo) {
                returnValue = PropertyValuesHolder::ofFloat(propertyName, {valueTo});
            }
        } else {
            int valueFrom = 0, valueTo = 0;
            if (hasFrom) {
                valueFrom = ta->getInt(R::styleable::PropertyValuesHolder_valueFrom, 0);
            }
            if (hasTo) {
                valueTo = ta->getInt(R::styleable::PropertyValuesHolder_valueTo, 0);
            }
            if (hasFrom && hasTo) {
                returnValue = PropertyValuesHolder::ofInt(propertyName, {valueFrom, valueTo});
            } else if (hasFrom) {
                returnValue = PropertyValuesHolder::ofInt(propertyName, {valueFrom});
            } else if (hasTo) {
                returnValue = PropertyValuesHolder::ofInt(propertyName, {valueTo});
            }
        }
        if (returnValue != nullptr && evaluator != nullptr) {
            returnValue->setEvaluator(evaluator);
        }
    }
    return returnValue;
}

void AnimatorInflater::parseAnimatorFromTypeArray(Context*ctx, const Resources::Theme* theme, ValueAnimator* anim,const AttributeSet&atts, float pixelSize) {
    auto ta = obtainAttributes(ctx, theme, atts, R::styleable::Animator);
    const long duration = ta->getInt(R::styleable::Animator_duration, 300);
    const long startDelay = ta->getInt(R::styleable::Animator_startOffset, 0);
    // propertyName is in PropertyAnimator styleable, not Animator; read via PVH styleable.
    std::string propertyName;
    { auto ta2 = obtainAttributes(ctx, theme, atts, R::styleable::PropertyValuesHolder);
      propertyName = ta2->getString(R::styleable::PropertyValuesHolder_propertyName);
    }

    // AOSP: valueType from the attr; if unspecified, infer from valueFrom/valueTo.
    int valueType = ta->getInt(R::styleable::Animator_valueType, VALUE_TYPE_UNDEFINED);
    if (valueType == VALUE_TYPE_UNDEFINED) {
        valueType = inferValueTypeFromValues(*ta, R::styleable::Animator_valueFrom,
                R::styleable::Animator_valueTo);
    }

    PropertyValuesHolder* pvh = getPVH(ctx,theme,atts, valueType,propertyName);
    if (pvh != nullptr) {
        anim->setValues({pvh});
    }

    anim->setDuration(duration);
    anim->setStartDelay(startDelay);

    anim->setRepeatCount(ta->getInt(R::styleable::Animator_repeatCount, 0));
    anim->setRepeatMode(ta->getInt(R::styleable::Animator_repeatMode, ValueAnimator::RESTART));

    if((propertyName.empty()==false)&&dynamic_cast<ObjectAnimator*>(anim)){
       ((ObjectAnimator*)anim)->setPropertyName(propertyName);
    }
}

TypeEvaluator AnimatorInflater::setupAnimatorForPath(Context*ctx, const Resources::Theme* theme, ValueAnimator* anim,const AttributeSet&arrayAnimator){
    TypeEvaluator evaluator = nullptr;
    auto ta = obtainAttributes(ctx, theme, arrayAnimator, R::styleable::Animator);
    const std::string fromString = ta->getString(R::styleable::Animator_valueFrom);
    const std::string toString   = ta->getString(R::styleable::Animator_valueTo);

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

void AnimatorInflater::setupObjectAnimator(Context*ctx, const Resources::Theme* theme, ValueAnimator* anim, const AttributeSet&arrayObjectAnimator,int valueType, float pixelSize){
    auto ta = obtainAttributes(ctx, theme, arrayObjectAnimator, R::styleable::PropertyAnimator);
    ObjectAnimator* oa = (ObjectAnimator*) anim;
    std::string pathData = ta->getString(R::styleable::PropertyAnimator_pathData);
    // Path can be involved in an ObjectAnimator in the following 3 ways:
    // 1) Path morphing: the property to be animated is pathData, and valueFrom and valueTo
    //    are both of pathType. valueType = pathType needs to be explicitly defined.
    // 2) A property in X or Y dimension can be animated along a path: the property needs to be
    //    defined in propertyXName or propertyYName attribute, the path will be defined in the
    //    pathData attribute. valueFrom and valueTo will not be necessary for this animation.
    // 3) PathInterpolator can also define a path (in pathData) for its interpolation curve.
    // Here we are dealing with case 2:
    if (!pathData.empty()) {
        std::string propertyXName = ta->getString(R::styleable::PropertyAnimator_propertyXName);
        std::string propertyYName = ta->getString(R::styleable::PropertyAnimator_propertyYName);

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
        std::string propertyName = ta->getString(R::styleable::PropertyAnimator_propertyName);
        oa->setPropertyName(propertyName);
    }
}

ObjectAnimator* AnimatorInflater::loadObjectAnimator(Context*ctx,const Resources::Theme* theme,const AttributeSet& atts,float){
    ObjectAnimator*anim = new ObjectAnimator();
    loadAnimator(ctx,theme,atts,anim,1.f);
    return anim;
}

ValueAnimator* AnimatorInflater::loadAnimator(Context*context,const Resources::Theme* theme,const AttributeSet& attrs, ValueAnimator* anim, float pathErrorScale){
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

    parseAnimatorFromTypeArray(context,theme,anim,attrs, pathErrorScale);

    auto taInt = obtainAttributes(context, theme, attrs, R::styleable::Animator);
    const int resID = taInt->getResourceId(R::styleable::Animator_interpolator, 0);
    if (resID != 0) {
        Interpolator* interpolator = AnimationUtils::loadInterpolator(context, resID);
        anim->setInterpolator(interpolator);
    }
    return anim;
}

ValueAnimator*  AnimatorInflater::loadValueAnimator(Context*context,const Resources::Theme* theme,const AttributeSet& atts, ValueAnimator*anim,float){
    auto ta = obtainAttributes(context, theme, atts, R::styleable::Animator);
    auto taPvh = obtainAttributes(context, theme, atts, R::styleable::PropertyValuesHolder);
    // AOSP: valueType from the attr; if unspecified, infer from valueFrom/valueTo.
    int valueType = ta->getInt(R::styleable::Animator_valueType, VALUE_TYPE_UNDEFINED);
    if (valueType == VALUE_TYPE_UNDEFINED) {
        valueType = inferValueTypeFromValues(*ta, R::styleable::Animator_valueFrom,
                R::styleable::Animator_valueTo);
    }

    const std::string propertyName = taPvh->getString(R::styleable::PropertyValuesHolder_propertyName);
    const int intpResource = ta->getResourceId(R::styleable::Animator_interpolator, 0);
    Interpolator* interpolator = nullptr;
    if (intpResource != 0) {
        interpolator = AnimationUtils::loadInterpolator(context, intpResource);
    }
    if(anim==nullptr){
        anim = new ValueAnimator();
    }
    if(interpolator){
        anim->setInterpolator(interpolator);
    }
    anim->setDuration(ta->getInt(R::styleable::Animator_duration, 300));
    anim->setStartDelay(ta->getInt(R::styleable::Animator_startOffset, 0));
    anim->setRepeatCount(ta->getInt(R::styleable::Animator_repeatCount, 0));
    anim->setRepeatMode(ta->getInt(R::styleable::Animator_repeatMode, ValueAnimator::RESTART));

    PropertyValuesHolder* pvh = getPVH(context,theme,atts,valueType,propertyName);
    if(pvh)
        anim->setValues({pvh});
    return anim;
}

}
