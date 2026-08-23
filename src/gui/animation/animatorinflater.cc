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
#include <animation/pathkeyframes.h>
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
            PropertyValuesHolder* pvh = loadPvh(ctx, theme, parser, propertyName, valueType);
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

PropertyValuesHolder* AnimatorInflater::loadPvh(Context*ctx,const Resources::Theme* theme,
        XmlPullParser& parser,const std::string& propertyName, int valueType){
    // AOSP AnimatorInflater.loadPvh, ported: nested <keyframe> elements become
    // an ofKeyframes() holder; fraction gaps are filled/distributed per AOSP.
    int type;
    PropertyValuesHolder* value = nullptr;
    std::vector<Keyframe*> keyframes;

    while ((type = parser.next()) != XmlPullParser::END_TAG &&
            type != XmlPullParser::END_DOCUMENT) {
        if (type != XmlPullParser::START_TAG)continue;
        const std::string name = parser.getName();
        if (name.compare("keyframe")==0) {
            if (valueType == VALUE_TYPE_UNDEFINED) {
                valueType = inferValueTypeOfKeyframe(ctx, theme, parser);
            }
            Keyframe* keyframe = loadKeyframe(ctx, theme, parser, valueType);
            if (keyframe != nullptr) {
                keyframes.push_back(keyframe);
            }
            parser.next();
        }
    }

    int count;
    if ((count = (int)keyframes.size()) > 0) {
        // make sure we have keyframes at 0 and 1
        // If we have keyframes with set fractions, add keyframes at start/end
        // appropriately. If start/end have no set fractions:
        // if there's only one keyframe, set its fraction to 1 and add one at 0
        // if >1 keyframe, set the last fraction to 1, the first fraction to 0
        Keyframe* firstKeyframe = keyframes[0];
        Keyframe* lastKeyframe = keyframes[count - 1];
        float endFraction = lastKeyframe->getFraction();
        if (endFraction < 1) {
            if (endFraction < 0) {
                lastKeyframe->setFraction(1);
            } else {
                keyframes.push_back(createNewKeyframe(lastKeyframe, 1));
                ++count;
            }
        }
        float startFraction = firstKeyframe->getFraction();
        if (startFraction != 0) {
            if (startFraction < 0) {
                firstKeyframe->setFraction(0);
            } else {
                keyframes.insert(keyframes.begin(), createNewKeyframe(firstKeyframe, 0));
                ++count;
            }
        }
        for (int i = 0; i < count; ++i) {
            Keyframe* keyframe = keyframes[i];
            if (keyframe->getFraction() < 0) {
                if (i == 0) {
                    keyframe->setFraction(0);
                } else if (i == count - 1) {
                    keyframe->setFraction(1);
                } else {
                    // figure out the start/end parameters of the current gap
                    // in fractions and distribute the gap among those keyframes
                    int startIndex = i;
                    int endIndex = i;
                    for (int j = startIndex + 1; j < count - 1; ++j) {
                        if (keyframes[j]->getFraction() >= 0) {
                            break;
                        }
                        endIndex = j;
                    }
                    float gap = keyframes[endIndex + 1]->getFraction() -
                            keyframes[startIndex - 1]->getFraction();
                    distributeKeyframes(keyframes, gap, startIndex, endIndex);
                }
            }
        }
        value = PropertyValuesHolder::ofKeyframes(propertyName, keyframes);
        if (valueType == VALUE_TYPE_COLOR) {
            value->setEvaluator(PropertyValuesHolder::ArgbEvaluator);
        }
    }
    return value;
}

// AOSP AnimatorInflater.isColorType.
static bool isColorType(int type) {
    return type >= TypedValue::TYPE_FIRST_COLOR_INT && type <= TypedValue::TYPE_LAST_COLOR_INT;
}

int AnimatorInflater::inferValueTypeOfKeyframe(Context*ctx,const Resources::Theme* theme,const AttributeSet& attrs){
    auto a = obtainAttributes(ctx, theme, attrs, R::styleable::Keyframe);
    TypedValue tv;
    const bool hasValue = a->peekValue(R::styleable::Keyframe_value, &tv);
    // When no value type is provided, check whether it's a color type first.
    // If not, fall back to default value type (i.e. float type).
    const int valueType = (hasValue && isColorType(tv.type)) ? VALUE_TYPE_COLOR : VALUE_TYPE_FLOAT;
    return valueType;
}

Keyframe* AnimatorInflater::loadKeyframe(Context*ctx,const Resources::Theme* theme,
        const AttributeSet& attrs,int valueType){
    auto a = obtainAttributes(ctx, theme, attrs, R::styleable::Keyframe);

    Keyframe* keyframe = nullptr;

    float fraction = a->getFloat(R::styleable::Keyframe_fraction, -1.f);

    TypedValue tv;
    const bool hasValue = a->peekValue(R::styleable::Keyframe_value, &tv);
    if (valueType == VALUE_TYPE_UNDEFINED) {
        // When no value type is provided, check whether it's a color type first.
        // If not, fall back to default value type (i.e. float type).
        valueType = (hasValue && isColorType(tv.type)) ? VALUE_TYPE_COLOR : VALUE_TYPE_FLOAT;
    }

    if (hasValue) {
        switch (valueType) {
            case VALUE_TYPE_FLOAT:
                keyframe = Keyframe::ofFloat(fraction, a->getFloat(R::styleable::Keyframe_value, 0.f));
                break;
            case VALUE_TYPE_COLOR:
            case VALUE_TYPE_INT:
                keyframe = Keyframe::ofInt(fraction, a->getInt(R::styleable::Keyframe_value, 0));
                break;
        }
    } else {
        keyframe = (valueType == VALUE_TYPE_FLOAT) ? Keyframe::ofFloat(fraction) :
                Keyframe::ofInt(fraction);
    }

    const int resID = a->getResourceId(R::styleable::Keyframe_interpolator, 0);
    if (resID > 0) {
        Interpolator* interpolator = AnimationUtils::loadInterpolator(ctx, resID);
        keyframe->setInterpolator(interpolator);
    }
    return keyframe;
}

Keyframe* AnimatorInflater::createNewKeyframe(Keyframe* sampleKeyframe, float fraction){
    // AOSP branches on getType() == float.class / int.class; the port matches
    // the concrete keyframe classes (same information, RTTI instead of Class).
    return dynamic_cast<FloatKeyframe*>(sampleKeyframe) ?
                        Keyframe::ofFloat(fraction) :
                        (dynamic_cast<IntKeyframe*>(sampleKeyframe)) ?
                                Keyframe::ofInt(fraction) :
                                Keyframe::ofObject(fraction);
}

void AnimatorInflater::distributeKeyframes(std::vector<Keyframe*>& keyframes, float gap,
        int startIndex, int endIndex){
    // Utility function to set fractions on keyframes to cover a gap in which the
    // fractions are not currently set. Keyframe fractions will be distributed evenly
    // in this gap.
    const int count = endIndex - startIndex + 2;
    const float increment = gap / count;
    for (int i = startIndex; i <= endIndex; ++i) {
        keyframes[i]->setFraction(keyframes[i-1]->getFraction() + increment);
    }
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

    // AOSP gates both on hasValue: an absent repeatCount/repeatMode keeps the
    // animator's current setting instead of resetting it to the default.
    if (ta->hasValue(R::styleable::Animator_repeatCount)) {
        anim->setRepeatCount(ta->getInt(R::styleable::Animator_repeatCount, 0));
    }
    if (ta->hasValue(R::styleable::Animator_repeatMode)) {
        anim->setRepeatMode(ta->getInt(R::styleable::Animator_repeatMode,
                ValueAnimator::RESTART));
    }

    // AOSP: arrayObjectAnimator != null -> setupObjectAnimator(...) — the
    // path (propertyXName/propertyYName) object-animator setup lives there,
    // not in an inline setPropertyName.
    if (dynamic_cast<ObjectAnimator*>(anim)) {
        setupObjectAnimator(ctx, theme, anim, atts, valueType, pixelSize);
    }
}

TypeEvaluator AnimatorInflater::setupAnimatorForPath(Context*ctx, const Resources::Theme* theme, ValueAnimator* anim,const AttributeSet&arrayAnimator){
    TypeEvaluator evaluator = nullptr;
    auto ta = obtainAttributes(ctx, theme, arrayAnimator, R::styleable::Animator);
    const std::string fromString = ta->getString(R::styleable::Animator_valueFrom);
    const std::string toString   = ta->getString(R::styleable::Animator_valueTo);

    // AOSP setObjectValues(...) + new PathDataEvaluator(): CDROID's Object
    // values are AnimateValues, so the values go in through a PathData PHV.
    // (Dead upstream too — the valueFrom/valueTo path-morphing inflow goes
    // through getPVH's VALUE_TYPE_PATH branch; kept implemented for parity.)
    if (!fromString.empty()) {
        PathParser::PathData pathDataFrom(fromString);
        if (!toString.empty()) {
            PathParser::PathData pathDataTo(toString);
            if (!PathParser::canMorph(pathDataFrom, pathDataTo)) {
                throw std::runtime_error(//arrayAnimator.getPositionDescription()
                        " Can't morph from " + fromString + " to " + toString);
            }
            anim->setValues({PropertyValuesHolder::ofObject("", {pathDataFrom, pathDataTo})});
        } else {
            anim->setValues({PropertyValuesHolder::ofObject("", {pathDataFrom})});
        }
        evaluator = PropertyValuesHolder::PathDataEvaluator;
    } else if (!toString.empty()) {
        PathParser::PathData pathDataTo(toString);
        anim->setValues({PropertyValuesHolder::ofObject("", {pathDataTo})});
        evaluator = PropertyValuesHolder::PathDataEvaluator;
    }
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
        if (propertyXName.empty() && propertyYName.empty()) {
            throw std::runtime_error(//arrayObjectAnimator.getPositionDescription()
                    " propertyXName or propertyYName is needed for PathData");
        } else {
            auto path = PathParser::createPathFromPathData(pathData);
            const float error = 0.5f * pixelSize; // max half a pixel error
            // AOSP KeyframeSet.ofPath(path, error): the X/Y holders share the
            // sampled PathKeyframes (shared_ptr keeps the parent alive).
            auto keyframeSet = std::make_shared<PathKeyframes>(path, error);
            Keyframes* xKeyframes = nullptr;
            Keyframes* yKeyframes = nullptr;
            if (valueType == VALUE_TYPE_FLOAT) {
                xKeyframes = keyframeSet->createXFloatKeyframes();
                yKeyframes = keyframeSet->createYFloatKeyframes();
            } else {
                xKeyframes = keyframeSet->createXIntKeyframes();
                yKeyframes = keyframeSet->createYIntKeyframes();
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
