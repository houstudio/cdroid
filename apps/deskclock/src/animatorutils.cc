#include <animatorutils.h>

#include <animation/objectanimator.h>
#include <animation/propertyvaluesholder.h>
#include <drawable/drawable.h>
#include <drawable/layerdrawable.h>
#include <core/rect.h>
#include <view/view.h>
#include <widget/imageview.h>

namespace cdroid {
namespace deskclock {
namespace AnimatorUtils {

namespace {

/** Interpolator { x -> 0.5f + 4.0f * (x - 0.5f)^3 } */
class DecelerateAccelerateInterpolator : public Interpolator {
public:
    float getInterpolation(float x) const override {
        const float d = x - 0.5f;
        return 0.5f + 4.0f * d * d * d;
    }
};

/** Property<View, Int> "background.alpha" */
class BackgroundAlphaProperty : public Property {
public:
    BackgroundAlphaProperty() : Property("background.alpha", INT_TYPE) {}

    AnimateValue get(void* t) const override {
        View& view = *static_cast<View*>(t);
        Drawable* background = view.getBackground();
        if (dynamic_cast<LayerDrawable*>(background) && ((LayerDrawable*)background)->getNumberOfLayers() > 0) {
            background = ((LayerDrawable*)background)->getDrawable(0);
        }
        return background ? AnimateValue(background->getAlpha()) : AnimateValue(0);
    }

    void set(void* o, const AnimateValue& value) const override {
        setBackgroundAlpha(*static_cast<View*>(o), GET_VARIANT(value, int));
    }
};

/** Property<ImageView, Int> "drawable.alpha" */
class DrawableAlphaProperty : public Property {
public:
    DrawableAlphaProperty() : Property("drawable.alpha", INT_TYPE) {}

    AnimateValue get(void* t) const override {
        ImageView& view = *static_cast<ImageView*>(t);
        return view.getDrawable() ? AnimateValue(view.getDrawable()->getAlpha()) : AnimateValue(0);
    }

    void set(void* o, const AnimateValue& value) const override {
        ImageView& view = *static_cast<ImageView*>(o);
        if (view.getDrawable()) view.getDrawable()->setAlpha(GET_VARIANT(value, int));
    }
};

/** Property<ImageView, Int> "drawable.tint" */
class DrawableTintProperty : public Property {
public:
    DrawableTintProperty() : Property("drawable.tint", INT_TYPE) {}

    AnimateValue get(void*) const override {
        // Tint cannot be read back through the compat shim upstream either.
        return AnimateValue(0);
    }

    void set(void* o, const AnimateValue& value) const override {
        ImageView& view = *static_cast<ImageView*>(o);
        if (Drawable* drawable = view.getDrawable()) {
            drawable->setTint(GET_VARIANT(value, int));
        }
    }
};

#define VIEW_BOUNDS_PROPERTY(Name, Getter, Setter)                                    \
    class Name##Property : public Property {                                          \
    public:                                                                           \
        Name##Property() : Property(#Name, INT_TYPE) {}                               \
        AnimateValue get(void* t) const override {                                    \
            return AnimateValue(static_cast<View*>(t)->Getter());                     \
        }                                                                             \
        void set(void* o, const AnimateValue& value) const override {                 \
            static_cast<View*>(o)->Setter(GET_VARIANT(value, int));                   \
        }                                                                             \
    };

VIEW_BOUNDS_PROPERTY(ViewLeft, getLeft, setLeft)
VIEW_BOUNDS_PROPERTY(ViewTop, getTop, setTop)
VIEW_BOUNDS_PROPERTY(ViewRight, getRight, setRight)
VIEW_BOUNDS_PROPERTY(ViewBottom, getBottom, setBottom)

} // namespace

const Interpolator* DECELERATE_ACCELERATE_INTERPOLATOR() {
    static DecelerateAccelerateInterpolator sInstance;
    return &sInstance;
}

const Interpolator* INTERPOLATOR_FAST_OUT_SLOW_IN() {
    // FastOutSlowInInterpolator() = PathInterpolator(0.4f, 0f, 0.2f, 1f)
    static PathInterpolator sInstance(0.4f, 0.0f, 0.2f, 1.0f);
    return &sInstance;
}

const Property* BACKGROUND_ALPHA() {
    static BackgroundAlphaProperty sInstance;
    return &sInstance;
}

void setBackgroundAlpha(View& view, int value) {
    Drawable* background = view.getBackground();
    if (dynamic_cast<LayerDrawable*>(background) && ((LayerDrawable*)background)->getNumberOfLayers() > 0) {
        background = ((LayerDrawable*)background)->getDrawable(0);
    }
    if (background) background->setAlpha(value);
}

const Property* DRAWABLE_ALPHA() {
    static DrawableAlphaProperty sInstance;
    return &sInstance;
}

const Property* DRAWABLE_TINT() {
    static DrawableTintProperty sInstance;
    return &sInstance;
}

void setAnimatedFraction(ValueAnimator& animator, float fraction) {
    animator.setCurrentFraction(fraction);
}

void reverse(const std::vector<ValueAnimator*>& animators) {
    for (ValueAnimator* animator : animators) {
        const float fraction = animator->getAnimatedFraction();
        if (fraction > 0.0f) {
            animator->reverse();
            setAnimatedFraction(*animator, 1.0f - fraction);
        }
    }
}

void cancel(const std::vector<Animator*>& animators) {
    for (Animator* animator : animators) {
        animator->cancel();
    }
}

ValueAnimator* getScaleAnimator(View* view, const std::vector<float>& values) {
    std::vector<PropertyValuesHolder*> holders;
    holders.push_back(PropertyValuesHolder::ofFloat(View::SCALE_X, values));
    holders.push_back(PropertyValuesHolder::ofFloat(View::SCALE_Y, values));
    return ObjectAnimator::ofPropertyValuesHolder(view, holders);
}

ValueAnimator* getAlphaAnimator(View* view, const std::vector<float>& values) {
    return ObjectAnimator::ofFloat(view, View::ALPHA, values);
}

const Property* VIEW_LEFT()  { static ViewLeftProperty s;   return &s; }
const Property* VIEW_TOP()   { static ViewTopProperty s;    return &s; }
const Property* VIEW_RIGHT() { static ViewRightProperty s;  return &s; }
const Property* VIEW_BOTTOM(){ static ViewBottomProperty s; return &s; }

Animator* getBoundsAnimator(View& target, View& from, View& to) {
    // Fetch the content insets for the views. Content bounds are what matter, not total bounds.
    Rect targetInsets, fromInsets, toInsets;
    if (Drawable* bg = target.getBackground()) bg->getPadding(targetInsets);
    if (Drawable* bg = from.getBackground())   bg->getPadding(fromInsets);
    if (Drawable* bg = to.getBackground())     bg->getPadding(toInsets);

    // Before animating, the content bounds of target must match the content bounds of from.
    const int startLeft   = from.getLeft()   - fromInsets.left   + targetInsets.left;
    const int startTop    = from.getTop()    - fromInsets.top    + targetInsets.top;
    const int startRight  = from.getRight()  - fromInsets.right()  + targetInsets.right();
    const int startBottom = from.getBottom() - fromInsets.bottom() + targetInsets.bottom();

    // After animating, the content bounds of target must match the content bounds of to.
    const int endLeft   = to.getLeft()   - toInsets.left   + targetInsets.left;
    const int endTop    = to.getTop()    - toInsets.top    + targetInsets.top;
    const int endRight  = to.getRight()  - toInsets.right()  + targetInsets.right();
    const int endBottom = to.getBottom() - toInsets.bottom() + targetInsets.bottom();

    return getBoundsAnimator(target, startLeft, startTop, startRight, startBottom,
            endLeft, endTop, endRight, endBottom);
}

Animator* getBoundsAnimator(View& view, int fromLeft, int fromTop, int fromRight, int fromBottom,
                            int toLeft, int toTop, int toRight, int toBottom) {
    view.setLeft(fromLeft);
    view.setTop(fromTop);
    view.setRight(fromRight);
    view.setBottom(fromBottom);

    std::vector<PropertyValuesHolder*> holders;
    holders.push_back(PropertyValuesHolder::ofInt(VIEW_LEFT(),   {toLeft}));
    holders.push_back(PropertyValuesHolder::ofInt(VIEW_TOP(),    {toTop}));
    holders.push_back(PropertyValuesHolder::ofInt(VIEW_RIGHT(),  {toRight}));
    holders.push_back(PropertyValuesHolder::ofInt(VIEW_BOTTOM(), {toBottom}));
    return ObjectAnimator::ofPropertyValuesHolder(&view, holders);
}

void startDrawableAnimation(ImageView& view) {
    Drawable* d = view.getDrawable();
    if (Animatable* a = dynamic_cast<Animatable*>(d)) {
        a->start();
    }
}

} // namespace AnimatorUtils
} // namespace deskclock
} // namespace cdroid
