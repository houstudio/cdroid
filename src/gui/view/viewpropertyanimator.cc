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
#include <view/view.h>
#include <view/viewgroup.h>
#include <widget/cdwindow.h>
#include <view/viewpropertyanimator.h>
#include "cdlog.h"

namespace cdroid{

ViewPropertyAnimator::ViewPropertyAnimator(View* view){
    mView = view;
    view->ensureTransformationInfo();
    mDurationSet = false;
    mStartDelaySet = false;
    mInterpolatorSet = false;
    mInterpolator = nullptr;
    mTempValueAnimator = nullptr;
    mAnimationStarter = [this](){startAnimation();};

    mAnimatorEventListener.onAnimationStart = [this](Animator&animation,bool reverse){
        auto it = mAnimatorSetupMap.find(&animation);
        if (it!=mAnimatorSetupMap.end()){
            if(it->second)it->second();
            mAnimatorSetupMap.erase(it);
        }

        it = mAnimatorOnStartMap.find(&animation);
        if (it!=mAnimatorOnStartMap.end()) {
            if(it->second)it->second();
            mAnimatorOnStartMap.erase(it);
        }

        if (mListener.onAnimationStart) {
            mListener.onAnimationStart(animation,reverse);
        }
    };

    mAnimatorEventListener.onAnimationCancel = [this](Animator&animation){
        if (mListener.onAnimationCancel) {
            mListener.onAnimationCancel(animation);
        }
        auto it= mAnimatorOnEndMap.find(&animation);
        if (mAnimatorOnEndMap.end()!=it) {
            mAnimatorOnEndMap.erase(it);
        }
    };

    mAnimatorEventListener.onAnimationRepeat = [this](Animator&animation){
        if (mListener.onAnimationRepeat) {
            mListener.onAnimationRepeat(animation);
        }
    };

    mAnimatorEventListener.onAnimationEnd = [this](Animator&animation,bool reverse){
        mView->setHasTransientState(false);
        auto it = mAnimatorCleanupMap.find(&animation);
        if ( (it!=mAnimatorCleanupMap.end()) && it->second) {
            it->second();
            mAnimatorCleanupMap.erase(it);
        }
        if (mListener.onAnimationEnd) {
            mListener.onAnimationEnd(animation,reverse);
        }
        it = mAnimatorOnEndMap.find(&animation);
        if ( (it!=mAnimatorOnEndMap.end()) && it->second){
            it->second();
            mAnimatorOnEndMap.erase(it);
        }
        auto it2 = mAnimatorMap.find(&animation);
        if(it2!=mAnimatorMap.end()){
            delete it2->first;
            mAnimatorMap.erase(it2);
        }
    };

    mAnimatorEventListener2 = [this](ValueAnimator&animation){
        auto it = mAnimatorMap.find(&animation);
        if (it == mAnimatorMap.end()) {
            // Shouldn't happen, but just to play it safe
            return;
        }

        const bool hardwareAccelerated = mView->isHardwareAccelerated();
        PropertyBundle& propertyBundle =it->second;/* = mAnimatorMap.get(animation);*/

        // alpha requires slightly different treatment than the other (transform) properties.
        // The logic in setAlpha() is not simply setting mAlpha, plus the invalidation
        // logic is dependent on how the view handles an internal call to onSetAlpha().
        // We track what kinds of properties are set, and how alpha is handled when it is
        // set, and perform the invalidation steps appropriately.
        bool alphaHandled = false;
        if (!hardwareAccelerated) {
            mView->invalidateParentCaches();
        }
        const float fraction = animation.getAnimatedFraction();
        const int propertyMask = propertyBundle.mPropertyMask;
        if ((propertyMask & TRANSFORM_MASK) != 0) {
            mView->invalidateViewProperty(hardwareAccelerated, false);
        }
        std::vector<NameValuesHolder>& valueList = propertyBundle.mNameValuesHolder;

        const int count = valueList.size();
        RenderNode* node = mView->mRenderNode;
        Matrix matrix;
        Rect rect1,rect2;
        node->getMatrix(matrix);
        rect1.set(mView->getLeft(),mView->getTop(), mView->getWidth(),mView->getHeight());
        matrix.transform_rectangle((Cairo::RectangleInt&)rect1);

        for (int i = 0; i < count; ++i) {
            NameValuesHolder& values = valueList.at(i);
            const float value = values.mFromValue + fraction * values.mDeltaValue;
            /*if (values.mNameConstant == ALPHA) {//must be setted in setValue
                alphaHandled = mView->setAlphaNoInvalidation(value);
            } else */{
                setValue(values.mNameConstant, value);
            }
        }
        node->getMatrix(matrix);
        //LOGD("matrix=(%.3f,%.3f , %.3f,%.3f , %.3f,%.3f) node.scale=%.3f,%.3f",matrix.xx,matrix.yx,matrix.xy,matrix.yy,
        //    matrix.x0,matrix.y0,node->getScaleX(),node->getScaleY());
        rect2.set(mView->getLeft(),mView->getTop(), mView->getWidth(),mView->getHeight());
        matrix.transform_rectangle((Cairo::RectangleInt&)rect2);
        rect1.inflate(1,1);rect2.inflate(1,1);
        rect2.Union(rect1);
        if(mView->mParent){
            mView->mParent->invalidate(rect2);
        } else if(dynamic_cast<Window*>(mView)){
            mView->invalidate(rect2);
        }

        if ((propertyMask & TRANSFORM_MASK) != 0) {
            if (!hardwareAccelerated) {
                mView->mPrivateFlags |= View::PFLAG_DRAWN; // force another invalidation
            }
        }
        // invalidate(false) in all cases except if alphaHandled gets set to true
        // via the call to setAlphaNoInvalidation(), above
        if (alphaHandled) {
            mView->invalidate(true);
        } else {
            mView->invalidateViewProperty(false, false);
        }
        if (mUpdateListener != nullptr) {
            mUpdateListener(animation);
        }
    }; 
}

ViewPropertyAnimator::~ViewPropertyAnimator(){
    delete mTempValueAnimator;
    for(auto am:mAnimatorMap){
        delete am.first;
    }
    mAnimatorMap.clear();
}

ViewPropertyAnimator& ViewPropertyAnimator::setDuration(long duration) {
    LOGE_IF(duration<0,"Animators cannot have negative duration:%d" ,duration);
    mDurationSet = true;
    mDuration = duration;
    return *this;
}

long ViewPropertyAnimator::getDuration(){
    if (mDurationSet) {
        return mDuration;
    } else {
        // Just return the default from ValueAnimator, since that's what we'd get if
        // the value has not been set otherwise
        if (mTempValueAnimator == nullptr) {
            mTempValueAnimator = new ValueAnimator();
        }
        return mTempValueAnimator->getDuration();
    }
}

ViewPropertyAnimator& ViewPropertyAnimator::setStartDelay(long startDelay){
    LOGE_IF(startDelay<0,"Animators cannot have negative start delay: %d",startDelay);
    mStartDelaySet = true;
    mStartDelay = startDelay;
    return *this;
}

long ViewPropertyAnimator::getStartDelay()const{
    if (mStartDelaySet) {
        return mStartDelay;
    } else {
        // Just return the default from ValueAnimator (0), since that's what we'd get if
        // the value has not been set otherwise
        return 0;
    }
}

ViewPropertyAnimator& ViewPropertyAnimator::setInterpolator(const TimeInterpolator* interpolator){
    mInterpolatorSet = true;
    mInterpolator = interpolator;
    return *this;
}

const TimeInterpolator* ViewPropertyAnimator::getInterpolator() {
    if (mInterpolatorSet) {
        return mInterpolator;
    } else {
        // Just return the default from ValueAnimator, since that's what we'd get if
        // the value has not been set otherwise
        if (mTempValueAnimator == nullptr) {
            mTempValueAnimator = new ValueAnimator();
        }
        return mTempValueAnimator->getInterpolator();
    }
}

ViewPropertyAnimator& ViewPropertyAnimator::setListener(const Animator::AnimatorListener& listener) {
    mListener = listener;
    return *this;
}

Animator::AnimatorListener ViewPropertyAnimator::getListener()const{
    return mListener;
}

ViewPropertyAnimator& ViewPropertyAnimator::setUpdateListener(const ValueAnimator::AnimatorUpdateListener& listener) {
    mUpdateListener = listener;
    return *this;
}

ValueAnimator::AnimatorUpdateListener ViewPropertyAnimator::getUpdateListener()const{
    return mUpdateListener;
}

void ViewPropertyAnimator::start() {
    mView->removeCallbacks(mAnimationStarter);
    startAnimation();
}

std::set<Animator*>ViewPropertyAnimator::map2set(const std::unordered_map<Animator*, ViewPropertyAnimator::PropertyBundle>&amap){
    std::set<Animator*>aset;
    for(auto a:amap){
        aset.insert(a.first);
    }
    return aset;
}

void ViewPropertyAnimator::cancel() {
    if (mAnimatorMap.size() > 0) {
        std::set<Animator*>aset= map2set(mAnimatorMap);
        for (auto a: aset) {
            a->cancel();
        }
    }
    mPendingAnimations.clear();
    mPendingSetupAction   = nullptr;
    mPendingCleanupAction = nullptr;
    mPendingOnStartAction = nullptr;
    mPendingOnEndAction   = nullptr;
    mView->removeCallbacks(mAnimationStarter);
}

ViewPropertyAnimator& ViewPropertyAnimator::x(float value){
    animateProperty(X, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::xBy(float value){
    animatePropertyBy(X, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::y(float value){
    animateProperty(Y, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::yBy(float value){
    animatePropertyBy(Y, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::z(float value){
    animateProperty(Z, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::zBy(float value){
    animatePropertyBy(Y, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::rotation(float value){
    animateProperty(ROTATION, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::rotationBy(float value){
    animatePropertyBy(ROTATION, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::rotationX(float value){
    animateProperty(ROTATION_X, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::rotationXBy(float value){
    animatePropertyBy(ROTATION_X, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::rotationY(float value){
    animateProperty(ROTATION_Y, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::rotationYBy(float value){
    animatePropertyBy(ROTATION_Y, value);
    return *this;
}


ViewPropertyAnimator& ViewPropertyAnimator::translationX(float value){
    animateProperty(TRANSLATION_X, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::translationXBy(float value){
    animatePropertyBy(TRANSLATION_X, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::translationY(float value){
    animateProperty(TRANSLATION_Y, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::translationYBy(float value){
    animatePropertyBy(TRANSLATION_Y, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::translationZ(float value){
    animateProperty(TRANSLATION_Z, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::translationZBy(float value){
    animatePropertyBy(TRANSLATION_Z, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::scaleX(float value){
    animateProperty(SCALE_X, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::scaleXBy(float value){
    animatePropertyBy(SCALE_X, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::scaleY(float value){
    animateProperty(SCALE_Y, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::scaleYBy(float value){
    animatePropertyBy(SCALE_Y, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::alpha(float value){
    animateProperty(ALPHA, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::alphaBy(float value){
    animatePropertyBy(ALPHA, value);
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::withStartAction(Runnable runnable){
    mPendingOnStartAction = runnable;
    return *this;
}

ViewPropertyAnimator& ViewPropertyAnimator::withEndAction(Runnable runnable){
    mPendingOnEndAction = runnable;
    return *this;
}

bool ViewPropertyAnimator::hasActions()const{
    return  (mPendingSetupAction!=nullptr)
                || (mPendingCleanupAction!=nullptr)
                || (mPendingOnStartAction!=nullptr)
                || (mPendingOnEndAction==nullptr);
}

void ViewPropertyAnimator::startAnimation(){
    mView->setHasTransientState(true);
    ValueAnimator* animator = ValueAnimator::ofFloat({0,1.0f});
    std::vector<NameValuesHolder> nameValueList =mPendingAnimations;
    mPendingAnimations.clear();
    int propertyMask = 0;
    const int propertyCount = nameValueList.size();
    for (int i = 0; i < propertyCount; ++i) {
        NameValuesHolder& nameValuesHolder = nameValueList.at(i);
        propertyMask |= nameValuesHolder.mNameConstant;
    }
    mAnimatorMap.insert(std::pair<Animator*,PropertyBundle>(animator,PropertyBundle(propertyMask, nameValueList)));
    if (mPendingSetupAction) {
        mAnimatorSetupMap.insert(std::pair<Animator*,Runnable>(animator, mPendingSetupAction));
        mPendingSetupAction = nullptr;
    }
    if (mPendingCleanupAction) {
        mAnimatorCleanupMap.insert(std::pair<Animator*,Runnable>(animator, mPendingCleanupAction));
        mPendingCleanupAction = nullptr;
    }
    if (mPendingOnStartAction) {
        mAnimatorOnStartMap.insert(std::pair<Animator*,Runnable>(animator, mPendingOnStartAction));
        mPendingOnStartAction = nullptr;
    }
    if (mPendingOnEndAction) {
        mAnimatorOnEndMap.insert(std::pair<Animator*,Runnable>(animator, mPendingOnEndAction));
        mPendingOnEndAction = nullptr;
    }
    animator->addUpdateListener(mAnimatorEventListener2);
    animator->addListener(mAnimatorEventListener);
    if (mStartDelaySet) {
        animator->setStartDelay(mStartDelay);
    }
    if (mDurationSet) {
        animator->setDuration(mDuration);
    }
    if (mInterpolatorSet) {
        animator->setInterpolator(mInterpolator);
    }
    animator->start();
}

void ViewPropertyAnimator::animateProperty(int constantName, float toValue){
    const float fromValue = getValue(constantName);
    const float deltaValue = toValue - fromValue;
    animatePropertyBy(constantName, fromValue, deltaValue);
}

void ViewPropertyAnimator::animatePropertyBy(int constantName, float byValue){
    const float fromValue = getValue(constantName);
    animatePropertyBy(constantName, fromValue, byValue);
}

void ViewPropertyAnimator::animatePropertyBy(int constantName, float startValue, float byValue){
    if (mAnimatorMap.size() > 0) {
        Animator* animatorToCancel = nullptr;
        std::set<Animator*> animatorSet = map2set(mAnimatorMap);
        for (Animator* runningAnim : animatorSet) {
            auto it =mAnimatorMap.find(runningAnim);
            PropertyBundle& bundle = it->second;
            if (bundle.cancel(constantName)) {
                // property was canceled - cancel the animation if it's now empty
                // Note that it's safe to break out here because every new animation
                // on a property will cancel a previous animation on that property, so
                // there can only ever be one such animation running.
                if (bundle.mPropertyMask == NONE) {
                    // the animation is no longer changing anything - cancel it
                    animatorToCancel = runningAnim;
                    break;
                }
            }
        }
        if (animatorToCancel) animatorToCancel->cancel();
    }

    NameValuesHolder nameValuePair = NameValuesHolder(constantName, startValue, byValue);
    mPendingAnimations.push_back(std::move(nameValuePair));
    mView->removeCallbacks(mAnimationStarter);
    mView->postOnAnimation(mAnimationStarter);
}

void ViewPropertyAnimator::setValue(int propertyConstant, float value) {
    RenderNode* node = mView->mRenderNode;
    switch (propertyConstant) {
    case TRANSLATION_X: node->setTranslationX(value);     break;
    case TRANSLATION_Y: node->setTranslationY(value);     break;
    case TRANSLATION_Z: node->setTranslationZ(value);     break;
    case ROTATION:      node->setRotation(value);         break;
    case ROTATION_X:    node->setRotationX(value);        break;
    case ROTATION_Y:    node->setRotationY(value);        break;
    case SCALE_X:       node->setScaleX(value);           break;
    case SCALE_Y:       node->setScaleY(value);           break;
    case X:  node->setTranslationX(value - mView->mLeft); break;
    case Y:  node->setTranslationY(value - mView->mTop ); break;
    case Z:  node->setTranslationZ(value - node->getElevation());  break;
    case ALPHA:
             mView->mTransformationInfo->mAlpha = value;
             node->setAlpha(value);
             break;
    }
}

float ViewPropertyAnimator::getValue(int propertyConstant)const{
    RenderNode* node = mView->mRenderNode;
    switch (propertyConstant) {
    case TRANSLATION_X: return node->getTranslationX();
    case TRANSLATION_Y: return node->getTranslationY();
    case TRANSLATION_Z: return node->getTranslationZ();
    case ROTATION:   return node->getRotation();
    case ROTATION_X: return node->getRotationX();
    case ROTATION_Y: return node->getRotationY();
    case SCALE_X:  return node->getScaleX();
    case SCALE_Y:  return node->getScaleY();
    case X:     return mView->mLeft + node->getTranslationX();
    case Y:     return mView->mTop + node->getTranslationY();
    case Z:     return node->getElevation() + node->getTranslationZ();
    case ALPHA: return mView->mTransformationInfo->mAlpha;
    }
    return 0;
}

ViewPropertyAnimator::PropertyBundle::PropertyBundle(int propertyMask,const std::vector<NameValuesHolder>& nameValuesHolder){
    mPropertyMask = propertyMask;
    mNameValuesHolder = nameValuesHolder;
}

bool ViewPropertyAnimator::PropertyBundle::cancel(int propertyConstant){
    if ((mPropertyMask & propertyConstant) != 0 ) {
        for (auto it = mNameValuesHolder.begin();it!=mNameValuesHolder.end();it++){
            NameValuesHolder& nameValuesHolder = (*it);
            if (nameValuesHolder.mNameConstant == propertyConstant) {
                it = mNameValuesHolder.erase(it);
                mPropertyMask &= ~propertyConstant;
                return true;
            }
        }
    }
    return false;
}

}//endof namespace
