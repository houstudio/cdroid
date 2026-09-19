#ifndef __DESKCLOCK_ANIMATORUTILS_H__
#define __DESKCLOCK_ANIMATORUTILS_H__
/*********************************************************************************
 * Port of com.android.deskclock.AnimatorUtils — interpolators, animated view
 * properties (background/drawable alpha, tint, bounds) and animator factories
 * shared by the tab fragments.
 *********************************************************************************/
#include <string>
#include <vector>

#include <animation/animator.h>
#include <animation/interpolators.h>
#include <animation/property.h>
#include <animation/valueanimator.h>

namespace cdroid {

class View;
class ImageView;

namespace deskclock {
namespace AnimatorUtils {

// Interpolator { x -> 0.5f + 4.0f * (x - 0.5f)^3 }
const Interpolator* DECELERATE_ACCELERATE_INTERPOLATOR();

// FastOutSlowInInterpolator() = PathInterpolator(0.4f, 0f, 0.2f, 1f)
const Interpolator* INTERPOLATOR_FAST_OUT_SLOW_IN();

// Property<View, Int> "background.alpha"
const Property* BACKGROUND_ALPHA();

// Sets the alpha of the top layer's drawable (of the background) only, if the
// background is a layer drawable, so touch-feedback layers are unaffected.
void setBackgroundAlpha(View& view, int value);

// Property<ImageView, Int> "drawable.alpha"
const Property* DRAWABLE_ALPHA();

// Property<ImageView, Int> "drawable.tint"
const Property* DRAWABLE_TINT();

// Sets the fraction of the animator directly, unaffected by animator scale/time.
void setAnimatedFraction(ValueAnimator& animator, float fraction);

// Reverses each animator from its current fraction (or leaves stopped ones alone).
void reverse(const std::vector<ValueAnimator*>& animators);

void cancel(const std::vector<Animator*>& animators);

// ObjectAnimator ofPropertyValuesHolder(SCALE_X/SCALE_Y, values...)
ValueAnimator* getScaleAnimator(View* view, const std::vector<float>& values);

// ObjectAnimator ofFloat(view, ALPHA, values...)
ValueAnimator* getAlphaAnimator(View* view, const std::vector<float>& values);

// Property<View, Int> left/top/right/bottom (for bounds morph animations)
const Property* VIEW_LEFT();
const Property* VIEW_TOP();
const Property* VIEW_RIGHT();
const Property* VIEW_BOTTOM();

// Morphs the target between the content bounds of `from` and `to` (background
// padding insets are subtracted so the *content* bounds match).
Animator* getBoundsAnimator(View& target, View& from, View& to);

// Returns an animator that animates the bounds of a single view.
Animator* getBoundsAnimator(View& view, int fromLeft, int fromTop, int fromRight, int fromBottom,
                            int toLeft, int toTop, int toRight, int toBottom);

// Starts the view's drawable animation if it is an Animatable.
void startDrawableAnimation(ImageView& view);

} // namespace AnimatorUtils
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_ANIMATORUTILS_H__
