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
    auto ta = ctx->obtainStyledAttributes(a, internal::R::styleable::KeyAttribute);
    k->mAlpha       = ta&&ta->hasValue(internal::R::styleable::KeyAttribute_alpha) ? ta->getFloat(internal::R::styleable::KeyAttribute_alpha, k->mAlpha) : a.getFloat("alpha", k->mAlpha);
    k->mElevation   = ta&&ta->hasValue(internal::R::styleable::KeyAttribute_elevation) ? ta->getFloat(internal::R::styleable::KeyAttribute_elevation, k->mElevation) : a.getFloat("elevation", k->mElevation);
    k->mRotation    = ta&&ta->hasValue(internal::R::styleable::KeyAttribute_rotation) ? ta->getFloat(internal::R::styleable::KeyAttribute_rotation, k->mRotation) : a.getFloat("rotation", k->mRotation);
    k->mRotationX   = ta&&ta->hasValue(internal::R::styleable::KeyAttribute_rotationX) ? ta->getFloat(internal::R::styleable::KeyAttribute_rotationX, k->mRotationX) : a.getFloat("rotationX", k->mRotationX);
    k->mRotationY   = ta&&ta->hasValue(internal::R::styleable::KeyAttribute_rotationY) ? ta->getFloat(internal::R::styleable::KeyAttribute_rotationY, k->mRotationY) : a.getFloat("rotationY", k->mRotationY);
    k->mPivotX      = ta&&ta->hasValue(internal::R::styleable::KeyAttribute_transformPivotX) ? ta->getFloat(internal::R::styleable::KeyAttribute_transformPivotX, k->mPivotX) : a.getFloat("transformPivotX", k->mPivotX);
    k->mPivotY      = ta&&ta->hasValue(internal::R::styleable::KeyAttribute_transformPivotY) ? ta->getFloat(internal::R::styleable::KeyAttribute_transformPivotY, k->mPivotY) : a.getFloat("transformPivotY", k->mPivotY);
    k->mScaleX      = ta&&ta->hasValue(internal::R::styleable::KeyAttribute_scaleX) ? ta->getFloat(internal::R::styleable::KeyAttribute_scaleX, k->mScaleX) : a.getFloat("scaleX", k->mScaleX);
    k->mScaleY      = ta&&ta->hasValue(internal::R::styleable::KeyAttribute_scaleY) ? ta->getFloat(internal::R::styleable::KeyAttribute_scaleY, k->mScaleY) : a.getFloat("scaleY", k->mScaleY);
    k->mTranslationX = ta&&ta->hasValue(internal::R::styleable::KeyAttribute_translationX) ? ta->getFloat(internal::R::styleable::KeyAttribute_translationX, k->mTranslationX) : a.getFloat("translationX", k->mTranslationX);
    k->mTranslationY = ta&&ta->hasValue(internal::R::styleable::KeyAttribute_translationY) ? ta->getFloat(internal::R::styleable::KeyAttribute_translationY, k->mTranslationY) : a.getFloat("translationY", k->mTranslationY);
    k->mTranslationZ = ta&&ta->hasValue(internal::R::styleable::KeyAttribute_translationZ) ? ta->getFloat(internal::R::styleable::KeyAttribute_translationZ, k->mTranslationZ) : a.getFloat("translationZ", k->mTranslationZ);
    k->mTransitionPathRotate = ta&&ta->hasValue(internal::R::styleable::KeyAttribute_transitionPathRotate) ? ta->getFloat(internal::R::styleable::KeyAttribute_transitionPathRotate, k->mTransitionPathRotate) : a.getFloat("transitionPathRotate", k->mTransitionPathRotate);
    k->mProgress    = ta&&ta->hasValue(internal::R::styleable::KeyAttribute_motionProgress) ? ta->getFloat(internal::R::styleable::KeyAttribute_motionProgress, k->mProgress) : a.getFloat("motionProgress", k->mProgress);
    k->mCurveFit    = ta&&ta->hasValue(internal::R::styleable::KeyAttribute_curveFit) ? ta->getInt(internal::R::styleable::KeyAttribute_curveFit, k->mCurveFit) : a.getInt("curveFit", k->mCurveFit);
    k->mVisibility  = a.getBoolean("visibility", k->mVisibility != 0) ? 1 : 0;
    return k;
}

std::unique_ptr<MotionKey> makeKeyPosition(const AttributeSet& a) {
    auto k = std::make_unique<MotionKeyPosition>();
    loadCommon(*k, a);
    Context* ctx = a.getContext();
    auto ta = ctx->obtainStyledAttributes(a, internal::R::styleable::KeyPosition);
    k->mTransitionEasing = ta&&ta->hasValue(internal::R::styleable::KeyPosition_transitionEasing) ? ta->getString(internal::R::styleable::KeyPosition_transitionEasing) : a.getString("transitionEasing", k->mTransitionEasing);
    k->mDrawPath       = ta&&ta->hasValue(internal::R::styleable::KeyPosition_drawPath) ? ta->getInt(internal::R::styleable::KeyPosition_drawPath, k->mDrawPath) : a.getInt("drawPath", k->mDrawPath);
    k->mPercentX       = ta&&ta->hasValue(internal::R::styleable::KeyPosition_percentX) ? ta->getFloat(internal::R::styleable::KeyPosition_percentX, k->mPercentX) : a.getFloat("percentX", k->mPercentX);
    k->mPercentY       = ta&&ta->hasValue(internal::R::styleable::KeyPosition_percentY) ? ta->getFloat(internal::R::styleable::KeyPosition_percentY, k->mPercentY) : a.getFloat("percentY", k->mPercentY);
    k->mPercentWidth   = ta&&ta->hasValue(internal::R::styleable::KeyPosition_percentWidth) ? ta->getFloat(internal::R::styleable::KeyPosition_percentWidth, k->mPercentWidth) : a.getFloat("percentWidth", k->mPercentWidth);
    k->mPercentHeight  = ta&&ta->hasValue(internal::R::styleable::KeyPosition_percentHeight) ? ta->getFloat(internal::R::styleable::KeyPosition_percentHeight, k->mPercentHeight) : a.getFloat("percentHeight", k->mPercentHeight);
    k->mAltPercentX    = ta&&ta->hasValue(internal::R::styleable::KeyPosition_sizePercent) ? ta->getFloat(internal::R::styleable::KeyPosition_sizePercent, k->mAltPercentX) : a.getFloat("sizePercent", k->mAltPercentX);
    k->mPathMotionArc  = ta&&ta->hasValue(internal::R::styleable::KeyPosition_pathMotionArc) ? ta->getInt(internal::R::styleable::KeyPosition_pathMotionArc, k->mPathMotionArc) : a.getInt("pathMotionArc", k->mPathMotionArc);
    k->mPositionType   = ta&&ta->hasValue(internal::R::styleable::KeyPosition_keyPositionType) ? ta->getInt(internal::R::styleable::KeyPosition_keyPositionType, k->mPositionType) : a.getInt("keyPositionType", kPositionType, k->mPositionType);
    return k;
}

// KeyCycle and KeyTimeCycle share the same attribute set (wave params + transform values).
template <typename KeyT>
std::unique_ptr<MotionKey> makeKeyCycle(const AttributeSet& a) {
    auto k = std::make_unique<KeyT>();
    loadCommon(*k, a);
    Context* ctx = a.getContext();
    auto ta = ctx->obtainStyledAttributes(a, internal::R::styleable::KeyCycle);
    k->mWaveShape  = ta&&ta->hasValue(internal::R::styleable::KeyCycle_waveShape) ? ta->getInt(internal::R::styleable::KeyCycle_waveShape, k->mWaveShape) : a.getInt("waveShape", k->mWaveShape);
    k->mWavePeriod = ta&&ta->hasValue(internal::R::styleable::KeyCycle_wavePeriod) ? ta->getFloat(internal::R::styleable::KeyCycle_wavePeriod, k->mWavePeriod) : a.getFloat("wavePeriod", k->mWavePeriod);
    k->mWaveOffset = ta&&ta->hasValue(internal::R::styleable::KeyCycle_waveOffset) ? ta->getFloat(internal::R::styleable::KeyCycle_waveOffset, k->mWaveOffset) : a.getFloat("waveOffset", k->mWaveOffset);
    k->mAlpha       = ta&&ta->hasValue(internal::R::styleable::KeyCycle_alpha) ? ta->getFloat(internal::R::styleable::KeyCycle_alpha, k->mAlpha) : a.getFloat("alpha", k->mAlpha);
    k->mElevation   = ta&&ta->hasValue(internal::R::styleable::KeyCycle_elevation) ? ta->getFloat(internal::R::styleable::KeyCycle_elevation, k->mElevation) : a.getFloat("elevation", k->mElevation);
    k->mRotation    = ta&&ta->hasValue(internal::R::styleable::KeyCycle_rotation) ? ta->getFloat(internal::R::styleable::KeyCycle_rotation, k->mRotation) : a.getFloat("rotation", k->mRotation);
    k->mRotationX   = ta&&ta->hasValue(internal::R::styleable::KeyCycle_rotationX) ? ta->getFloat(internal::R::styleable::KeyCycle_rotationX, k->mRotationX) : a.getFloat("rotationX", k->mRotationX);
    k->mRotationY   = ta&&ta->hasValue(internal::R::styleable::KeyCycle_rotationY) ? ta->getFloat(internal::R::styleable::KeyCycle_rotationY, k->mRotationY) : a.getFloat("rotationY", k->mRotationY);
    k->mScaleX      = ta&&ta->hasValue(internal::R::styleable::KeyCycle_scaleX) ? ta->getFloat(internal::R::styleable::KeyCycle_scaleX, k->mScaleX) : a.getFloat("scaleX", k->mScaleX);
    k->mScaleY      = ta&&ta->hasValue(internal::R::styleable::KeyCycle_scaleY) ? ta->getFloat(internal::R::styleable::KeyCycle_scaleY, k->mScaleY) : a.getFloat("scaleY", k->mScaleY);
    k->mTranslationX = ta&&ta->hasValue(internal::R::styleable::KeyCycle_translationX) ? ta->getFloat(internal::R::styleable::KeyCycle_translationX, k->mTranslationX) : a.getFloat("translationX", k->mTranslationX);
    k->mTranslationY = ta&&ta->hasValue(internal::R::styleable::KeyCycle_translationY) ? ta->getFloat(internal::R::styleable::KeyCycle_translationY, k->mTranslationY) : a.getFloat("translationY", k->mTranslationY);
    k->mTranslationZ = ta&&ta->hasValue(internal::R::styleable::KeyCycle_translationZ) ? ta->getFloat(internal::R::styleable::KeyCycle_translationZ, k->mTranslationZ) : a.getFloat("translationZ", k->mTranslationZ);
    k->mTransitionPathRotate = ta&&ta->hasValue(internal::R::styleable::KeyCycle_transitionPathRotate) ? ta->getFloat(internal::R::styleable::KeyCycle_transitionPathRotate, k->mTransitionPathRotate) : a.getFloat("transitionPathRotate", k->mTransitionPathRotate);
    k->mProgress    = ta&&ta->hasValue(internal::R::styleable::KeyCycle_motionProgress) ? ta->getFloat(internal::R::styleable::KeyCycle_motionProgress, k->mProgress) : a.getFloat("motionProgress", k->mProgress);
    return k;
}

std::unique_ptr<MotionKey> makeKeyTrigger(const AttributeSet& a) {
    auto k = std::make_unique<MotionKeyTrigger>();
    loadCommon(*k, a);
    Context* ctx = a.getContext();
    auto ta = ctx->obtainStyledAttributes(a, internal::R::styleable::KeyTrigger);
    k->mCross         = ta&&ta->hasValue(internal::R::styleable::KeyTrigger_onCross) ? ta->getString(internal::R::styleable::KeyTrigger_onCross) : a.getString("onCross", k->mCross);
    k->mPositiveCross = ta&&ta->hasValue(internal::R::styleable::KeyTrigger_onPositiveCross) ? ta->getString(internal::R::styleable::KeyTrigger_onPositiveCross) : a.getString("onPositiveCross", k->mPositiveCross);
    k->mNegativeCross = ta&&ta->hasValue(internal::R::styleable::KeyTrigger_onNegativeCross) ? ta->getString(internal::R::styleable::KeyTrigger_onNegativeCross) : a.getString("onNegativeCross", k->mNegativeCross);
    k->mTriggerID     = ta ? (int)ta->getResourceId(internal::R::styleable::KeyTrigger_triggerId, k->mTriggerID) : k->mTriggerID;
    k->mTriggerReceiver = ta ? (int)ta->getResourceId(internal::R::styleable::KeyTrigger_triggerReceiver, k->mTriggerReceiver) : k->mTriggerReceiver;
    k->mTriggerSlack  = ta&&ta->hasValue(internal::R::styleable::KeyTrigger_triggerSlack) ? ta->getFloat(internal::R::styleable::KeyTrigger_triggerSlack, k->mTriggerSlack) : a.getFloat("triggerSlack", k->mTriggerSlack);
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
