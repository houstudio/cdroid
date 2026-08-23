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
#ifndef __ANIMATOR_INFLATER_H__
#define __ANIMATOR_INFLATER_H__
#include <animation/objectanimator.h>
#include <animation/statelistanimator.h>
#include <core/resources.h>
namespace cdroid{
class TypedArray;
class TypedValue;
class Keyframe;
}
namespace cdroid{
class AnimatorInflater{
private:
    static constexpr int VALUE_TYPE_FLOAT     = 0;
    static constexpr int VALUE_TYPE_INT       = 1;
    static constexpr int VALUE_TYPE_PATH      = 2;
    static constexpr int VALUE_TYPE_COLOR     = 3;
    static constexpr int VALUE_TYPE_UNDEFINED = 4;
    static constexpr int TOGETHER = 0;
    static constexpr int SEQUENTIALLY = 1;
private:
    // AOSP AnimatorInflater private obtainAttributes(res, theme, set, attrs):
    // a null theme falls back to the Context's theme chain, exactly like the
    // themeless Resources.obtainAttributes.
    static std::unique_ptr<TypedArray> obtainAttributes(Context*ctx,const Resources::Theme* theme,
            const AttributeSet& set,const uint32_t* attrs);
    static Animator* createAnimatorFromXml(Context*ctx,const Resources::Theme* theme,XmlPullParser& parser,float pixelSize);
    static Animator* createAnimatorFromXml(Context*ctx,const Resources::Theme* theme,XmlPullParser&parser,const AttributeSet& atts,
                         AnimatorSet*parent,int sequenceOrdering,float pixelSize);
    static StateListAnimator* createStateListAnimatorFromXml(Context*ctx,const Resources::Theme* theme,XmlPullParser&,const AttributeSet&);
    // AOSP AnimatorInflater: infer the value type from valueFrom/valueTo's raw
    // TypedValues (typed/binary face; the old propertyName map was a text-XML shim).
    static int inferValueTypeFromValues(const TypedArray& a, int valueFromId, int valueToId);
    static int inferValueTypeFromType(const TypedValue& tv);
    static PropertyValuesHolder* getPVH(Context*ctx,const Resources::Theme* theme,const AttributeSet&atts, int valueType,const std::string& propertyName);
    static void parseAnimatorFromTypeArray(Context*ctx,const Resources::Theme* theme,ValueAnimator* anim,const AttributeSet&atts, float pixelSize);
    static TypeEvaluator setupAnimatorForPath(Context*ctx,const Resources::Theme* theme,ValueAnimator* anim,const AttributeSet&arrayAnimator);
    static void setupObjectAnimator(Context*ctx,const Resources::Theme* theme,ValueAnimator* anim,const AttributeSet&arrayObjectAnimator,int valueType,float pixelSize);
    static ObjectAnimator* loadObjectAnimator(Context*ctx,const Resources::Theme* theme,const AttributeSet& attrs,float );
    static ValueAnimator* loadValueAnimator(Context*context,const Resources::Theme* theme,const AttributeSet& attrs, ValueAnimator*anim,float);
    static ValueAnimator* loadAnimator(Context*ctx,const Resources::Theme* theme,const AttributeSet& attrs, ValueAnimator* anim, float pathErrorScale);
    static std::vector<PropertyValuesHolder*> loadValues(Context*ctx,const Resources::Theme* theme,XmlPullParser& parser,const  AttributeSet& attrs);
    // AOSP loadPvh(res, theme, parser, propertyName, valueType): parses nested
    // <keyframe> elements into a ofKeyframes() holder (no <keyframe> → null,
    // caller falls back to getPVH's valueFrom/valueTo form).
    static PropertyValuesHolder* loadPvh(Context*ctx,const Resources::Theme* theme,XmlPullParser& parser,const std::string& propertyName, int valueType);
    static int inferValueTypeOfKeyframe(Context*ctx,const Resources::Theme* theme,const AttributeSet& attrs);
    static Keyframe* loadKeyframe(Context*ctx,const Resources::Theme* theme,const AttributeSet& attrs,int valueType);
    static Keyframe* createNewKeyframe(Keyframe* sampleKeyframe, float fraction);
    static void distributeKeyframes(std::vector<Keyframe*>& keyframes, float gap,int startIndex, int endIndex);
public:
    static Animator* loadAnimator(Context* context,const std::string&resid);
    static Animator* loadAnimator(Context* context,const std::string&resid,float pathErrorScale);
    // AOSP AnimatorInflater.loadAnimator(Resources, Theme, @AnimatorRes int[, float])
    // (@hide): the caller's theme — not the Context's current one — drives every
    // attribute read (themed drawable/AVD loads). CDROID opens the XML through
    // the Context (XmlPullParser ctor), so the Resources argument is dropped;
    // resid 0 → null.
    static Animator* loadAnimator(Context* context,const Resources::Theme* theme,const std::string&resid,float pathErrorScale);
    // AOSP AnimatorInflater.loadAnimator(Context, @AnimatorRes int): opened by id
    // (binary AXML via Resources.getXml); resid 0 → null.
    static Animator* loadAnimator(Context* context,int resid);
    static Animator* loadAnimator(Context* context,int resid,float pathErrorScale);
    static Animator* loadAnimator(Context* context,const Resources::Theme* theme,int resid,float pathErrorScale);
    static StateListAnimator* loadStateListAnimator(Context*context,const std::string&resid);
    // AOSP AnimatorInflater.loadStateListAnimator(Context, @AnimatorRes int): the
    // resource is opened directly by id (binary AXML via Resources.getXml); resid 0 → null.
    static StateListAnimator* loadStateListAnimator(Context*context,int resid);
};
}//endof namespace
#endif
