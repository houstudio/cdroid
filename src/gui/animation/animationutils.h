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
#include <core/xmlpullparser.h>
#include <animation/animationset.h>
#include <animation/layoutanimationcontroller.h>
namespace cdroid{

class AnimationUtils{
private:
    static std::unordered_map<std::string,std::shared_ptr<Interpolator>>mInterpolators;
    static Animation* createAnimationFromXml(Context* c, XmlPullParser& parser,AnimationSet* parent,const AttributeSet& attrs);
    static LayoutAnimationController* createLayoutAnimationFromXml(Context* c,XmlPullParser& parser,const AttributeSet& attrs);
    static Interpolator* createInterpolatorFromXml(Context* context,XmlPullParser&,const std::string&resid);
public:
    static int64_t currentAnimationTimeMillis();
    static Animation* loadAnimation(Context* context,const std::string&id);
    static LayoutAnimationController* loadLayoutAnimation(Context* context,const std::string&id);
    static Animation* makeInAnimation(Context* c, bool fromLeft);
    static Animation* makeOutAnimation(Context* c, bool toRight);
    static Animation* makeInChildBottomAnimation(Context* c);
    static Interpolator* loadInterpolator(Context*,const std::string& id);
};

}//endof namespace

#endif/*__ANIMATION_UTILS_H__*/
