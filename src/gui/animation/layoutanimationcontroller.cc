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
#include <stdlib.h>
#include <view/viewgroup.h>
#include <animation/animationutils.h>
#include <animation/layoutanimationcontroller.h>
#include <core/typedarray.h>
#include <widget/framework_styleable.h>
#include <random>

namespace cdroid{
using namespace cdroid::internal;

LayoutAnimationController::LayoutAnimationController(Context* context, const AttributeSet& attrs){
    // AOSP LayoutAnimationController: LayoutAnimation styleable, in order
    // animation → delay → animationOrder → interpolator.
    auto a = context->obtainStyledAttributes(attrs, R::styleable::LayoutAnimation);

    mAnimation    = nullptr;
    mInterpolator = nullptr;
    mMaxDelay  = LONG_MIN;

    int resource = a->getResourceId(R::styleable::LayoutAnimation_animation, 0);
    if(resource) setAnimation(context,resource);

    mDelay = a->getFloat(R::styleable::LayoutAnimation_delay, 0.5f/*DEFAULT_DELAY*/);
    // aapt2 pre-resolves the animationOrder enum names (normal/reverse/random).
    mOrder = a->getInt(R::styleable::LayoutAnimation_animationOrder, ORDER_NORMAL);

    resource = a->getResourceId(R::styleable::LayoutAnimation_interpolator, 0);
    if(resource) setInterpolator(context,resource);
}

LayoutAnimationController::LayoutAnimationController(Animation* animation,float delay){
    mDelay = delay;
    mOrder =0;
    mInterpolator= nullptr;
    mAnimation   = nullptr;
    mMaxDelay    = LONG_MIN;
    setAnimation(animation);
}

LayoutAnimationController::LayoutAnimationController(Animation* animation)
 :LayoutAnimationController(animation,.5f){
}

LayoutAnimationController::~LayoutAnimationController(){
    delete mAnimation;
    delete mInterpolator;
}

int LayoutAnimationController::getOrder()const{
    return mOrder;
}

void LayoutAnimationController::setOrder(int order){
    mOrder = order;
}

void LayoutAnimationController::setAnimation(Context* context,int resourceID){
    setAnimation(AnimationUtils::loadAnimation(context,resourceID));
}

void LayoutAnimationController::setAnimation(Animation* animation){
    if(mAnimation)delete mAnimation;
    mAnimation = animation;
    mAnimation->setFillBefore(true);
}

Animation* LayoutAnimationController::getAnimation(){
    return mAnimation;
}

void LayoutAnimationController::setInterpolator(Context* context,int resourceID){
    setInterpolator(AnimationUtils::loadInterpolator(context,resourceID));
}

void LayoutAnimationController::setInterpolator(const Interpolator* interpolator){
    mInterpolator = interpolator;
}

const Interpolator* LayoutAnimationController::getInterpolator()const{
    return mInterpolator;
}

float LayoutAnimationController::getDelay()const{
    return mDelay;
}

void LayoutAnimationController::setDelay(float delay){
    mDelay= delay;
}

bool LayoutAnimationController::willOverlap(){
    return mDelay < 1.0f;
}

void LayoutAnimationController::start(){
    mDuration = mAnimation->getDuration();
    mMaxDelay = LONG_MIN;//Long.MIN_VALUE;
    mAnimation->setStartTime(-1);
}

Animation* LayoutAnimationController::getAnimationForView(View* view){
    const int64_t delay = getDelayForView(view) + mAnimation->getStartOffset();
    mMaxDelay = std::max(mMaxDelay, delay);

    Animation* animation = mAnimation->clone();
    animation->setStartOffset(delay);
    return animation;
}

bool LayoutAnimationController::isDone()const{
    return AnimationUtils::currentAnimationTimeMillis() >
                mAnimation->getStartTime() + mMaxDelay + mDuration;
}

int64_t LayoutAnimationController::getDelayForView(View* view){
    ViewGroup::LayoutParams* lp = view->getLayoutParams();
    AnimationParameters* params = lp->layoutAnimationParameters;

    if (params == nullptr) return 0;

    const float delay = mDelay * (float)mAnimation->getDuration();
    const int64_t viewDelay= getTransformedIndex(params) * delay;
    const float totalDelay = delay * params->count;

    if (mInterpolator == nullptr) {
        mInterpolator = new LinearInterpolator();
    }

    float normalizedDelay = viewDelay / totalDelay;
    normalizedDelay = mInterpolator->getInterpolation(normalizedDelay);
    LOGV("%p:%d totalDelay=%.2f %d/%d mDelay=%.2f dur=%d",view,view->getId(),totalDelay,params->index,params->count,mDelay,mAnimation->getDuration());
    return normalizedDelay * totalDelay;
}

int LayoutAnimationController::getTransformedIndex(const AnimationParameters* params){
    switch (getOrder()) {
    case ORDER_REVERSE: return params->count - 1 - params->index;
    case ORDER_RANDOM :
#if defined(__linux__)||defined(__unix__)
        return static_cast<int> (params->count * drand48());
#else
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(0.0, 1.0);
            return static_cast<int>(params->count * dis(gen));
        }
#endif
    case ORDER_NORMAL : default: return params->index;
    }
}

}
