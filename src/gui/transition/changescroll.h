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
#ifndef __CDROID_TRANSITION_CHANGESCROLL_H__
#define __CDROID_TRANSITION_CHANGESCROLL_H__

#include <string>
#include <vector>

#include <transition/transition.h>

namespace cdroid {

class Context;
class AttributeSet;

/**
 * This transition captures the scroll properties of targets before and after the
 * scene change and animates any changes. Ported from android-36 android.transition.ChangeScroll.
 */
class ChangeScroll: public Transition {
  public:
    ChangeScroll() = default;
    ChangeScroll(Context* context, AttributeSet* attrs);

    void captureStartValues(TransitionValues& transitionValues) override;
    void captureEndValues(TransitionValues& transitionValues) override;
    std::vector<std::string> getTransitionProperties() override;
    Animator* createAnimator(ViewGroup* sceneRoot,
                             TransitionValues* startValues, TransitionValues* endValues) override;

    ChangeScroll* clone() const override {
        ChangeScroll* c = new ChangeScroll(*this);
        copyCloneFields(c);
        return c;
    }

  private:
    void captureValues(TransitionValues& transitionValues);

    static constexpr const char* PROPNAME_SCROLL_X = "android:changeScroll:x";
    static constexpr const char* PROPNAME_SCROLL_Y = "android:changeScroll:y";
    static const std::vector<std::string> PROPERTIES;
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_CHANGESCROLL_H__
