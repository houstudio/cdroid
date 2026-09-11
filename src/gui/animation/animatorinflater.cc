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
#include <content/typedvalue.h>
#include <drawable/pathparser.h>
#include <porting/cdlog.h>
#include <core/context.h>
#include <content/typedarray.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
using namespace cdroid::internal;

namespace cdroid{

// AOSP AnimatorInflater.loadAnimator(Context, int) (java:89): pure forward.
Animator* AnimatorInflater::loadAnimator(Context* context,int resid){
    if (context == nullptr) return nullptr;
    Resources::Theme theme = context->getTheme();   // local copy, shared engine
    return loadAnimator(&context->getResources(), &theme, resid);
}

// AOSP @hide loadAnimator(Resources, Theme, int) (java:104).
Animator* AnimatorInflater::loadAnimator(Resources* resources,const Resources::Theme* theme,int resid){
    return loadAnimator(resources, theme, resid, 1.f);
}

// AOSP @hide loadAnimator(Resources, Theme, int, float) (java:110): the
// cache-owning entry.
Animator* AnimatorInflater::loadAnimator(Resources* resources,const Resources::Theme* theme,int resid,float pathErrorScale){
    if (resid == 0) return nullptr;  // AOSP: 0 → null
    // AOSP: the ConfigurationBoundResourceCache on ResourcesImpl serves hits as
    // newInstance() — the cached source animator is never handed out.
    Animator* animator = resources->obtainCachedAnimator(resid,
            theme ? theme->_engineHandle() : nullptr);
    if (animator != nullptr) return animator;
    auto parser = resources->getXml(resid);
    animator = createAnimatorFromXml(resources, theme, *parser, pathErrorScale);
    if (animator != nullptr) {
        // AOSP appends getChangingConfigs(resources, id) so entries self-invalidate
        // via needNewResources; CDROID clears the whole cache on configuration
        // change (ResourcesImpl::updateConfiguration), making per-entry configs
        // unnecessary. createConstantState() transfers ownership of the parsed
        // animator to the constant state (AOSP relies on GC).
        const auto constantState = animator->createConstantState();
        if (constantState != nullptr) {
            resources->cacheAnimator(resid, theme ? theme->_engineHandle() : nullptr, constantState);
            // create a new animator so that cached version is never used by the user
            animator = constantState->newInstance();
        }
    }
    return animator;
}


StateListAnimator* AnimatorInflater::loadStateListAnimator(Context* context,int resid){
    if (resid == 0) return nullptr;  // AOSP: 0 → null
    // AOSP loadStateListAnimator(Context, id): ConfigurationBoundResourceCache
    // on ResourcesImpl; hits come back as newInstance() (a clone).
    Resources& res = context->getResources();
    Resources::Theme theme = context->getTheme();
    StateListAnimator* animator = res.obtainCachedStateListAnimator(resid, theme._engineHandle());
    if (animator != nullptr) return animator;
    auto parser = context->getResources().getXml(resid);
    const AttributeSet& attrs = *parser;
    animator = createStateListAnimatorFromXml(context, &theme, *parser, attrs);
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
Animator* AnimatorInflater::createAnimatorFromXml(Resources*res,const Resources::Theme* theme,XmlPullParser& parser,float pixelSize){
    const AttributeSet& attrs = parser;
    return createAnimatorFromXml(res,theme,parser, attrs, nullptr, 0,pixelSize);
}

Animator* AnimatorInflater::createAnimatorFromXml(Resources*res,const Resources::Theme* theme,XmlPullParser&parser,const AttributeSet& attrs,
        AnimatorSet*parent,int sequenceOrdering,float pixelSize){
     Animator* anim = nullptr;
     std::vector<Animator*> childAnims;

    // Make sure we are on a start tag.
    int type = 0;
    const int innerDepth = parser.getDepth()+1;
    while ((((type = parser.next()) != XmlPullParser::END_TAG) || (parser.getDepth() >= innerDepth))
            && (type != XmlPullParser::END_DOCUMENT) && (type != XmlPullParser::BAD_DOCUMENT) ) {

        if (type != XmlPullParser::START_TAG) {
            continue;
        }

        std::string name = parser.getName();
        bool gotValues = false;
        if (name.compare("objectAnimator")==0) {
            anim = loadObjectAnimator(res,theme,attrs, pixelSize);
        } else if (name.compare("animator")==0) {
            anim = loadAnimator(res, theme, attrs, nullptr, pixelSize);
        } else if (name.compare("set")==0) {
            anim = new AnimatorSet();
            // AOSP java:679: res.obtainAttributes (themeless) for the set's ordering.
            auto ta = res->obtainStyledAttributes(&attrs, R::styleable::AnimatorSet);
            const int ordering = ta->getInt(R::styleable::AnimatorSet_ordering, TOGETHER);
            createAnimatorFromXml(res, theme, parser, attrs, (AnimatorSet*) anim, ordering,pixelSize);
        } else if (name.compare("propertyValuesHolder")==0) {
            std::vector<PropertyValuesHolder*>values = loadValues(res,theme,parser,attrs);
            if (values.size() && (dynamic_cast<ValueAnimator*>(anim))) {
                ((ValueAnimator*) anim)->setValues(values);
            } else {
                // Unconsumed holders (no ValueAnimator parsed yet): AOSP's
                // list just goes out of scope for GC; free them here.
                for (auto v : values) delete v;
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
                // AOSP createStateListAnimatorFromXml: one pass over the item's
                // attrs — android:animation loads the animator through the
                // cached public loadAnimator (hit returns a clone), every other
                // attr is a state (+id when true, -id when false).
                std::vector<int>states;
                Animator* animator = nullptr;
                const int attributeCount = (int)attrs.getAttributeCount();
                for (int i = 0; i < attributeCount; i++) {
                    const int attrName = attrs.getAttributeNameResource(i);
                    if (attrName == R::attr::animation) {
                        animator = loadAnimator(context, attrs.getAttributeResourceValue(i, 0));
                    } else {
                        states.push_back(attrs.getAttributeBooleanValue(i, false) ? attrName : -attrName);
                    }
                }
                if (animator == nullptr) {
                    animator = createAnimatorFromXml(&context->getResources(),theme,parser,attrs, nullptr, 0,1.f);
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
 
std::vector<PropertyValuesHolder*> AnimatorInflater::loadValues(Resources*res,const Resources::Theme* theme,XmlPullParser& parser,const AttributeSet& attrs){
    std::vector<PropertyValuesHolder*> values;
    int type = XmlPullParser::START_TAG;
    while ((type != XmlPullParser::END_TAG) && (type != XmlPullParser::END_DOCUMENT)) {
        if (type != XmlPullParser::START_TAG) {
            type = parser.next();
            continue;
        }
        std::string name = parser.getName();
        if (name.compare("propertyValuesHolder")==0) {
            // AOSP java:739: res.obtainAttributes (themeless).
            auto ta = res->obtainStyledAttributes(&attrs, R::styleable::PropertyValuesHolder);
            const std::string propertyName = ta->getString(R::styleable::PropertyValuesHolder_propertyName);
            const int valueType = ta->getInt(R::styleable::PropertyValuesHolder_valueType, VALUE_TYPE_UNDEFINED);
            LOGD("propertyValuesHolder.%s type=%d",propertyName.c_str(),valueType);
            PropertyValuesHolder* pvh = loadPvh(res, theme, parser, propertyName, valueType);
            if (pvh == nullptr) {
                pvh = getPVH(*ta, valueType, R::styleable::PropertyValuesHolder_valueFrom,
                        R::styleable::PropertyValuesHolder_valueTo, propertyName);
            }
            if (pvh != nullptr) {
                values.push_back(pvh);
            }
        }
        type = parser.next();

    }
    return values;
}

PropertyValuesHolder* AnimatorInflater::loadPvh(Resources*res,const Resources::Theme* theme,
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
                valueType = inferValueTypeOfKeyframe(res, theme, parser);
            }
            Keyframe* keyframe = loadKeyframe(res, theme, parser, valueType);
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

int AnimatorInflater::inferValueTypeOfKeyframe(Resources*res,const Resources::Theme* theme,const AttributeSet& attrs){
    // AOSP java:783: res.obtainAttributes (themeless).
    auto a = res->obtainStyledAttributes(&attrs, R::styleable::Keyframe);
    TypedValue tv;
    const bool hasValue = a->peekValue(R::styleable::Keyframe_value, &tv);
    // When no value type is provided, check whether it's a color type first.
    // If not, fall back to default value type (i.e. float type).
    const int valueType = (hasValue && isColorType(tv.type)) ? VALUE_TYPE_COLOR : VALUE_TYPE_FLOAT;
    return valueType;
}

Keyframe* AnimatorInflater::loadKeyframe(Resources*res,const Resources::Theme* theme,
        const AttributeSet& attrs,int valueType){
    // AOSP: res.obtainAttributes (themeless).
    auto a = res->obtainStyledAttributes(&attrs, R::styleable::Keyframe);

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
        Interpolator* interpolator = AnimationUtils::loadInterpolator(res, theme, resID);
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

int AnimatorInflater::inferValueTypeFromValues(const TypedArray& a, int valueFromId, int valueToId) {
    // AOSP AnimatorInflater.inferValueTypeFromValues (android-36), verbatim:
    // a color-typed endpoint forces COLOR, everything else falls back to FLOAT.
    TypedValue tvFrom;
    const bool hasFrom = a.peekValue(valueFromId, &tvFrom);
    const int fromType = hasFrom ? tvFrom.type : 0;
    TypedValue tvTo;
    const bool hasTo = a.peekValue(valueToId, &tvTo);
    const int toType = hasTo ? tvTo.type : 0;

    if ((hasFrom && isColorType(fromType)) || (hasTo && isColorType(toType))) {
        return VALUE_TYPE_COLOR;
    }
    return VALUE_TYPE_FLOAT;
}

PropertyValuesHolder*AnimatorInflater::getPVH(const TypedArray& styledAttributes, int valueType,
        int valueFromId,int valueToId, const std::string& propertyName){
    // AOSP AnimatorInflater.getPVH (android-36), line-by-line: peek the raw
    // TypedValues for the type test, infer COLOR/FLOAT when unspecified (never
    // bail), read DIMENSION endpoints through getDimension, color endpoints
    // through getColor, and always build the degenerate single-value holder
    // when only valueTo (or neither) is set.
    const TypedArray& ta = styledAttributes;
    TypedValue tvFrom;
    const bool hasFrom = ta.peekValue(valueFromId, &tvFrom);
    const int fromType = hasFrom ? tvFrom.type : 0;
    TypedValue tvTo;
    const bool hasTo = ta.peekValue(valueToId, &tvTo);
    const int toType = hasTo ? tvTo.type : 0;

    if (valueType == VALUE_TYPE_UNDEFINED) {
        // Check whether it's color type. If not, fall back to default type (i.e. float type)
        if ((hasFrom && isColorType(fromType)) || (hasTo && isColorType(toType))) {
            valueType = VALUE_TYPE_COLOR;
        } else {
            valueType = VALUE_TYPE_FLOAT;
        }
    }

    const bool getFloats = (valueType == VALUE_TYPE_FLOAT);

    PropertyValuesHolder* returnValue = nullptr;

    if (valueType == VALUE_TYPE_PATH) {
        const std::string fromString = ta.getString(valueFromId);
        const std::string toString = ta.getString(valueToId);
        PathParser::PathData nodesFrom = fromString.empty() ? PathParser::PathData() : PathParser::PathData(fromString);
        PathParser::PathData nodesTo = toString.empty()  ? PathParser::PathData() : PathParser::PathData(toString);

        if (fromString.size() || toString.size()) {
            if (fromString.size()) {
                if (toString.size()) {
                    if (!PathParser::canMorph(nodesFrom, nodesTo)) {
                        throw std::runtime_error(std::string(" Can't morph from") + fromString + " to " + toString);
                    }
                    returnValue = PropertyValuesHolder::ofObject(propertyName, {nodesFrom, nodesTo});
                } else {
                    returnValue = PropertyValuesHolder::ofObject(propertyName, {nodesFrom});
                }
            } else if (toString.size()) {
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
                valueFrom = (fromType == TypedValue::TYPE_DIMENSION)
                        ? ta.getDimension(valueFromId, 0.f)
                        : ta.getFloat(valueFromId, 0.f);
                if (hasTo) {
                    valueTo = (toType == TypedValue::TYPE_DIMENSION)
                            ? ta.getDimension(valueToId, 0.f)
                            : ta.getFloat(valueToId, 0.f);
                    returnValue = PropertyValuesHolder::ofFloat(propertyName, {valueFrom, valueTo});
                } else {
                    returnValue = PropertyValuesHolder::ofFloat(propertyName, {valueFrom});
                }
            } else {
                valueTo = (toType == TypedValue::TYPE_DIMENSION)
                        ? ta.getDimension(valueToId, 0.f)
                        : ta.getFloat(valueToId, 0.f);
                returnValue = PropertyValuesHolder::ofFloat(propertyName, {valueTo});
            }
        } else {
            int valueFrom = 0, valueTo = 0;
            if (hasFrom) {
                if (fromType == TypedValue::TYPE_DIMENSION) {
                    valueFrom = (int) ta.getDimension(valueFromId, 0.f);
                } else if (isColorType(fromType)) {
                    valueFrom = (int) ta.getColor(valueFromId, 0);
                } else {
                    valueFrom = ta.getInt(valueFromId, 0);
                }
                if (hasTo) {
                    if (toType == TypedValue::TYPE_DIMENSION) {
                        valueTo = (int) ta.getDimension(valueToId, 0.f);
                    } else if (isColorType(toType)) {
                        valueTo = (int) ta.getColor(valueToId, 0);
                    } else {
                        valueTo = ta.getInt(valueToId, 0);
                    }
                    returnValue = PropertyValuesHolder::ofInt(propertyName, {valueFrom, valueTo});
                } else {
                    returnValue = PropertyValuesHolder::ofInt(propertyName, {valueFrom});
                }
            } else {
                if (hasTo) {
                    if (toType == TypedValue::TYPE_DIMENSION) {
                        valueTo = (int) ta.getDimension(valueToId, 0.f);
                    } else if (isColorType(toType)) {
                        valueTo = (int) ta.getColor(valueToId, 0);
                    } else {
                        valueTo = ta.getInt(valueToId, 0);
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

void AnimatorInflater::parseAnimatorFromTypeArray(ValueAnimator* anim, const TypedArray& arrayAnimator,
        const TypedArray* arrayObjectAnimator, float pixelSize) {
    // AOSP java:413-452: consumes the arrays styled once by loadAnimator.
    const long duration = arrayAnimator.getInt(R::styleable::Animator_duration, 300);
    const long startDelay = arrayAnimator.getInt(R::styleable::Animator_startOffset, 0);

    // AOSP: valueType from the attr; if unspecified, infer from valueFrom/valueTo.
    int valueType = arrayAnimator.getInt(R::styleable::Animator_valueType, VALUE_TYPE_UNDEFINED);
    if (valueType == VALUE_TYPE_UNDEFINED) {
        valueType = inferValueTypeFromValues(arrayAnimator, R::styleable::Animator_valueFrom,
                R::styleable::Animator_valueTo);
    }

    PropertyValuesHolder* pvh = getPVH(arrayAnimator, valueType,
            R::styleable::Animator_valueFrom, R::styleable::Animator_valueTo, std::string());
    if (pvh != nullptr) {
        anim->setValues({pvh});
    }

    anim->setDuration(duration);
    anim->setStartDelay(startDelay);

    // AOSP gates both on hasValue: an absent repeatCount/repeatMode keeps the
    // animator's current setting instead of resetting it to the default.
    if (arrayAnimator.hasValue(R::styleable::Animator_repeatCount)) {
        anim->setRepeatCount(arrayAnimator.getInt(R::styleable::Animator_repeatCount, 0));
    }
    if (arrayAnimator.hasValue(R::styleable::Animator_repeatMode)) {
        anim->setRepeatMode(arrayAnimator.getInt(R::styleable::Animator_repeatMode,
                ValueAnimator::RESTART));
    }

    // AOSP: arrayObjectAnimator != null -> setupObjectAnimator(...) — the
    // path (propertyXName/propertyYName) object-animator setup lives there,
    // not in an inline setPropertyName.
    if (arrayObjectAnimator != nullptr) {
        setupObjectAnimator(anim, *arrayObjectAnimator, valueType, pixelSize);
    }
}

TypeEvaluator AnimatorInflater::setupAnimatorForPath(ValueAnimator* anim, const TypedArray& arrayAnimator){
    TypeEvaluator evaluator = nullptr;
    const std::string fromString = arrayAnimator.getString(R::styleable::Animator_valueFrom);
    const std::string toString   = arrayAnimator.getString(R::styleable::Animator_valueTo);

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

void AnimatorInflater::setupObjectAnimator(ValueAnimator* anim, const TypedArray& arrayObjectAnimator, int valueType, float pixelSize){
    const TypedArray& ta = arrayObjectAnimator;
    ObjectAnimator* oa = (ObjectAnimator*) anim;
    std::string pathData = ta.getString(R::styleable::PropertyAnimator_pathData);
    // Path can be involved in an ObjectAnimator in the following 3 ways:
    // 1) Path morphing: the property to be animated is pathData, and valueFrom and valueTo
    //    are both of pathType. valueType = pathType needs to be explicitly defined.
    // 2) A property in X or Y dimension can be animated along a path: the property needs to be
    //    defined in propertyXName or propertyYName attribute, the path will be defined in the
    //    pathData attribute. valueFrom and valueTo will not be necessary for this animation.
    // 3) PathInterpolator can also define a path (in pathData) for its interpolation curve.
    // Here we are dealing with case 2:
    if (!pathData.empty()) {
        std::string propertyXName = ta.getString(R::styleable::PropertyAnimator_propertyXName);
        std::string propertyYName = ta.getString(R::styleable::PropertyAnimator_propertyYName);

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
            // Only the named axis adopts its keyframes; the other one was
            // still created above and must be freed (AOSP leans on GC here —
            // valgrind showed the un-adopted X/Y FloatKeyframes lost).
            if (!propertyXName.empty()) {
                x = PropertyValuesHolder::ofKeyframes(propertyXName, xKeyframes);
            } else {
                delete xKeyframes;
            }
            if (!propertyYName.empty()) {
                y = PropertyValuesHolder::ofKeyframes(propertyYName, yKeyframes);
            } else {
                delete yKeyframes;
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
        std::string propertyName = ta.getString(R::styleable::PropertyAnimator_propertyName);
        oa->setPropertyName(propertyName);
    }
}

// AOSP AnimatorInflater.setupValues: pushes the endpoint values straight onto
// the animator via setFloatValues/setIntValues. (Dead upstream too — no caller
// in android-36, the value inflow goes through getPVH; kept implemented for
// parity, like setupAnimatorForPath above.)
void AnimatorInflater::setupValues(ValueAnimator* anim, const TypedArray& arrayAnimator,
        bool getFloats, bool hasFrom, int fromType, bool hasTo, int toType) {
    const int valueFromIndex = R::styleable::Animator_valueFrom;
    const int valueToIndex = R::styleable::Animator_valueTo;
    if (getFloats) {
        float valueFrom;
        float valueTo;
        if (hasFrom) {
            if (fromType == TypedValue::TYPE_DIMENSION) {
                valueFrom = arrayAnimator.getDimension(valueFromIndex, 0.f);
            } else {
                valueFrom = arrayAnimator.getFloat(valueFromIndex, 0.f);
            }
            if (hasTo) {
                if (toType == TypedValue::TYPE_DIMENSION) {
                    valueTo = arrayAnimator.getDimension(valueToIndex, 0.f);
                } else {
                    valueTo = arrayAnimator.getFloat(valueToIndex, 0.f);
                }
                anim->setFloatValues({valueFrom, valueTo});
            } else {
                anim->setFloatValues({valueFrom});
            }
        } else {
            if (toType == TypedValue::TYPE_DIMENSION) {
                valueTo = arrayAnimator.getDimension(valueToIndex, 0.f);
            } else {
                valueTo = arrayAnimator.getFloat(valueToIndex, 0.f);
            }
            anim->setFloatValues({valueTo});
        }
    } else {
        int valueFrom;
        int valueTo;
        if (hasFrom) {
            if (fromType == TypedValue::TYPE_DIMENSION) {
                valueFrom = (int) arrayAnimator.getDimension(valueFromIndex, 0.f);
            } else if (isColorType(fromType)) {
                valueFrom = (int) arrayAnimator.getColor(valueFromIndex, 0);
            } else {
                valueFrom = arrayAnimator.getInt(valueFromIndex, 0);
            }
            if (hasTo) {
                if (toType == TypedValue::TYPE_DIMENSION) {
                    valueTo = (int) arrayAnimator.getDimension(valueToIndex, 0.f);
                } else if (isColorType(toType)) {
                    valueTo = (int) arrayAnimator.getColor(valueToIndex, 0);
                } else {
                    valueTo = arrayAnimator.getInt(valueToIndex, 0);
                }
                anim->setIntValues({valueFrom, valueTo});
            } else {
                anim->setIntValues({valueFrom});
            }
        } else {
            if (hasTo) {
                if (toType == TypedValue::TYPE_DIMENSION) {
                    valueTo = (int) arrayAnimator.getDimension(valueToIndex, 0.f);
                } else if (isColorType(toType)) {
                    valueTo = (int) arrayAnimator.getColor(valueToIndex, 0);
                } else {
                    valueTo = arrayAnimator.getInt(valueToIndex, 0);
                }
                anim->setIntValues({valueTo});
            }
        }
    }
}

ObjectAnimator* AnimatorInflater::loadObjectAnimator(Resources* res,const Resources::Theme* theme,const AttributeSet& atts,float pathErrorScale){
    ObjectAnimator*anim = new ObjectAnimator();
    loadAnimator(res,theme,atts,anim,pathErrorScale);
    return anim;
}

ValueAnimator* AnimatorInflater::loadAnimator(Resources* resources,const Resources::Theme* theme,const AttributeSet& attrs, ValueAnimator* anim, float pathErrorScale){
    // AOSP java:1028-1067: style both arrays once (theme-driven when a theme
    // is in play, themeless through res otherwise), parse from them, and load
    // the interpolator through (res, theme).
    std::unique_ptr<TypedArray> arrayAnimator;
    std::unique_ptr<TypedArray> arrayObjectAnimator;

    if (theme != nullptr) {
        arrayAnimator = theme->obtainStyledAttributes(&attrs, R::styleable::Animator);
    } else {
        arrayAnimator = resources->obtainStyledAttributes(&attrs, R::styleable::Animator);
    }

    // If anim is not null, then it is an object animator.
    if (anim != nullptr) {
        if (theme != nullptr) {
            arrayObjectAnimator = theme->obtainStyledAttributes(&attrs, R::styleable::PropertyAnimator);
        } else {
            arrayObjectAnimator = resources->obtainStyledAttributes(&attrs, R::styleable::PropertyAnimator);
        }
    }

    if (anim == nullptr) {
        anim = new ValueAnimator();
    }

    parseAnimatorFromTypeArray(anim, *arrayAnimator,
            arrayObjectAnimator ? arrayObjectAnimator.get() : nullptr, pathErrorScale);

    const int resID = arrayAnimator->getResourceId(R::styleable::Animator_interpolator, 0);
    if (resID > 0) {
        Interpolator* interpolator = AnimationUtils::loadInterpolator(resources, theme, resID);
        anim->setInterpolator(interpolator);
    }
    return anim;
}

}

