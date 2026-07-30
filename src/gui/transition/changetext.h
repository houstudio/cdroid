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
#ifndef __CDROID_TRANSITION_CHANGETEXT_H__
#define __CDROID_TRANSITION_CHANGETEXT_H__

#include <string>
#include <vector>

#include <transition/transition.h>

namespace cdroid {

class Context;
class AttributeSet;
class EditText;

/**
 * This transition tracks changes to the text in TextView targets. If the text changes
 * between the start and end scenes, the transition keeps the starting text until the
 * transition ends, then sets the end text. Ported from android-36 android.transition.ChangeText.
 *
 * @hide in android.
 *
 * Deviation: android stores the live CharSequence reference in TransitionValues (GC-safe).
 * CDROID's TextView::getText() returns a reference to the live text object, which is
 * replaced by the end text before createAnimator runs — so storing it would dangle, and
 * setText(CharSequence*) transfers ownership. CDROID therefore snapshots the text to a
 * UTF-8 std::string at capture time and restores via setText(string). Spans are not
 * preserved across the transition (acceptable for a text-swap animation).
 */
class ChangeText: public Transition {
  public:
    static constexpr int CHANGE_BEHAVIOR_KEEP  = 0;
    static constexpr int CHANGE_BEHAVIOR_OUT   = 1;
    static constexpr int CHANGE_BEHAVIOR_IN    = 2;
    static constexpr int CHANGE_BEHAVIOR_OUT_IN = 3;

    ChangeText() = default;
    ChangeText(Context* context, AttributeSet* attrs): Transition(context, attrs) {}

    ChangeText& setChangeBehavior(int changeBehavior);
    int getChangeBehavior() const {
        return mChangeBehavior;
    }
    std::vector<std::string> getTransitionProperties() override;
    void captureStartValues(TransitionValues& transitionValues) override;
    void captureEndValues(TransitionValues& transitionValues) override;
    Animator* createAnimator(ViewGroup* sceneRoot,
                             TransitionValues* startValues, TransitionValues* endValues) override;

    ChangeText* clone() const override {
        ChangeText* c = new ChangeText(*this);
        copyCloneFields(c);
        return c;
    }

  private:
    void captureValues(TransitionValues& transitionValues);

    int mChangeBehavior = CHANGE_BEHAVIOR_KEEP;

    static constexpr const char* PROPNAME_TEXT = "android:textchange:text";
    static constexpr const char* PROPNAME_TEXT_SELECTION_START = "android:textchange:textSelectionStart";
    static constexpr const char* PROPNAME_TEXT_SELECTION_END   = "android:textchange:textSelectionEnd";
    static constexpr const char* PROPNAME_TEXT_COLOR           = "android:textchange:textColor";
    static const std::vector<std::string> sTransitionProperties;
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_CHANGETEXT_H__
