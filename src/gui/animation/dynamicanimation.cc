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
*/
#include <animation/dynamicanimation.h>

namespace cdroid{

#define VIEW_PROPERTY(PROPNAME, METHOD, PROJ)               \
namespace {                                                 \
class prop_##PROJ : public FloatProperty {                  \
public:                                                     \
    prop_##PROJ() : FloatProperty(PROPNAME) {}              \
    void set(void*obj,const AnimateValue& v)const override{ \
        ((View*)obj)->set##METHOD(GET_VARIANT(v,float));    \
    }                                                       \
    AnimateValue get(void* obj)const override{              \
        return ((View*)obj)->get##METHOD();                 \
    }                                                       \
};                                                          \
static prop_##PROJ INST_##PROJ;                             \
}                                                           \
const FloatProperty& DynamicAnimation::PROJ =INST_##PROJ;

VIEW_PROPERTY("translationX",TranslationX, TRANSLATION_X);
VIEW_PROPERTY("translationY",TranslationY, TRANSLATION_Y);
VIEW_PROPERTY("translationZ",TranslationZ, TRANSLATION_Z);

VIEW_PROPERTY("scaleX",ScaleX, SCALE_X);
VIEW_PROPERTY("scaleY",ScaleY, SCALE_Y);

VIEW_PROPERTY("rotation",Rotation, ROTATION);
VIEW_PROPERTY("rotationX",RotationX, ROTATION_X);
VIEW_PROPERTY("rotationY",RotationY, ROTATION_Y);

VIEW_PROPERTY("x",X, X);
VIEW_PROPERTY("y",Y, Y);
VIEW_PROPERTY("z",Z, Z);
VIEW_PROPERTY("alpha",Alpha, ALPHA);

VIEW_PROPERTY("scrollX",ScrollX, SCROLL_X);
VIEW_PROPERTY("scrollY",ScrollY, SCROLL_Y);

class FloatValueHolderProperty:public FloatProperty{
public:
    FloatValueHolderProperty():FloatProperty("FloatValueHolder"){}
    void set(void*obj,const AnimateValue& v)const override{
        ((FloatValueHolder*)obj)->setValue(GET_VARIANT(v,float));
    }
    AnimateValue get(void* obj)const override{
        return ((FloatValueHolder*)obj)->getValue();
    }
};

static  FloatValueHolderProperty FLOATVALUEHOLDER;

DynamicAnimation::DynamicAnimation(FloatValueHolder* floatValueHolder) {
    mTarget = floatValueHolder;
    mProperty = &FLOATVALUEHOLDER;
    mMinVisibleChange = MIN_VISIBLE_CHANGE_PIXELS;
}

DynamicAnimation::DynamicAnimation(void* object,const FloatProperty* property) {
    mTarget = object;
    mProperty = property;
    if ((mProperty == &ROTATION) || (mProperty == &ROTATION_X) || (mProperty == &ROTATION_Y)) {
        mMinVisibleChange = MIN_VISIBLE_CHANGE_ROTATION_DEGREES;
    } else if (mProperty == &ALPHA) {
        mMinVisibleChange = MIN_VISIBLE_CHANGE_ALPHA;
    } else if ((mProperty == &SCALE_X) || (mProperty == &SCALE_Y)) {
        mMinVisibleChange = MIN_VISIBLE_CHANGE_ALPHA;
    } else {
        mMinVisibleChange = MIN_VISIBLE_CHANGE_PIXELS;
    }
}

DynamicAnimation::~DynamicAnimation(){
}

DynamicAnimation& DynamicAnimation::setStartValue(float startValue) {
    mValue = startValue;
    mStartValueIsSet = true;
    return *this;
}

DynamicAnimation& DynamicAnimation::setStartVelocity(float startVelocity) {
    mVelocity = startVelocity;
    return *this;
}

DynamicAnimation& DynamicAnimation::setMaxValue(float max) {
    // This max value should be checked and handled in the subclass animations, instead of
    // assuming the end of the animations when the max/min value is hit in the base class.
    // The reason is that hitting max/min value may just be a transient state, such as during
    // the spring oscillation.
    mMaxValue = max;
    return *this;
}

DynamicAnimation& DynamicAnimation::setMinValue(float min) {
    mMinValue = min;
    return *this;
}

DynamicAnimation& DynamicAnimation::addEndListener(const OnAnimationEndListener& listener) {
    auto it =std::find(mEndListeners.begin(),mEndListeners.end(),listener);
    if (it==mEndListeners.end()) {
        mEndListeners.push_back(listener);
    }
    return *this;
}

namespace {
    template<typename T>
    static void removeNullEntries(std::vector<T> list) {
        // Clean up null entries
        for (auto it= list.begin();it!=list.end();) {
            if ((*it) == nullptr) {
                it=list.erase(it);
            }else{
                it++;
            }
        }
    }
    template<typename T>
    void removeEntry(std::vector<T>& list, T entry) {
        auto it =std::find(list.begin(),list.end(),entry);
        if (it!=list.end()) {
            *it=nullptr;
        }
    }
}

void DynamicAnimation::removeEndListener(const OnAnimationEndListener& listener) {
    removeEntry(mEndListeners, listener);
}

DynamicAnimation& DynamicAnimation::addUpdateListener(const OnAnimationUpdateListener& listener) {
    if (isRunning()) {
        // Require update listener to be added before the animation, such as when we start
        // the animation, we know whether the animation is RenderThread compatible.
        throw std::runtime_error("Error: Update listeners must be added before the animation.");
    }
    auto it = std::find(mUpdateListeners.begin(),mUpdateListeners.end(),listener);
    if (it==mUpdateListeners.end()) {
        mUpdateListeners.push_back(listener);
    }
    return *this;
}

void DynamicAnimation::removeUpdateListener(const OnAnimationUpdateListener& listener) {
    removeEntry(mUpdateListeners, listener);
}

DynamicAnimation& DynamicAnimation::setMinimumVisibleChange(float minimumVisibleChange) {
    if (minimumVisibleChange <= 0) {
        throw std::invalid_argument("Minimum visible change must be positive.");
    }
    mMinVisibleChange = minimumVisibleChange;
    setValueThreshold(minimumVisibleChange * THRESHOLD_MULTIPLIER);
    return *this;
}

float DynamicAnimation::getMinimumVisibleChange() const{
    return mMinVisibleChange;
}

/****************Animation Lifecycle Management***************/

void DynamicAnimation::start() {
    if (Looper::myLooper() != Looper::getMainLooper()) {
        throw std::runtime_error("Animations may only be started on the main thread");
    }
    if (!mRunning) {
        startAnimationInternal();
    }
}

void DynamicAnimation::cancel() {
    if (Looper::myLooper() != Looper::getMainLooper()) {
        throw std::runtime_error("Animations may only be canceled on the main thread");
    }
    if (mRunning) {
        endAnimationInternal(true);
    }
}

bool DynamicAnimation::isRunning() const{
    return mRunning;
}

/************************** Private APIs below ********************************/

// This gets called when the animation is started, to finish the setup of the animation
// before the animation pulsing starts.
void DynamicAnimation::startAnimationInternal() {
    if (!mRunning) {
        mRunning = true;
        if (!mStartValueIsSet) {
            mValue = getPropertyValue();
        }
        // Sanity check:
        if (mValue > mMaxValue || mValue < mMinValue) {
            throw std::invalid_argument("Starting value need to be in between min value and max value");
        }
        AnimationHandler::getInstance().addAnimationFrameCallback(this, 0);
    }
}

bool DynamicAnimation::doAnimationFrame(int64_t frameTime) {
    if (mLastFrameTime == 0) {
        // First frame.
        mLastFrameTime = frameTime;
        setPropertyValue(mValue);
        return false;
    }
    int64_t deltaT = frameTime - mLastFrameTime;
    mLastFrameTime = frameTime;
    bool finished = updateValueAndVelocity(deltaT);
    // Clamp value & velocity.
    mValue = std::min(mValue, mMaxValue);
    mValue = std::max(mValue, mMinValue);

    setPropertyValue(mValue);

    if (finished) {
        endAnimationInternal(false);
    }
    return finished;
}

void DynamicAnimation::commitAnimationFrame(int64_t frameTime){
     
}

void DynamicAnimation::endAnimationInternal(bool canceled) {
    mRunning = false;
    AnimationHandler::getInstance().removeCallback(this);
    mLastFrameTime = 0;
    mStartValueIsSet = false;
    for (int i = 0; i < mEndListeners.size(); i++) {
        auto ls = mEndListeners.at(i);
        if (ls != nullptr) {
            ls(*this, canceled, mValue, mVelocity);
        }
    }
    removeNullEntries(mEndListeners);
}

void DynamicAnimation::setPropertyValue(float value) {
    mProperty->setValue(mTarget, value);
    for (int i = 0; i < mUpdateListeners.size(); i++) {
        auto ls = mUpdateListeners.at(i);
        if (ls != nullptr) {
            ls(*this, mValue, mVelocity);
        }
    }
    removeNullEntries(mUpdateListeners);
}

AnimationHandler& DynamicAnimation::getAnimationHandler()const{
    return AnimationHandler::getInstance();
}

float DynamicAnimation::getValueThreshold() const{
    return mMinVisibleChange * THRESHOLD_MULTIPLIER;
}

float DynamicAnimation::getPropertyValue() const{
    return mProperty->getValue(mTarget);
}
}/*endof namespace*/
