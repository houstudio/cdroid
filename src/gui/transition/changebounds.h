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
#ifndef __CDROID_TRANSITION_CHANGEBOUNDS_H__
#define __CDROID_TRANSITION_CHANGEBOUNDS_H__

#include <string>
#include <vector>

#include <transition/transition.h>

namespace cdroid{

class Context;
class AttributeSet;

/**
 * This transition captures the layout bounds of target views before and after the scene
 * change and animates those changes. Ported from android-36 android.transition.ChangeBounds.
 *
 * Reparent (setReparent, deprecated) relies on rendering the view into a Bitmap overlay;
 * CDROID's cairo 2D substrate has no DisplayList snapshot, so the reparent path is stubbed
 * (mReparent defaults false; the common same-parent bounds animation works fully).
 *
 * Rect semantics: CDROID CRect<int> is {left, top, width, height}; bounds are stored via
 * Rect::MakeLTRB and read with .left/.top fields and .right()/.bottom() methods.
 */
class ChangeBounds: public Transition{
public:
    ChangeBounds();
    ChangeBounds(Context* context, AttributeSet* attrs);

    std::vector<std::string> getTransitionProperties() override;
    void setResizeClip(bool resizeClip){ mResizeClip = resizeClip; }
    bool getResizeClip() const{ return mResizeClip; }
    /** @deprecated use ChangeTransform. CDROID stubs the reparent bitmap-overlay path. */
    void setReparent(bool reparent){ mReparent = reparent; }

    void captureStartValues(TransitionValues& transitionValues) override;
    void captureEndValues(TransitionValues& transitionValues) override;
    Animator* createAnimator(ViewGroup* sceneRoot,
            TransitionValues* startValues, TransitionValues* endValues) override;

    ChangeBounds* clone() const override{ ChangeBounds* c = new ChangeBounds(*this); copyCloneFields(c); return c; }

private:
    void captureValues(TransitionValues& values);
    bool parentMatches(ViewGroup* startParent, ViewGroup* endParent);

    static constexpr const char* PROPNAME_BOUNDS  = "android:changeBounds:bounds";
    static constexpr const char* PROPNAME_CLIP    = "android:changeBounds:clip";
    static constexpr const char* PROPNAME_PARENT  = "android:changeBounds:parent";
    static constexpr const char* PROPNAME_WINDOW_X = "android:changeBounds:windowX";
    static constexpr const char* PROPNAME_WINDOW_Y = "android:changeBounds:windowY";
    static const std::vector<std::string> sTransitionProperties;

    bool mResizeClip = false;
    bool mReparent = false;
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_CHANGEBOUNDS_H__
