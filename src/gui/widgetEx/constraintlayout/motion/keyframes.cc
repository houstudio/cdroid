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

/*
 * Ported to C++ for CDROID from androidx.constraintlayout.motion.widget.KeyFrames.
 */
#include <widgetEx/constraintlayout/motion/keyframes.h>

#include <widgetEx/widgetex_styleable.h>
#include <core/assets.h>
#include <core/xmlpullparser.h>

#include <widgetEx/constraintlayout/core/motion/motionkeyattributes.h>
#include <widgetEx/constraintlayout/core/motion/motionkeycycle.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyposition.h>
#include <widgetEx/constraintlayout/core/motion/motionkeytimecycle.h>
#include <widgetEx/constraintlayout/core/motion/motionkeytrigger.h>

namespace cdroid {

namespace {
// Enum-name -> int map for keyPositionType. The static constexpr position-type constants are cast
// to int inline at the use site (a prvalue) to avoid odr-using them.
const std::unordered_map<std::string, int> kPositionType = {
    {"deltaRelative",  (int)MotionKeyPosition::TYPE_CARTESIAN},
    {"cartesian",      (int)MotionKeyPosition::TYPE_CARTESIAN},
    {"pathRelative",   (int)MotionKeyPosition::TYPE_PATH},
    {"path",           (int)MotionKeyPosition::TYPE_PATH},
    {"parentRelative", (int)MotionKeyPosition::TYPE_SCREEN},
    {"screen",         (int)MotionKeyPosition::TYPE_SCREEN},
    {"axisRelative",   (int)MotionKeyPosition::TYPE_AXIS},
    {"axis",           (int)MotionKeyPosition::TYPE_AXIS}
};

// Read the attributes common to every keyframe type: motionTarget (view id) + framePosition (0..100).
void loadCommon(MotionKey& k, const AttributeSet& a) {
    k.mViewId = a.getResourceId("motionTarget", k.mViewId);
    k.mFramePosition = a.getInt("framePosition", k.mFramePosition);
}

std::unique_ptr<MotionKey> makeKeyAttribute(const AttributeSet& a) {
    auto k = std::make_unique<MotionKeyAttributes>();
    loadCommon(*k, a);
    Context* ctx = a.getContext();
    Assets* _assets = ctx ? dynamic_cast<Assets*>(ctx) : nullptr;
    auto ta = _assets ? _assets->obtainStyledAttributesTyped(
        a, styleable::KeyAttribute::IDS, styleable::KeyAttribute::COUNT) : nullptr;
    namespace SKA = styleable::KeyAttribute;
    k->mAlpha       = ta&&ta->hasValue(SKA::alpha) ? ta->getFloat(SKA::alpha, k->mAlpha) : a.getFloat("alpha", k->mAlpha);
    k->mElevation   = ta&&ta->hasValue(SKA::elevation) ? ta->getFloat(SKA::elevation, k->mElevation) : a.getFloat("elevation", k->mElevation);
    k->mRotation    = ta&&ta->hasValue(SKA::rotation) ? ta->getFloat(SKA::rotation, k->mRotation) : a.getFloat("rotation", k->mRotation);
    k->mRotationX   = ta&&ta->hasValue(SKA::rotationX) ? ta->getFloat(SKA::rotationX, k->mRotationX) : a.getFloat("rotationX", k->mRotationX);
    k->mRotationY   = ta&&ta->hasValue(SKA::rotationY) ? ta->getFloat(SKA::rotationY, k->mRotationY) : a.getFloat("rotationY", k->mRotationY);
    k->mPivotX      = ta&&ta->hasValue(SKA::transformPivotX) ? ta->getFloat(SKA::transformPivotX, k->mPivotX) : a.getFloat("transformPivotX", k->mPivotX);
    k->mPivotY      = ta&&ta->hasValue(SKA::transformPivotY) ? ta->getFloat(SKA::transformPivotY, k->mPivotY) : a.getFloat("transformPivotY", k->mPivotY);
    k->mScaleX      = ta&&ta->hasValue(SKA::scaleX) ? ta->getFloat(SKA::scaleX, k->mScaleX) : a.getFloat("scaleX", k->mScaleX);
    k->mScaleY      = ta&&ta->hasValue(SKA::scaleY) ? ta->getFloat(SKA::scaleY, k->mScaleY) : a.getFloat("scaleY", k->mScaleY);
    k->mTranslationX = ta&&ta->hasValue(SKA::translationX) ? ta->getFloat(SKA::translationX, k->mTranslationX) : a.getFloat("translationX", k->mTranslationX);
    k->mTranslationY = ta&&ta->hasValue(SKA::translationY) ? ta->getFloat(SKA::translationY, k->mTranslationY) : a.getFloat("translationY", k->mTranslationY);
    k->mTranslationZ = ta&&ta->hasValue(SKA::translationZ) ? ta->getFloat(SKA::translationZ, k->mTranslationZ) : a.getFloat("translationZ", k->mTranslationZ);
    k->mTransitionPathRotate = ta&&ta->hasValue(SKA::transitionPathRotate) ? ta->getFloat(SKA::transitionPathRotate, k->mTransitionPathRotate) : a.getFloat("transitionPathRotate", k->mTransitionPathRotate);
    k->mProgress    = ta&&ta->hasValue(SKA::motionProgress) ? ta->getFloat(SKA::motionProgress, k->mProgress) : a.getFloat("motionProgress", k->mProgress);
    k->mCurveFit    = ta&&ta->hasValue(SKA::curveFit) ? ta->getInt(SKA::curveFit, k->mCurveFit) : a.getInt("curveFit", k->mCurveFit);
    k->mVisibility  = a.getBoolean("visibility", k->mVisibility != 0) ? 1 : 0;
    return k;
}

std::unique_ptr<MotionKey> makeKeyPosition(const AttributeSet& a) {
    auto k = std::make_unique<MotionKeyPosition>();
    loadCommon(*k, a);
    Context* ctx = a.getContext();
    Assets* _assets = ctx ? dynamic_cast<Assets*>(ctx) : nullptr;
    auto ta = _assets ? _assets->obtainStyledAttributesTyped(
        a, styleable::KeyPosition::IDS, styleable::KeyPosition::COUNT) : nullptr;
    namespace SKP = styleable::KeyPosition;
    k->mTransitionEasing = ta&&ta->hasValue(SKP::transitionEasing) ? ta->getString(SKP::transitionEasing) : a.getString("transitionEasing", k->mTransitionEasing);
    k->mDrawPath       = ta&&ta->hasValue(SKP::drawPath) ? ta->getInt(SKP::drawPath, k->mDrawPath) : a.getInt("drawPath", k->mDrawPath);
    k->mPercentX       = ta&&ta->hasValue(SKP::percentX) ? ta->getFloat(SKP::percentX, k->mPercentX) : a.getFloat("percentX", k->mPercentX);
    k->mPercentY       = ta&&ta->hasValue(SKP::percentY) ? ta->getFloat(SKP::percentY, k->mPercentY) : a.getFloat("percentY", k->mPercentY);
    k->mPercentWidth   = ta&&ta->hasValue(SKP::percentWidth) ? ta->getFloat(SKP::percentWidth, k->mPercentWidth) : a.getFloat("percentWidth", k->mPercentWidth);
    k->mPercentHeight  = ta&&ta->hasValue(SKP::percentHeight) ? ta->getFloat(SKP::percentHeight, k->mPercentHeight) : a.getFloat("percentHeight", k->mPercentHeight);
    k->mAltPercentX    = ta&&ta->hasValue(SKP::sizePercent) ? ta->getFloat(SKP::sizePercent, k->mAltPercentX) : a.getFloat("sizePercent", k->mAltPercentX);
    k->mPathMotionArc  = ta&&ta->hasValue(SKP::pathMotionArc) ? ta->getInt(SKP::pathMotionArc, k->mPathMotionArc) : a.getInt("pathMotionArc", k->mPathMotionArc);
    k->mPositionType   = ta&&ta->hasValue(SKP::keyPositionType) ? ta->getInt(SKP::keyPositionType, k->mPositionType) : a.getInt("keyPositionType", kPositionType, k->mPositionType);
    return k;
}

// KeyCycle and KeyTimeCycle share the same attribute set (wave params + transform values).
template <typename KeyT>
std::unique_ptr<MotionKey> makeKeyCycle(const AttributeSet& a) {
    auto k = std::make_unique<KeyT>();
    loadCommon(*k, a);
    Context* ctx = a.getContext();
    Assets* _assets = ctx ? dynamic_cast<Assets*>(ctx) : nullptr;
    auto ta = _assets ? _assets->obtainStyledAttributesTyped(
        a, styleable::KeyCycle::IDS, styleable::KeyCycle::COUNT) : nullptr;
    namespace SKC = styleable::KeyCycle;
    k->mWaveShape  = ta&&ta->hasValue(SKC::waveShape) ? ta->getInt(SKC::waveShape, k->mWaveShape) : a.getInt("waveShape", k->mWaveShape);
    k->mWavePeriod = ta&&ta->hasValue(SKC::wavePeriod) ? ta->getFloat(SKC::wavePeriod, k->mWavePeriod) : a.getFloat("wavePeriod", k->mWavePeriod);
    k->mWaveOffset = ta&&ta->hasValue(SKC::waveOffset) ? ta->getFloat(SKC::waveOffset, k->mWaveOffset) : a.getFloat("waveOffset", k->mWaveOffset);
    k->mAlpha       = ta&&ta->hasValue(SKC::alpha) ? ta->getFloat(SKC::alpha, k->mAlpha) : a.getFloat("alpha", k->mAlpha);
    k->mElevation   = ta&&ta->hasValue(SKC::elevation) ? ta->getFloat(SKC::elevation, k->mElevation) : a.getFloat("elevation", k->mElevation);
    k->mRotation    = ta&&ta->hasValue(SKC::rotation) ? ta->getFloat(SKC::rotation, k->mRotation) : a.getFloat("rotation", k->mRotation);
    k->mRotationX   = ta&&ta->hasValue(SKC::rotationX) ? ta->getFloat(SKC::rotationX, k->mRotationX) : a.getFloat("rotationX", k->mRotationX);
    k->mRotationY   = ta&&ta->hasValue(SKC::rotationY) ? ta->getFloat(SKC::rotationY, k->mRotationY) : a.getFloat("rotationY", k->mRotationY);
    k->mScaleX      = ta&&ta->hasValue(SKC::scaleX) ? ta->getFloat(SKC::scaleX, k->mScaleX) : a.getFloat("scaleX", k->mScaleX);
    k->mScaleY      = ta&&ta->hasValue(SKC::scaleY) ? ta->getFloat(SKC::scaleY, k->mScaleY) : a.getFloat("scaleY", k->mScaleY);
    k->mTranslationX = ta&&ta->hasValue(SKC::translationX) ? ta->getFloat(SKC::translationX, k->mTranslationX) : a.getFloat("translationX", k->mTranslationX);
    k->mTranslationY = ta&&ta->hasValue(SKC::translationY) ? ta->getFloat(SKC::translationY, k->mTranslationY) : a.getFloat("translationY", k->mTranslationY);
    k->mTranslationZ = ta&&ta->hasValue(SKC::translationZ) ? ta->getFloat(SKC::translationZ, k->mTranslationZ) : a.getFloat("translationZ", k->mTranslationZ);
    k->mTransitionPathRotate = ta&&ta->hasValue(SKC::transitionPathRotate) ? ta->getFloat(SKC::transitionPathRotate, k->mTransitionPathRotate) : a.getFloat("transitionPathRotate", k->mTransitionPathRotate);
    k->mProgress    = ta&&ta->hasValue(SKC::motionProgress) ? ta->getFloat(SKC::motionProgress, k->mProgress) : a.getFloat("motionProgress", k->mProgress);
    return k;
}

std::unique_ptr<MotionKey> makeKeyTrigger(const AttributeSet& a) {
    auto k = std::make_unique<MotionKeyTrigger>();
    loadCommon(*k, a);
    Context* ctx = a.getContext();
    Assets* _assets = ctx ? dynamic_cast<Assets*>(ctx) : nullptr;
    auto ta = _assets ? _assets->obtainStyledAttributesTyped(
        a, styleable::KeyTrigger::IDS, styleable::KeyTrigger::COUNT) : nullptr;
    namespace SKT = styleable::KeyTrigger;
    k->mCross         = ta&&ta->hasValue(SKT::onCross) ? ta->getString(SKT::onCross) : a.getString("onCross", k->mCross);
    k->mPositiveCross = ta&&ta->hasValue(SKT::onPositiveCross) ? ta->getString(SKT::onPositiveCross) : a.getString("onPositiveCross", k->mPositiveCross);
    k->mNegativeCross = ta&&ta->hasValue(SKT::onNegativeCross) ? ta->getString(SKT::onNegativeCross) : a.getString("onNegativeCross", k->mNegativeCross);
    k->mTriggerID     = ta&&ta->hasValue(SKT::triggerId) ? (int)ta->getResourceId(SKT::triggerId, k->mTriggerID) : a.getResourceId("triggerId", k->mTriggerID);
    k->mTriggerReceiver = ta&&ta->hasValue(SKT::triggerReceiver) ? (int)ta->getResourceId(SKT::triggerReceiver, k->mTriggerReceiver) : a.getResourceId("triggerReceiver", k->mTriggerReceiver);
    k->mTriggerSlack  = ta&&ta->hasValue(SKT::triggerSlack) ? ta->getFloat(SKT::triggerSlack, k->mTriggerSlack) : a.getFloat("triggerSlack", k->mTriggerSlack);
    return k;
}
} // namespace

KeyFrames::KeyFrames(Context* /*ctx*/, XmlPullParser& parser) {
    // `parser` is at the <KeyFrameSet> START_TAG. Walk children until the matching END_TAG.
    while (parser.getEventType() != XmlPullParser::END_DOCUMENT &&
            parser.getEventType() != XmlPullParser::BAD_DOCUMENT) {
        const int eventType = parser.getEventType();
        if (eventType == XmlPullParser::START_TAG) {
            const std::string tag = parser.getName();
            std::unique_ptr<MotionKey> key;
            if (tag == "KeyAttribute")      key = makeKeyAttribute(parser);
            else if (tag == "KeyPosition")  key = makeKeyPosition(parser);
            else if (tag == "KeyCycle")     key = makeKeyCycle<MotionKeyCycle>(parser);
            else if (tag == "KeyTimeCycle") key = makeKeyCycle<MotionKeyTimeCycle>(parser);
            else if (tag == "KeyTrigger")   key = makeKeyTrigger(parser);
            if (key) addKey(std::move(key));
        } else if (eventType == XmlPullParser::END_TAG) {
            if (parser.getName() == "KeyFrameSet") return; // consumed the set
        }
        parser.next();
    }
}

void KeyFrames::addKey(std::unique_ptr<MotionKey> key) {
    if (!key) return;
    mFramesMap[key->mViewId].push_back(std::move(key));
}

std::vector<MotionKey*> KeyFrames::getKeysForView(int viewId) const {
    std::vector<MotionKey*> out;
    auto it = mFramesMap.find(viewId);
    if (it != mFramesMap.end()) {
        for (auto& k : it->second) out.push_back(k.get());
    }
    // Apply-to-all keys (target = UNSET) also apply to every view.
    auto all = mFramesMap.find((int)MotionKey::UNSET);
    if (all != mFramesMap.end()) {
        for (auto& k : all->second) out.push_back(k.get());
    }
    return out;
}

std::vector<MotionKey*> KeyFrames::getAllKeys() const {
    std::vector<MotionKey*> out;
    for (auto& kv : mFramesMap) {
        for (auto& k : kv.second) out.push_back(k.get());
    }
    return out;
}

std::vector<int> KeyFrames::getTargets() const {
    std::vector<int> targets;
    targets.reserve(mFramesMap.size());
    for (const auto& kv : mFramesMap) targets.push_back(kv.first);
    return targets;
}

} // namespace cdroid
