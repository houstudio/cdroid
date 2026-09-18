/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either
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
#include <content/resources.h>
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
    // AOSP AnimatorInflater.createAnimatorFromXml(Resources, Theme, parser[, ...]).
    static Animator* createAnimatorFromXml(Resources* res,const Resources::Theme* theme,XmlPullParser& parser,float pixelSize);
    static Animator* createAnimatorFromXml(Resources* res,const Resources::Theme* theme,XmlPullParser&parser,const AttributeSet& atts,
                         AnimatorSet*parent,int sequenceOrdering,float pixelSize);
    // AOSP AnimatorInflater.createStateListAnimatorFromXml(Context, Theme, ...).
    static StateListAnimator* createStateListAnimatorFromXml(Context*ctx,const Resources::Theme* theme,XmlPullParser&,const AttributeSet&);
    // AOSP AnimatorInflater: infer the value type from valueFrom/valueTo's raw
    // TypedValues (typed/binary face; the old propertyName map was a text-XML shim).
    static int inferValueTypeFromValues(const TypedArray& a, int valueFromId, int valueToId);
    // AOSP getPVH(TypedArray styledAttributes, int valueType, int valueFromId,
    // int valueToId, String propertyName): consumes the caller-styled array.
    static PropertyValuesHolder* getPVH(const TypedArray& styledAttributes,int valueType,
            int valueFromId,int valueToId,const std::string& propertyName);
    // AOSP parseAnimatorFromTypeArray(ValueAnimator, TypedArray arrayAnimator,
    // TypedArray arrayObjectAnimator, float pixelSize): both arrays styled once
    // by the caller (loadAnimator).
    static void parseAnimatorFromTypeArray(ValueAnimator* anim,const TypedArray& arrayAnimator,
            const TypedArray* arrayObjectAnimator,float pixelSize);
    // AOSP setupAnimatorForPath(ValueAnimator, TypedArray). Dead upstream (no
    // caller in android-36); kept for parity.
    static TypeEvaluator setupAnimatorForPath(ValueAnimator* anim,const TypedArray& arrayAnimator);
    // AOSP setupObjectAnimator(ValueAnimator, TypedArray arrayObjectAnimator,
    // int valueType, float pixelSize).
    static void setupObjectAnimator(ValueAnimator* anim,const TypedArray& arrayObjectAnimator,int valueType,float pixelSize);
    // AOSP setupValues: dead upstream (no caller in android-36); kept for parity.
    static void setupValues(ValueAnimator* anim, const TypedArray& arrayAnimator,
            bool getFloats, bool hasFrom, int fromType, bool hasTo, int toType);
    static ObjectAnimator* loadObjectAnimator(Resources* res,const Resources::Theme* theme,const AttributeSet& attrs,float pathErrorScale);
    static ValueAnimator* loadAnimator(Resources* res,const Resources::Theme* theme,const AttributeSet& attrs, ValueAnimator* anim, float pathErrorScale);
    static std::vector<PropertyValuesHolder*> loadValues(Resources* res,const Resources::Theme* theme,XmlPullParser& parser,const  AttributeSet& attrs);
    // AOSP loadPvh(res, theme, parser, propertyName, valueType): parses nested
    // <keyframe> elements into a ofKeyframes() holder (no <keyframe> → null,
    // caller falls back to getPVH's valueFrom/valueTo form).
    static PropertyValuesHolder* loadPvh(Resources* res,const Resources::Theme* theme,XmlPullParser& parser,const std::string& propertyName, int valueType);
    static int inferValueTypeOfKeyframe(Resources* res,const Resources::Theme* theme,const AttributeSet& attrs);
    static Keyframe* loadKeyframe(Resources* res,const Resources::Theme* theme,const AttributeSet& attrs,int valueType);
    static Keyframe* createNewKeyframe(Keyframe* sampleKeyframe, float fraction);
    static void distributeKeyframes(std::vector<Keyframe*>& keyframes, float gap,int startIndex, int endIndex);
public:
    // AOSP AnimatorInflater.loadAnimator(Context, @AnimatorRes int): forwards
    // with the context's resources and theme (java:89-91).
    static Animator* loadAnimator(Context* context,int resid);
    // AOSP @hide loadAnimator(Resources, Theme, int) (java:104): forwards with
    // pathErrorScale 1.
    static Animator* loadAnimator(Resources* resources,const Resources::Theme* theme,int resid);
    // AOSP @hide loadAnimator(Resources, Theme, int, float) (java:110): the
    // cache-owning entry — hits come back as newInstance() clones (the cached
    // source animator is never handed out); resid 0 → null.
    static Animator* loadAnimator(Resources* resources,const Resources::Theme* theme,int resid,float pathErrorScale);
    // AOSP AnimatorInflater.loadStateListAnimator(Context, @AnimatorRes int): the
    // resource is opened directly by id (binary AXML via Resources.getXml); resid 0 → null.
    static StateListAnimator* loadStateListAnimator(Context*context,int resid);
};
}//endof namespace
#endif
