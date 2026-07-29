/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.helper.widget.MotionEffect.
 *
 * A MotionLayout decorator that auto-inserts fade + position-stick keyframes (and optional slide)
 * into the Motion controllers of referenced views moving in the dominant screen direction. It runs
 * once per transition setup (onPreSetup), mutating the keyframe set; MotionLayout's interpolation
 * does the rest.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_MOTION_EFFECT_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_MOTION_EFFECT_H

#include <memory>
#include <vector>

#include <widgetEx/constraintlayout/motion/motionhelper.h>
#include <widgetEx/constraintlayout/core/motion/motionkey.h>

namespace cdroid {

class MotionEffect : public MotionHelper {
  public:
    static constexpr int UNSET = -1;
    // Screen direction of a view's overall motion (finalX-startX, finalY-startY).
    static constexpr int AUTO = -1;  // pick the dominant direction automatically
    static constexpr int NORTH = 0;
    static constexpr int SOUTH = 1;
    static constexpr int EAST  = 2;
    static constexpr int WEST  = 3;

    MotionEffect(Context* ctx, const AttributeSet& attrs);
    explicit MotionEffect(int width, int height);

    bool isDecorator() const override { return true; }
    void onPreSetup(MotionLayout* motionLayout, MotionMap& motions) override;

    // Vote-tally the per-view motion deltas (Δx,Δy) into the opposite-of-dominant screen direction
    // (NORTH/SOUTH/EAST/WEST) the fade will apply to. Factored out for unit testing.
    static int computeFadeDirection(const std::vector<std::pair<float, float>>& deltas);

  protected:
    void init(const AttributeSet& attrs) override;

  private:
    float mMotionEffectAlpha = 0.1f;
    int   mMotionEffectStart = 49;
    int   mMotionEffectEnd = 50;
    int   mMotionEffectTranslationX = 0;
    int   mMotionEffectTranslationY = 0;
    bool  mMotionEffectStrictMove = true;
    int   mViewTransitionId = UNSET;
    int   mFadeMove = AUTO;

    // Owns the keyframes created per onPreSetup (Motion::addKey borrows raw pointers; the controllers
    // are rebuilt before each onPreSetup, so clearing here on the next build frees the previous set).
    std::vector<std::unique_ptr<MotionKey>> mOwnedKeys;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_MOTION_EFFECT_H
