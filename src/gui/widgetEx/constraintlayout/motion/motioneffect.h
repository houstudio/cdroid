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
