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
#ifndef __ANIMATION_UTILS_H__
#define __ANIMATION_UTILS_H__
#include <memory>
#include <map>
#include <utility>
#include <core/xmlpullparser.h>
#include <animation/animationset.h>
#include <animation/layoutanimationcontroller.h>
#include <content/resources.h>   // Resources::Theme (the @hide loadInterpolator face)
namespace cdroid{

class AnimationUtils{
private:
    // Interpolator cache, keyed by resource id (AOSP AnimationUtils caches by
    // @InterpolatorRes int). The legacy string overload resolves its name to an
    // id and shares this cache. Keyed (id, theme engine) — same theme under a
    // different engine resolves independently, matching the themed animator
    // cache on Resources (AOSP has no interpolator cache; every load re-parses).
    static std::map<std::pair<int,void*>,std::shared_ptr<Interpolator>>mInterpolators;
    static Animation* createAnimationFromXml(Context* c, XmlPullParser& parser,AnimationSet* parent,const AttributeSet& attrs);
    static LayoutAnimationController* createLayoutAnimationFromXml(Context* c,XmlPullParser& parser,const AttributeSet& attrs);
    // AOSP createInterpolatorFromXml(Resources, Theme, parser) (java:452): the
    // interpolators' attribute reads are theme-driven.
    static std::shared_ptr<Interpolator> createInterpolatorFromXml(Resources* res,const Resources::Theme* theme,XmlPullParser& parser);
public:
    static int64_t currentAnimationTimeMillis();
    static Animation* loadAnimation(Context* context,int id);
    static LayoutAnimationController* loadLayoutAnimation(Context* context,int id);
    static Animation* makeInAnimation(Context* c, bool fromLeft);
    static Animation* makeOutAnimation(Context* c, bool toRight);
    static Animation* makeInChildBottomAnimation(Context* c);
    // AOSP AnimationUtils.loadInterpolator(Context, @InterpolatorRes int): the
    // resource is opened by id (binary AXML via Resources.getXml); id 0 → null.
    // OWNERSHIP: like the XxxInterpolator::Instance constants, cached loads are
    // process-resident and must NOT be deleted by callers — animators store the
    // pointer as a borrow (ValueAnimator::setInterpolator does not own it).
    // Direct construction (new XxxInterpolator(...)) stays caller-owned.
    static Interpolator* loadInterpolator(Context*,int id);
    // AOSP @hide AnimationUtils.loadInterpolator(Resources, Theme, int): the
    // face AnimatorInflater's private chain calls. CDROID's interpolator loads
    // open the XML through the Context (reached from res via getContext()) —
    // the theme is not threaded into interpolator styling yet.
    static Interpolator* loadInterpolator(Resources* res,const Resources::Theme* theme,int id);

    static float lerp(float startValue, float endValue, float fraction) {
        return startValue + fraction * (endValue - startValue);
    }

    static int lerp(int startValue, int endValue, float fraction) {
        return startValue + std::round(fraction * (float)(endValue - startValue));
    }

    static float lerp(float outputMin, float outputMax, float inputMin, float inputMax, float value) {
        if (value <= inputMin) {
            return outputMin;
        } else {
            return value >= inputMax ? outputMax : lerp(outputMin, outputMax, (value - inputMin) / (inputMax - inputMin));
        }
    }
};

}//endof namespace

#endif/*__ANIMATION_UTILS_H__*/
