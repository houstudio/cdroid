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
    static Animator* createAnimatorFromXml(Context*ctx,XmlPullParser& parser,float pixelSize);
    static Animator* createAnimatorFromXml(Context*ctx,XmlPullParser&parser,const AttributeSet& atts,
                         AnimatorSet*parent,int sequenceOrdering,float pixelSize);
    static StateListAnimator* createStateListAnimatorFromXml(Context*ctx,XmlPullParser&,const AttributeSet&);
    static int inferValueTypeFromPropertyName(const AttributeSet&atts, const std::string& propertyName);
    static PropertyValuesHolder* getPVH(const AttributeSet&atts, int valueType,const std::string& propertyName);
    static void parseAnimatorFromTypeArray(ValueAnimator* anim,const AttributeSet&atts, float pixelSize);
    static TypeEvaluator setupAnimatorForPath(ValueAnimator* anim,const AttributeSet&arrayAnimator);
    static void setupObjectAnimator(ValueAnimator* anim,const AttributeSet&arrayObjectAnimator,int valueType,float pixelSize);
    static ObjectAnimator* loadObjectAnimator(Context*ctx,const AttributeSet& attrs,float );
    static ValueAnimator* loadValueAnimator(Context*context,const AttributeSet& attrs, ValueAnimator*anim,float);
    static ValueAnimator* loadAnimator(Context*ctx,const AttributeSet& attrs, ValueAnimator* anim, float pathErrorScale);
    static std::vector<PropertyValuesHolder*> loadValues(XmlPullParser& parser,const  AttributeSet& attrs);
    static PropertyValuesHolder* loadPvh(XmlPullParser& parser,const std::string& propertyName, int valueType);
public:
    static Animator* loadAnimator(Context* context,const std::string&resid);
    static Animator* loadAnimator(Context* context,const std::string&resid,float pathErrorScale);
    static StateListAnimator* loadStateListAnimator(Context*context,const std::string&resid);
};
}//endof namespace
#endif
