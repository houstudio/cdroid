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
 * Ported to C++ for CDROID from androidx.constraintlayout.helper.widget.MotionEffect.
 */
#include <widgetEx/constraintlayout/motion/motioneffect.h>

#include <cmath>

#include <widgetEx/constraintlayout/motion/motionlayout.h>
#include <widgetEx/constraintlayout/core/motion/motion.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyattributes.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyposition.h>

DECLARE_WIDGET(MotionEffect)

namespace cdroid {

MotionEffect::MotionEffect(Context* ctx, const AttributeSet& attrs)
    : MotionHelper(ctx, attrs) {
    init(attrs);
}

MotionEffect::MotionEffect(int width, int height)
    : MotionHelper(width, height) {
}

void MotionEffect::init(const AttributeSet& attrs) {
    ConstraintHelper::init(attrs);
    mMotionEffectStart = std::max(0, std::min(99, attrs.getInt("motionEffect_start", mMotionEffectStart)));
    mMotionEffectEnd   = std::max(0, std::min(99, attrs.getInt("motionEffect_end", mMotionEffectEnd)));
    mMotionEffectTranslationX = attrs.getDimensionPixelOffset("motionEffect_translationX", mMotionEffectTranslationX);
    mMotionEffectTranslationY = attrs.getDimensionPixelOffset("motionEffect_translationY", mMotionEffectTranslationY);
    mMotionEffectAlpha  = attrs.getFloat("motionEffect_alpha", mMotionEffectAlpha);
    mMotionEffectStrictMove = attrs.getBoolean("motionEffect_strict", mMotionEffectStrictMove);
    mViewTransitionId  = attrs.getResourceId("motionEffect_viewTransition", UNSET);
    mFadeMove = attrs.getInt("motionEffect_move", std::unordered_map<std::string,int>{
        {"auto", (int) AUTO}, {"north", (int) NORTH}, {"south", (int) SOUTH},
        {"east", (int) EAST}, {"west", (int) WEST}
    }, mFadeMove);
    if (mMotionEffectStart == mMotionEffectEnd) {
        if (mMotionEffectStart > 0) mMotionEffectStart--;
        else mMotionEffectEnd++;
    }
}

int MotionEffect::computeFadeDirection(const std::vector<std::pair<float, float>>& deltas) {
    int direction[4] = {0, 0, 0, 0};
    for (const auto& d : deltas) {
        float x = d.first;
        float y = d.second;
        if (y < 0) direction[SOUTH]++;
        if (y > 0) direction[NORTH]++;
        if (x > 0) direction[WEST]++;
        if (x < 0) direction[EAST]++;
    }
    int max = direction[0];
    int moveDirection = 0;
    for (int i = 1; i < 4; i++) {
        if (max < direction[i]) {
            max = direction[i];
            moveDirection = i;
        }
    }
    return moveDirection;
}

void MotionEffect::onPreSetup(MotionLayout* motionLayout, MotionMap& motions) {
    mOwnedKeys.clear();
    if (mIds.empty()) {
        return;
    }

    // Reusable keyframes (owned by this MotionEffect; Motion::addKey borrows the raw pointers).
    auto newAlpha = [&](int frame) -> MotionKeyAttributes* {
        auto k = std::make_unique<MotionKeyAttributes>();
        k->mAlpha = mMotionEffectAlpha;
        k->setFramePosition(frame);
        MotionKeyAttributes* raw = k.get();
        mOwnedKeys.push_back(std::move(k));
        return raw;
    };
    MotionKeyAttributes* alpha1 = newAlpha(mMotionEffectStart);
    MotionKeyAttributes* alpha2 = newAlpha(mMotionEffectEnd);

    auto newStick = [&](int frame, float px, float py) -> MotionKeyPosition* {
        auto k = std::make_unique<MotionKeyPosition>();
        k->mPositionType = MotionKeyPosition::TYPE_CARTESIAN;
        k->mPercentX = px;
        k->mPercentY = py;
        k->setFramePosition(frame);
        MotionKeyPosition* raw = k.get();
        mOwnedKeys.push_back(std::move(k));
        return raw;
    };
    // "Stick" the view's position at the start (percent 0) and end (percent 1) of the fade window so
    // it doesn't interpolate across it while fading.
    MotionKeyPosition* stick1 = newStick(mMotionEffectStart, 0, 0);
    MotionKeyPosition* stick2 = newStick(mMotionEffectEnd, 1, 1);

    MotionKeyAttributes* translationX1 = nullptr;
    MotionKeyAttributes* translationX2 = nullptr;
    if (mMotionEffectTranslationX > 0) {
        translationX1 = newAlpha(mMotionEffectEnd);
        translationX1->mTranslationX = mMotionEffectTranslationX;
        translationX2 = newAlpha(mMotionEffectEnd - 1);
        translationX2->mTranslationX = 0;
    }
    MotionKeyAttributes* translationY1 = nullptr;
    MotionKeyAttributes* translationY2 = nullptr;
    if (mMotionEffectTranslationY > 0) {
        translationY1 = newAlpha(mMotionEffectEnd);
        translationY1->mTranslationY = mMotionEffectTranslationY;
        translationY2 = newAlpha(mMotionEffectEnd - 1);
        translationY2->mTranslationY = 0;
    }

    int moveDirection = mFadeMove;
    if (mFadeMove == AUTO) {
        std::vector<std::pair<float, float>> deltas;
        for (int id : mIds) {
            auto it = motions.find(id);
            if (it == motions.end()) continue;
            Motion* mc = it->second;
            deltas.push_back({mc->getEndMotionPath().mX - mc->getStartMotionPath().mX,
                              mc->getEndMotionPath().mY - mc->getStartMotionPath().mY});
        }
        moveDirection = computeFadeDirection(deltas);
    }

    for (int id : mIds) {
        auto it = motions.find(id);
        if (it == motions.end()) continue;
        Motion* mc = it->second;
        float x = mc->getEndMotionPath().mX - mc->getStartMotionPath().mX;
        float y = mc->getEndMotionPath().mY - mc->getStartMotionPath().mY;
        bool apply = true;

        // Views moving in the dominant direction are faded; strict mode also fades diagonal movers.
        if (moveDirection == NORTH) {
            if (y > 0 && (!mMotionEffectStrictMove || x == 0)) apply = false;
        } else if (moveDirection == SOUTH) {
            if (y < 0 && (!mMotionEffectStrictMove || x == 0)) apply = false;
        } else if (moveDirection == EAST) {
            if (x < 0 && (!mMotionEffectStrictMove || y == 0)) apply = false;
        } else if (moveDirection == WEST) {
            if (x > 0 && (!mMotionEffectStrictMove || y == 0)) apply = false;
        }

        if (apply) {
            if (mViewTransitionId == UNSET) {
                mc->addKey(alpha1);
                mc->addKey(alpha2);
                mc->addKey(stick1);
                mc->addKey(stick2);
                if (translationX1 != nullptr) {
                    mc->addKey(translationX1);
                    mc->addKey(translationX2);
                }
                if (translationY1 != nullptr) {
                    mc->addKey(translationY1);
                    mc->addKey(translationY2);
                }
            } else if (motionLayout != nullptr) {
                motionLayout->applyViewTransition(mViewTransitionId, mc);
            }
        }
    }
}

} // namespace cdroid
