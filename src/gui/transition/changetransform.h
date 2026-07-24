/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.ChangeTransform.
 *
 * Substrate limits: android drives the combined transform via setAnimationMatrix + uses a
 * GhostView to handle parent changes (reparent). CDROID's cairo 2D has neither (View renders
 * transforms via property getters; setAnimationMatrix is a no-op stub). So this port captures
 * the transforms/matrix/parent faithfully and animates the matrix fraction-driven through
 * setAnimationMatrix (which does not visually apply mid-animation on cairo), restoring the
 * final transform via the property setters on end. The GhostView reparent-overlay path is
 * stubbed. Net: capture/restore + lifecycle correct; mid-animation transform + reparent are
 * limited by the substrate (3D/animation-matrix not available in cairo).
 *********************************************************************************/
#ifndef __CDROID_TRANSITION_CHANGETRANSFORM_H__
#define __CDROID_TRANSITION_CHANGETRANSFORM_H__

#include <cairomm/matrix.h>

#include <transition/transition.h>

namespace cdroid{

class Context;
class AttributeSet;

/**
 * Captures scale and rotation (the transform matrix) for Views before and after the scene
 * change and animates those changes. Ported from android-36 android.transition.ChangeTransform.
 */
class ChangeTransform: public Transition{
public:
    ChangeTransform() = default;
    ChangeTransform(Context* context, AttributeSet* attrs): Transition(context, attrs){}

    std::vector<std::string> getTransitionProperties() override;
    bool getReparentWithOverlay() const{ return mUseOverlay; }
    void setReparentWithOverlay(bool v){ mUseOverlay = v; }
    bool getReparent() const{ return mReparent; }
    void setReparent(bool v){ mReparent = v; }

    void captureStartValues(TransitionValues& transitionValues) override;
    void captureEndValues(TransitionValues& transitionValues) override;
    Animator* createAnimator(ViewGroup* sceneRoot,
            TransitionValues* startValues, TransitionValues* endValues) override;

    ChangeTransform* clone() const override{ ChangeTransform* c = new ChangeTransform(*this); copyCloneFields(c); return c; }

private:
    void captureValues(TransitionValues& transitionValues);

    // android: private static nested Transforms — the 8 view transform properties.
    struct Transforms{
        float translationX, translationY, translationZ;
        float scaleX, scaleY;
        float rotationX, rotationY, rotationZ;
        Transforms() = default;
        explicit Transforms(View* view);
        void restore(View* view) const;
    };
    static void setIdentityTransforms(View* view);
    static void setTransforms(View* view, float translationX, float translationY, float translationZ,
            float scaleX, float scaleY, float rotationX, float rotationY, float rotationZ);

    static constexpr const char* PROPNAME_MATRIX    = "android:changeTransform:matrix";
    static constexpr const char* PROPNAME_TRANSFORMS = "android:changeTransform:transforms";
    static constexpr const char* PROPNAME_PARENT    = "android:changeTransform:parent";
    static constexpr const char* PROPNAME_PARENT_MATRIX = "android:changeTransform:parentMatrix";
    static const std::vector<std::string> sTransitionProperties;

    bool mUseOverlay = true;
    bool mReparent = true;
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_CHANGETRANSFORM_H__
