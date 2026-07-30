/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.ChangeImageTransform.
 *
 * Matrix note: android uses android.graphics.Matrix + ObjectAnimator.ofObject(property,
 * MatrixEvaluator). CDROID keeps Cairo::Matrix OUT of the AnimateValue variant (plan), so
 * the matrix is animated fraction-driven: a ValueAnimator 0→1 whose update listener lerps
 * the two Cairo::Matrix 6-field affine matrices and calls ImageView::animateTransform.
 * Behavior is equivalent.
 *********************************************************************************/
#ifndef __CDROID_TRANSITION_CHANGEIMAGETRANSFORM_H__
#define __CDROID_TRANSITION_CHANGEIMAGETRANSFORM_H__

#include <transition/transition.h>

namespace cdroid {

class Context;
class AttributeSet;

/**
 * Captures an ImageView's matrix before and after the scene change and animates it.
 * Ported from android-36 android.transition.ChangeImageTransform.
 */
class ChangeImageTransform: public Transition {
  public:
    ChangeImageTransform() = default;
    ChangeImageTransform(Context* context, AttributeSet* attrs): Transition(context, attrs) {}

    std::vector<std::string> getTransitionProperties() override;
    void captureStartValues(TransitionValues& transitionValues) override;
    void captureEndValues(TransitionValues& transitionValues) override;
    Animator* createAnimator(ViewGroup* sceneRoot,
                             TransitionValues* startValues, TransitionValues* endValues) override;

    ChangeImageTransform* clone() const override {
        ChangeImageTransform* c = new ChangeImageTransform(*this);
        copyCloneFields(c);
        return c;
    }

  private:
    void captureValues(TransitionValues& transitionValues);
    static Animator* createNullAnimator(View* imageView);
    static Animator* createMatrixAnimator(View* imageView,
                                          const Cairo::Matrix& startMatrix, const Cairo::Matrix& endMatrix);

    static constexpr const char* PROPNAME_MATRIX = "android:changeImageTransform:matrix";
    static constexpr const char* PROPNAME_BOUNDS = "android:changeImageTransform:bounds";
    static const std::vector<std::string> sTransitionProperties;
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_CHANGEIMAGETRANSFORM_H__
