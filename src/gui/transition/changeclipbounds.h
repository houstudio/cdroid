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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *********************************************************************************/
#ifndef __CDROID_TRANSITION_CHANGECLIPBOUNDS_H__
#define __CDROID_TRANSITION_CHANGECLIPBOUNDS_H__

#include <string>
#include <vector>

#include <transition/transition.h>

namespace cdroid {

class Context;
class AttributeSet;

/**
 * ChangeClipBounds captures View#getClipBounds() before and after the scene change and
 * animates those changes. Ported from android-36 android.transition.ChangeClipBounds.
 *
 * Null-clip note: android stores a null Rect in the values map (key present, value null).
 * CDROID's nonstd::any has no null, so a default-constructed (empty) any represents null;
 * has_value() distinguishes clip-present from clip-absent.
 */
class ChangeClipBounds: public Transition {
  public:
    ChangeClipBounds() = default;
    ChangeClipBounds(Context* context, AttributeSet* attrs): Transition(context, attrs) {}

    std::vector<std::string> getTransitionProperties() override;
    void captureStartValues(TransitionValues& transitionValues) override;
    void captureEndValues(TransitionValues& transitionValues) override;
    Animator* createAnimator(ViewGroup* sceneRoot,
                             TransitionValues* startValues, TransitionValues* endValues) override;

    ChangeClipBounds* clone() const override {
        ChangeClipBounds* c = new ChangeClipBounds(*this);
        copyCloneFields(c);
        return c;
    }

  private:
    void captureValues(TransitionValues& values);

    static constexpr const char* PROPNAME_CLIP   = "android:clipBounds:clip";
    static constexpr const char* PROPNAME_BOUNDS = "android:clipBounds:bounds";
    static const std::vector<std::string> sTransitionProperties;
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_CHANGECLIPBOUNDS_H__
