/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.motion.widget.KeyFrames.
 */
#include <widgetEx/constraintlayout/keyframes.h>

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
    k->mAlpha       = a.getFloat("alpha", k->mAlpha);
    k->mElevation   = a.getFloat("elevation", k->mElevation);
    k->mRotation    = a.getFloat("rotation", k->mRotation);
    k->mRotationX   = a.getFloat("rotationX", k->mRotationX);
    k->mRotationY   = a.getFloat("rotationY", k->mRotationY);
    k->mPivotX      = a.getFloat("transformPivotX", k->mPivotX);
    k->mPivotY      = a.getFloat("transformPivotY", k->mPivotY);
    k->mScaleX      = a.getFloat("scaleX", k->mScaleX);
    k->mScaleY      = a.getFloat("scaleY", k->mScaleY);
    k->mTranslationX = a.getFloat("translationX", k->mTranslationX);
    k->mTranslationY = a.getFloat("translationY", k->mTranslationY);
    k->mTranslationZ = a.getFloat("translationZ", k->mTranslationZ);
    k->mTransitionPathRotate = a.getFloat("transitionPathRotate", k->mTransitionPathRotate);
    k->mProgress    = a.getFloat("motionProgress", k->mProgress);
    k->mCurveFit    = a.getInt("curveFit", k->mCurveFit);
    k->mVisibility  = a.getBoolean("visibility", k->mVisibility != 0) ? 1 : 0;
    return k;
}

std::unique_ptr<MotionKey> makeKeyPosition(const AttributeSet& a) {
    auto k = std::make_unique<MotionKeyPosition>();
    loadCommon(*k, a);
    k->mTransitionEasing = a.getString("transitionEasing", k->mTransitionEasing);
    k->mDrawPath       = a.getInt("drawPath", k->mDrawPath);
    k->mPercentX       = a.getFloat("percentX", k->mPercentX);
    k->mPercentY       = a.getFloat("percentY", k->mPercentY);
    k->mPercentWidth   = a.getFloat("percentWidth", k->mPercentWidth);
    k->mPercentHeight  = a.getFloat("percentHeight", k->mPercentHeight);
    k->mAltPercentX    = a.getFloat("sizePercent", k->mAltPercentX);
    k->mPathMotionArc  = a.getInt("pathMotionArc", k->mPathMotionArc);
    k->mPositionType   = a.getInt("keyPositionType", kPositionType, k->mPositionType);
    return k;
}

// KeyCycle and KeyTimeCycle share the same attribute set (wave params + transform values).
template <typename KeyT>
std::unique_ptr<MotionKey> makeKeyCycle(const AttributeSet& a) {
    auto k = std::make_unique<KeyT>();
    loadCommon(*k, a);
    k->mWaveShape  = a.getInt("waveShape", k->mWaveShape);
    k->mWavePeriod = a.getFloat("wavePeriod", k->mWavePeriod);
    k->mWaveOffset = a.getFloat("waveOffset", k->mWaveOffset);
    k->mAlpha       = a.getFloat("alpha", k->mAlpha);
    k->mElevation   = a.getFloat("elevation", k->mElevation);
    k->mRotation    = a.getFloat("rotation", k->mRotation);
    k->mRotationX   = a.getFloat("rotationX", k->mRotationX);
    k->mRotationY   = a.getFloat("rotationY", k->mRotationY);
    k->mScaleX      = a.getFloat("scaleX", k->mScaleX);
    k->mScaleY      = a.getFloat("scaleY", k->mScaleY);
    k->mTranslationX = a.getFloat("translationX", k->mTranslationX);
    k->mTranslationY = a.getFloat("translationY", k->mTranslationY);
    k->mTranslationZ = a.getFloat("translationZ", k->mTranslationZ);
    k->mTransitionPathRotate = a.getFloat("transitionPathRotate", k->mTransitionPathRotate);
    k->mProgress    = a.getFloat("motionProgress", k->mProgress);
    return k;
}

std::unique_ptr<MotionKey> makeKeyTrigger(const AttributeSet& a) {
    auto k = std::make_unique<MotionKeyTrigger>();
    loadCommon(*k, a);
    k->mCross         = a.getString("onCross", k->mCross);
    k->mPositiveCross = a.getString("onPositiveCross", k->mPositiveCross);
    k->mNegativeCross = a.getString("onNegativeCross", k->mNegativeCross);
    k->mTriggerID     = a.getResourceId("triggerId", k->mTriggerID);
    k->mTriggerReceiver = a.getResourceId("triggerReceiver", k->mTriggerReceiver);
    k->mTriggerSlack  = a.getFloat("triggerSlack", k->mTriggerSlack);
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
