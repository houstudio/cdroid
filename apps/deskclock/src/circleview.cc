#include <circleview.h>

#include <R.h>

#include <cmath>

#include <core/context.h>

#include <view/gravity.h>
#include <widget/imageview.h>

#include <utils.h>

using namespace ::deskclock; // generated R.h namespace

namespace cdroid {
namespace deskclock {

namespace {

/** Property<CircleView, Int> "fillColor" */
class FillColorProperty : public Property {
public:
    FillColorProperty() : Property("fillColor", INT_TYPE) {}
    AnimateValue get(void* t) const override {
        return AnimateValue(static_cast<CircleView*>(t)->getFillColor());
    }
    void set(void* o, const AnimateValue& value) const override {
        static_cast<CircleView*>(o)->setFillColor(GET_VARIANT(value, int));
    }
};

/** Property<CircleView, Float> "radius" */
class RadiusProperty : public Property {
public:
    RadiusProperty() : Property("radius", FLOAT_TYPE) {}
    AnimateValue get(void* t) const override {
        return AnimateValue(static_cast<CircleView*>(t)->getRadius());
    }
    void set(void* o, const AnimateValue& value) const override {
        static_cast<CircleView*>(o)->setRadius(GET_VARIANT(value, float));
    }
};

} // namespace

CircleView::CircleView(Context* ctx, const AttributeSet* attrs)
    : View(ctx, attrs)
    , mCircleColor(0xFFFFFFFF)
    , mGravity(Gravity::NO_GRAVITY)
    , mCenterX(0.0f)
    , mCenterY(0.0f)
    , mRadius(0.0f) {
    auto a = ctx->obtainStyledAttributes(attrs, R::styleable::CircleView);

    mGravity = a->getInt(R::styleable::CircleView_gravity, Gravity::NO_GRAVITY);
    mCenterX = a->getDimension(R::styleable::CircleView_centerX, 0.0f);
    mCenterY = a->getDimension(R::styleable::CircleView_centerY, 0.0f);
    mRadius = a->getDimension(R::styleable::CircleView_radius, 0.0f);

    mCircleColor = a->getColor(R::styleable::CircleView_fillColor, 0xFFFFFFFF);
}

void CircleView::onRtlPropertiesChanged(int layoutDirection) {
    View::onRtlPropertiesChanged(layoutDirection);

    if (mGravity != Gravity::NO_GRAVITY) {
        applyGravity(mGravity, layoutDirection);
    }
}

void CircleView::onLayout(bool changed, int left, int top, int right, int bottom) {
    View::onLayout(changed, left, top, right, bottom);

    if (mGravity != Gravity::NO_GRAVITY) {
        applyGravity(mGravity, getLayoutDirection());
    }
}

void CircleView::onDraw(Canvas& canvas) {
    View::onDraw(canvas);

    // draw the circle, duh (Canvas == cairo_t)
    canvas.set_color(mCircleColor);
    canvas.arc(mCenterX, mCenterY, mRadius, 0.0, 2.0 * M_PI);
    canvas.fill();
}

bool CircleView::hasOverlappingRendering() const {
    // only if we have a background, which we shouldn't...
    return getBackground() != nullptr;
}

CircleView& CircleView::setGravity(int gravity) {
    if (mGravity != gravity) {
        mGravity = gravity;

        if (gravity != Gravity::NO_GRAVITY && isLayoutDirectionResolved()) {
            applyGravity(gravity, getLayoutDirection());
        }
    }
    return *this;
}

CircleView& CircleView::setFillColor(int color) {
    if (mCircleColor != color) {
        mCircleColor = color;

        // invalidate the current area
        invalidateCircle(mCenterX, mCenterY, mRadius);
    }
    return *this;
}

CircleView& CircleView::setCenterX(float centerX) {
    const float oldCenterX = mCenterX;
    if (oldCenterX != centerX) {
        mCenterX = centerX;

        // invalidate the old/new areas
        invalidateCircle(oldCenterX, mCenterY, mRadius);
        invalidateCircle(centerX, mCenterY, mRadius);
    }

    // clear the horizontal gravity flags
    mGravity &= ~Gravity::HORIZONTAL_GRAVITY_MASK;

    return *this;
}

CircleView& CircleView::setCenterY(float centerY) {
    const float oldCenterY = mCenterY;
    if (oldCenterY != centerY) {
        mCenterY = centerY;

        // invalidate the old/new areas
        invalidateCircle(mCenterX, oldCenterY, mRadius);
        invalidateCircle(mCenterX, centerY, mRadius);
    }

    // clear the vertical gravity flags
    mGravity &= ~Gravity::VERTICAL_GRAVITY_MASK;

    return *this;
}

CircleView& CircleView::setRadius(float radius) {
    const float oldRadius = mRadius;
    if (oldRadius != radius) {
        mRadius = radius;

        // invalidate the old/new areas
        invalidateCircle(mCenterX, mCenterY, oldRadius);
        if (radius > oldRadius) {
            invalidateCircle(mCenterX, mCenterY, radius);
        }
    }

    // clear the fill gravity flags
    if ((mGravity & Gravity::FILL_HORIZONTAL) == Gravity::FILL_HORIZONTAL) {
        mGravity &= ~Gravity::FILL_HORIZONTAL;
    }
    if ((mGravity & Gravity::FILL_VERTICAL) == Gravity::FILL_VERTICAL) {
        mGravity &= ~Gravity::FILL_VERTICAL;
    }

    return *this;
}

void CircleView::invalidateCircle(float centerX, float centerY, float radius) {
    invalidate((int) (centerX - radius - 0.5f), (int) (centerY - radius - 0.5f),
               (int) (centerX + radius + 0.5f), (int) (centerY + radius + 0.5f));
}

void CircleView::applyGravity(int gravity, int layoutDirection) {
    const int absoluteGravity = Gravity::getAbsoluteGravity(gravity, layoutDirection);

    const float oldRadius = mRadius;
    const float oldCenterX = mCenterX;
    const float oldCenterY = mCenterY;

    switch (absoluteGravity & Gravity::HORIZONTAL_GRAVITY_MASK) {
        case Gravity::LEFT:              mCenterX = 0.0f; break;
        case Gravity::CENTER_HORIZONTAL:
        case Gravity::FILL_HORIZONTAL:   mCenterX = getWidth() / 2.0f; break;
        case Gravity::RIGHT:             mCenterX = (float) getWidth(); break;
    }

    switch (absoluteGravity & Gravity::VERTICAL_GRAVITY_MASK) {
        case Gravity::TOP:               mCenterY = 0.0f; break;
        case Gravity::CENTER_VERTICAL:
        case Gravity::FILL_VERTICAL:     mCenterY = getHeight() / 2.0f; break;
        case Gravity::BOTTOM:            mCenterY = (float) getHeight(); break;
    }

    switch (absoluteGravity & Gravity::FILL) {
        case Gravity::FILL:              mRadius = std::min(getWidth(), getHeight()) / 2.0f; break;
        case Gravity::FILL_HORIZONTAL:   mRadius = getWidth() / 2.0f; break;
        case Gravity::FILL_VERTICAL:     mRadius = getHeight() / 2.0f; break;
    }

    if (oldCenterX != mCenterX || oldCenterY != mCenterY || oldRadius != mRadius) {
        invalidateCircle(oldCenterX, oldCenterY, oldRadius);
        invalidateCircle(mCenterX, mCenterY, mRadius);
    }
}

const Property* CircleView::FILL_COLOR() {
    static FillColorProperty s;
    return &s;
}

const Property* CircleView::RADIUS() {
    static RadiusProperty s;
    return &s;
}

} // namespace deskclock

typedef cdroid::deskclock::CircleView CircleView;
DECLARE_WIDGET2(CircleView, "CircleView");

} // namespace cdroid
