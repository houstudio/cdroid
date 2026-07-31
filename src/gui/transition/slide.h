/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.Slide.
 *********************************************************************************/
#ifndef __CDROID_TRANSITION_SLIDE_H__
#define __CDROID_TRANSITION_SLIDE_H__

#include <functional>

#include <transition/visibility.h>
#include <view/gravity.h>

namespace cdroid {

class Context;
class AttributeSet;

/**
 * Tracks visibility changes and moves views in/out from one edge of the scene.
 * Ported from android-36 android.transition.Slide.
 */
class Slide: public Visibility {
  public:
    Slide();
    explicit Slide(int slideEdge);
    Slide(Context* context, AttributeSet* attrs);

    void setSlideEdge(int slideEdge);
    int getSlideEdge() const {
        return mSlideEdge;
    }

    Animator* onAppear(ViewGroup* sceneRoot, View* view,
                       TransitionValues* startValues, TransitionValues* endValues) override;
    Animator* onDisappear(ViewGroup* sceneRoot, View* view,
                          TransitionValues* startValues, TransitionValues* endValues) override;

    // TransitionManager clones the transition before running it; without this override the clone
    // is a non-Slide base and the slide never plays (mirrors Fade::clone).
    Slide* clone() const override {
        Slide* c = new Slide(*this);
        copyCloneFields(c);
        return c;
    }

    void setSlideFraction(float slideFraction) {
        mSlideFraction = slideFraction;
    }

  protected:
    void captureStartValues(TransitionValues& transitionValues) override;
    void captureEndValues(TransitionValues& transitionValues) override;

  private:
    // android: CalculateSlide interface + Horizontal/Vertical abstract bases + 6 impls.
    // C++: a struct of two callables (getGoneX/getGoneY); the 6 instances are static.
    struct CalculateSlide {
        std::function<float(ViewGroup*, View*, float)> getGoneX;
        std::function<float(ViewGroup*, View*, float)> getGoneY;
    };
    static const CalculateSlide sCalculateLeft;
    static const CalculateSlide sCalculateStart;
    static const CalculateSlide sCalculateTop;
    static const CalculateSlide sCalculateRight;
    static const CalculateSlide sCalculateEnd;
    static const CalculateSlide sCalculateBottom;

    void captureValues(TransitionValues& transitionValues);

    static constexpr const char* PROPNAME_SCREEN_POSITION = "android:slide:screenPosition";

    const CalculateSlide* mSlideCalculator;
    int mSlideEdge = Gravity::BOTTOM;
    float mSlideFraction = 1;
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_SLIDE_H__
