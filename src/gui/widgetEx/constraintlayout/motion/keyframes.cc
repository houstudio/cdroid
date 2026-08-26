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
#include <widget/internal_R.h>
#include <core/context.h>
#include <core/typedarray.h>
#include <widgetEx/constraintlayout/motion/keyframes.h>

#include <widgetEx/widgetex_styleable.h>
#include <core/xmlpullparser.h>

#include <widgetEx/constraintlayout/core/motion/motionkeyattributes.h>
#include <widgetEx/constraintlayout/core/motion/motionkeycycle.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyposition.h>
#include <widgetEx/constraintlayout/core/motion/motionkeytimecycle.h>
#include <widgetEx/constraintlayout/core/motion/motionkeytrigger.h>

namespace cdroid {
using namespace cdroid::internal;

namespace {

// motionTarget (view id) + framePosition are common to every keyframe but sit at
// different indices per styleable, so each make* reads them from its own TypedArray.

std::unique_ptr<MotionKey> makeKeyAttribute(Context* ctx, const AttributeSet* a) {
    auto k = std::make_unique<MotionKeyAttributes>();
    auto ta = ctx->obtainStyledAttributes(a, R::styleable::KeyAttribute);
    if (ta) {
        namespace KA = R::styleable;
        k->mViewId        = (int)ta->getResourceId(KA::KeyAttribute_motionTarget, k->mViewId);
        k->mFramePosition = ta->getInt(KA::KeyAttribute_framePosition, k->mFramePosition);
        k->mAlpha       = ta->getFloat(KA::KeyAttribute_alpha, k->mAlpha);
        k->mElevation   = ta->getFloat(KA::KeyAttribute_elevation, k->mElevation);
        k->mRotation    = ta->getFloat(KA::KeyAttribute_rotation, k->mRotation);
        k->mRotationX   = ta->getFloat(KA::KeyAttribute_rotationX, k->mRotationX);
        k->mRotationY   = ta->getFloat(KA::KeyAttribute_rotationY, k->mRotationY);
        k->mPivotX      = ta->getFloat(KA::KeyAttribute_transformPivotX, k->mPivotX);
        k->mPivotY      = ta->getFloat(KA::KeyAttribute_transformPivotY, k->mPivotY);
        k->mScaleX      = ta->getFloat(KA::KeyAttribute_scaleX, k->mScaleX);
        k->mScaleY      = ta->getFloat(KA::KeyAttribute_scaleY, k->mScaleY);
        k->mTranslationX = ta->getFloat(KA::KeyAttribute_translationX, k->mTranslationX);
        k->mTranslationY = ta->getFloat(KA::KeyAttribute_translationY, k->mTranslationY);
        k->mTranslationZ = ta->getFloat(KA::KeyAttribute_translationZ, k->mTranslationZ);
        k->mTransitionPathRotate = ta->getFloat(KA::KeyAttribute_transitionPathRotate, k->mTransitionPathRotate);
        k->mProgress    = ta->getFloat(KA::KeyAttribute_motionProgress, k->mProgress);
        k->mCurveFit    = ta->getInt(KA::KeyAttribute_curveFit, k->mCurveFit);
    }
    return k;
}

std::unique_ptr<MotionKey> makeKeyPosition(Context* ctx, const AttributeSet* a) {
    auto k = std::make_unique<MotionKeyPosition>();
    auto ta = ctx->obtainStyledAttributes(a, R::styleable::KeyPosition);
    if (ta) {
        namespace KP = R::styleable;
        k->mViewId        = (int)ta->getResourceId(KP::KeyPosition_motionTarget, k->mViewId);
        k->mFramePosition = ta->getInt(KP::KeyPosition_framePosition, k->mFramePosition);
        k->mTransitionEasing = ta->getString(KP::KeyPosition_transitionEasing);
        k->mDrawPath       = ta->getInt(KP::KeyPosition_drawPath, k->mDrawPath);
        k->mPercentX       = ta->getFloat(KP::KeyPosition_percentX, k->mPercentX);
        k->mPercentY       = ta->getFloat(KP::KeyPosition_percentY, k->mPercentY);
        k->mPercentWidth   = ta->getFloat(KP::KeyPosition_percentWidth, k->mPercentWidth);
        k->mPercentHeight  = ta->getFloat(KP::KeyPosition_percentHeight, k->mPercentHeight);
        k->mAltPercentX    = ta->getFloat(KP::KeyPosition_sizePercent, k->mAltPercentX);
        k->mPathMotionArc  = ta->getInt(KP::KeyPosition_pathMotionArc, k->mPathMotionArc);
        k->mPositionType   = ta->getInt(KP::KeyPosition_keyPositionType, k->mPositionType);
    }
    return k;
}

// KeyCycle and KeyTimeCycle share the same attribute set (wave params + transform values).
template <typename KeyT>
std::unique_ptr<MotionKey> makeKeyCycle(Context* ctx, const AttributeSet* a) {
    auto k = std::make_unique<KeyT>();
    auto ta = ctx->obtainStyledAttributes(a, R::styleable::KeyCycle);
    if (ta) {
        namespace KC = R::styleable;
        k->mViewId        = (int)ta->getResourceId(KC::KeyCycle_motionTarget, k->mViewId);
        k->mFramePosition = ta->getInt(KC::KeyCycle_framePosition, k->mFramePosition);
        k->mWaveShape  = ta->getInt(KC::KeyCycle_waveShape, k->mWaveShape);
        k->mWavePeriod = ta->getFloat(KC::KeyCycle_wavePeriod, k->mWavePeriod);
        k->mWaveOffset = ta->getFloat(KC::KeyCycle_waveOffset, k->mWaveOffset);
        k->mAlpha       = ta->getFloat(KC::KeyCycle_alpha, k->mAlpha);
        k->mElevation   = ta->getFloat(KC::KeyCycle_elevation, k->mElevation);
        k->mRotation    = ta->getFloat(KC::KeyCycle_rotation, k->mRotation);
        k->mRotationX   = ta->getFloat(KC::KeyCycle_rotationX, k->mRotationX);
        k->mRotationY   = ta->getFloat(KC::KeyCycle_rotationY, k->mRotationY);
        k->mScaleX      = ta->getFloat(KC::KeyCycle_scaleX, k->mScaleX);
        k->mScaleY      = ta->getFloat(KC::KeyCycle_scaleY, k->mScaleY);
        k->mTranslationX = ta->getFloat(KC::KeyCycle_translationX, k->mTranslationX);
        k->mTranslationY = ta->getFloat(KC::KeyCycle_translationY, k->mTranslationY);
        k->mTranslationZ = ta->getFloat(KC::KeyCycle_translationZ, k->mTranslationZ);
        k->mTransitionPathRotate = ta->getFloat(KC::KeyCycle_transitionPathRotate, k->mTransitionPathRotate);
        k->mProgress    = ta->getFloat(KC::KeyCycle_motionProgress, k->mProgress);
    }
    return k;
}

std::unique_ptr<MotionKey> makeKeyTrigger(Context* ctx,const AttributeSet* a) {
    auto k = std::make_unique<MotionKeyTrigger>();
    auto ta = ctx->obtainStyledAttributes(a, R::styleable::KeyTrigger);
    if (ta) {
        namespace KT = R::styleable;
        k->mViewId        = (int)ta->getResourceId(KT::KeyTrigger_motionTarget, k->mViewId);
        k->mFramePosition = ta->getInt(KT::KeyTrigger_framePosition, k->mFramePosition);
        k->mCross         = ta->getString(KT::KeyTrigger_onCross);
        k->mPositiveCross = ta->getString(KT::KeyTrigger_onPositiveCross);
        k->mNegativeCross = ta->getString(KT::KeyTrigger_onNegativeCross);
        k->mTriggerID       = (int)ta->getResourceId(KT::KeyTrigger_triggerId, k->mTriggerID);
        k->mTriggerReceiver = (int)ta->getResourceId(KT::KeyTrigger_triggerReceiver, k->mTriggerReceiver);
        k->mTriggerSlack  = ta->getFloat(KT::KeyTrigger_triggerSlack, k->mTriggerSlack);
    }
    return k;
}
} // namespace

KeyFrames::KeyFrames(Context* ctx, XmlPullParser& parser) {
    // `parser` is at the <KeyFrameSet> START_TAG. Walk children until the matching END_TAG.
    while (parser.getEventType() != XmlPullParser::END_DOCUMENT &&
            parser.getEventType() != XmlPullParser::BAD_DOCUMENT) {
        const int eventType = parser.getEventType();
        if (eventType == XmlPullParser::START_TAG) {
            const std::string tag = parser.getName();
            std::unique_ptr<MotionKey> key;
            if (tag == "KeyAttribute")      key = makeKeyAttribute(ctx, &parser);
            else if (tag == "KeyPosition")  key = makeKeyPosition(ctx, &parser);
            else if (tag == "KeyCycle")     key = makeKeyCycle<MotionKeyCycle>(ctx, &parser);
            else if (tag == "KeyTimeCycle") key = makeKeyCycle<MotionKeyTimeCycle>(ctx, &parser);
            else if (tag == "KeyTrigger")   key = makeKeyTrigger(ctx, &parser);
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
