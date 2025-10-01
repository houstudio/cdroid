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
#include <animation/animationutils.h>
#include <animations.h>
#include <core/systemclock.h>
#include <porting/cdlog.h>

namespace cdroid{

std::unordered_map<std::string,std::shared_ptr<Interpolator>>AnimationUtils::mInterpolators;

int64_t AnimationUtils::currentAnimationTimeMillis(){
    return SystemClock::uptimeMillis();
}

Animation* AnimationUtils::loadAnimation(Context* context,const std::string&resid){
    Animation*anim = nullptr;
    int type,depth;
    XmlPullParser parser(context,resid);
    const AttributeSet& attrs = parser;
    return createAnimationFromXml(context,parser,nullptr,attrs);
}

Animation* AnimationUtils::createAnimationFromXml(Context* c, XmlPullParser& parser,AnimationSet* parent,const AttributeSet& attrs){
    // Make sure we are on a start tag.
    int type;
    Animation* anim = nullptr;
    const int depth = parser.getDepth();

    while (((type=parser.next()) != XmlPullParser::END_TAG || parser.getDepth() > depth)
           && (type != XmlPullParser::END_DOCUMENT) && (type!=XmlPullParser::BAD_DOCUMENT) ) {

        if (type != XmlPullParser::START_TAG) {
            continue;
        }
        std::string  name = parser.getName();
        if (name.compare("set")==0) {
            anim = new AnimationSet(c, attrs);
            createAnimationFromXml(c, parser, (AnimationSet*)anim, attrs);
        } else if (name.compare("alpha")==0) {
            anim = new AlphaAnimation(c, attrs);
        } else if (name.compare("scale")==0) {
            anim = new ScaleAnimation(c, attrs);
        }  else if (name.compare("rotate")==0) {
            anim = new RotateAnimation(c, attrs);
        }  else if (name.compare("translate")==0) {
            anim = new TranslateAnimation(c, attrs);
        } else if (name.compare("cliprect")==0) {
            anim = new ClipRectAnimation(c, attrs);
        } else if(name.compare("extend")==0){
            anim = new ExtendAnimation(c,attrs);
        } else {
            LOGW("Unknown animation name:%s",name.c_str());
        }
        if (parent != nullptr) {
            parent->addAnimation(anim);
        }
    }
    return anim;
}

LayoutAnimationController* AnimationUtils::loadLayoutAnimation(Context* context,const std::string&resid){
    int type,depth;
    XmlPullParser parser(context,resid);
    const AttributeSet& attrs = parser;
    return createLayoutAnimationFromXml(context,parser,attrs);
}

LayoutAnimationController* AnimationUtils::createLayoutAnimationFromXml(Context* c,
        XmlPullParser& parser,const AttributeSet& attrs){
    int type;
    const int depth = parser.getDepth();
    LayoutAnimationController* controller = nullptr;
    while (((type = parser.next()) != XmlPullParser::END_TAG || parser.getDepth()>depth)
            && (type != XmlPullParser::END_DOCUMENT) && (type!=XmlPullParser::BAD_DOCUMENT) ) {

        if (type != XmlPullParser::START_TAG) {
            continue;
        }

        const std::string name = parser.getName();
        if (name.compare("layoutAnimation")==0) {
            controller = new LayoutAnimationController(c, attrs);
        } else if (name.compare("gridLayoutAnimation")==0) {
            controller = new GridLayoutAnimationController(c, attrs);
        } else {
            LOGE("Unknown layout animation name:%s ",name.c_str());
        }
    }

    return controller;
}

Animation* AnimationUtils::makeInAnimation(Context* c, bool fromLeft){
    Animation*a = loadAnimation(c,fromLeft?"cdroid:anim/slide_in_left.xml":"cdroid:anim/slide_in_right.xml");
    a->setInterpolator(DecelerateInterpolator::gDecelerateInterpolator.get());
    a->setStartTime(currentAnimationTimeMillis());
    return a;
}

Animation* AnimationUtils::makeOutAnimation(Context* c, bool toRight){
    Animation*a = loadAnimation(c,toRight?"cdroid:anim/slide_out_right.xml":"cdroid:anim/slide_out_left.xml");
    a->setInterpolator(AccelerateInterpolator::gAccelerateInterpolator.get());
    a->setStartTime(currentAnimationTimeMillis());
    return a;
}

Animation* AnimationUtils::makeInChildBottomAnimation(Context* c){
    Animation*a = loadAnimation(c,"cdroid:anim/slide_in_child_bottom.xml");
    a->setInterpolator(AccelerateInterpolator::gAccelerateInterpolator.get());
    a->setStartTime(currentAnimationTimeMillis());
    return a;
}

Interpolator* AnimationUtils::loadInterpolator(Context*context,const std::string& id){
    XmlPullParser parser(context,id);
    return createInterpolatorFromXml(context, parser,id);
}

static std::unordered_map<std::string,std::shared_ptr<Interpolator>>mInterpolators;
Interpolator* AnimationUtils::createInterpolatorFromXml(Context* context,XmlPullParser&parser,const std::string&resid){
    int type;
    const int depth = parser.getDepth();
    std::shared_ptr<BaseInterpolator>interpolator;
    const AttributeSet& attrs = parser;
    auto it = mInterpolators.find(resid);
    if(it!=mInterpolators.end())
        return it->second.get();
    while(((type = parser.next()) != XmlPullParser::END_TAG || parser.getDepth() > depth)
                && type != XmlPullParser::END_DOCUMENT){
        if (type != XmlPullParser::START_TAG) {
            continue;
        }

        const std::string name = parser.getName();
        if (0==name.compare("linearInterpolator")) {
            interpolator = std::make_shared<LinearInterpolator>();
        } else if (0==name.compare("accelerateInterpolator")) {
            interpolator = std::make_shared<AccelerateInterpolator>(context, attrs);
        } else if (0==name.compare("decelerateInterpolator")) {
            interpolator = std::make_shared<DecelerateInterpolator>(context, attrs);
        } else if (0==name.compare("accelerateDecelerateInterpolator")) {
            interpolator = std::make_shared<AccelerateDecelerateInterpolator>();
        } else if (0==name.compare("cycleInterpolator")) {
            interpolator = std::make_shared<CycleInterpolator>(context, attrs);
        } else if (0==name.compare("anticipateInterpolator")) {
            interpolator = std::make_shared<AnticipateInterpolator>(context,attrs);
        } else if (0==name.compare("overshootInterpolator")) {
            interpolator = std::make_shared<OvershootInterpolator>(context, attrs);
        } else if (0==name.compare("anticipateOvershootInterpolator")) {
            interpolator = std::make_shared<AnticipateOvershootInterpolator>(context,attrs);
        } else if (0==name.compare("bounceInterpolator")) {
            interpolator = std::make_shared<BounceInterpolator>();
        } else if (0==name.compare("pathInterpolator")) {
            interpolator = std::make_shared<PathInterpolator>(context,attrs);
        } else {
            LOGE("Unknown interpolator name: %s",name.c_str());
        }
    }
    mInterpolators.insert({resid,interpolator});
    return interpolator.get();
}

}
