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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <cmath>
#include <algorithm>
#include <utils/mathutils.h>
#include <widget/internal_R.h>
#include <drawable/colorstatelist.h>
#include <animation/propertyvaluesholder.h>
#include <widgetEx/widgetex_styleable.h>
#include <widgetEx/wear/circledimageview.h>

namespace cdroid{
using namespace cdroid::internal;

DECLARE_WIDGET2(CircledImageView, "androidx.wear.widget.CircledImageView");

constexpr uint32_t CircledImageView::OvalShadowPainter::SHADER_COLORS[];
constexpr float CircledImageView::OvalShadowPainter::SHADER_STOPS[];

CircledImageView::CircledImageView(Context* context):CircledImageView(context,nullptr){}

CircledImageView::CircledImageView(Context* context,const AttributeSet* attrs)
    :CircledImageView(context,attrs,0){}

CircledImageView::CircledImageView(Context* context,const AttributeSet* attrs,int defStyle)
    :View(context,attrs,defStyle){
    auto a = context->obtainStyledAttributes(attrs, internal::R::styleable::CircledImageView, 0, 0);
    mDrawable = a->getDrawable(internal::R::styleable::CircledImageView_src);
    if (mDrawable != nullptr && mDrawable->getConstantState() != nullptr) {
        // The provided Drawable may be used elsewhere, so make a mutable clone before setTint()
        // or setAlpha() is called on it.
        Drawable* clone = mDrawable->getConstantState()
                ->newDrawable(&context->getResources());
        delete mDrawable;   // Java GC reclaims the original
        mDrawable = clone->mutate();
    }

    mCircleColor = a->getColorStateList(internal::R::styleable::CircledImageView_background_color);
    if (mCircleColor == nullptr) {
        mCircleColor = ColorStateList::valueOf(context->getColor((int)internal::R::color::darker_gray));
    }

    mCircleRadius = a->getDimension(internal::R::styleable::CircledImageView_background_radius, 0);
    mInitialCircleRadius = mCircleRadius;
    mCircleRadiusPressed = a->getDimension(
            internal::R::styleable::CircledImageView_background_radius_pressed, mCircleRadius);
    mCircleBorderColor = a->getColor(
            internal::R::styleable::CircledImageView_background_border_color, Color::BLACK);
    // Paint.Cap.values()[...]: butt=0, round=1, square=2 (Cairo::Context::LineCap ordinals).
    mCircleBorderCap = a->getInt(internal::R::styleable::CircledImageView_background_border_cap, 0);
    mCircleBorderWidth = a->getDimension(
            internal::R::styleable::CircledImageView_background_border_width, 0);

    if (mCircleBorderWidth > 0) {
        // The border arc is drawn from the middle of the arc - take that into account.
        mRadiusInset += mCircleBorderWidth / 2;
    }

    float circlePadding = a->getDimension(internal::R::styleable::CircledImageView_img_padding, 0);
    if (circlePadding > 0) {
        mRadiusInset += circlePadding;
    }

    mImageCirclePercentage = a->getFloat(
            internal::R::styleable::CircledImageView_img_circle_percentage, 0.f);

    mImageHorizontalOffcenterPercentage =
            a->getFloat(internal::R::styleable::CircledImageView_img_horizontal_offset_percentage, 0.f);

    if (a->hasValue(internal::R::styleable::CircledImageView_img_tint)) {
        mImageTint = a->getColor(internal::R::styleable::CircledImageView_img_tint, 0);
        mImageTintSet = true;
    }

    if (a->hasValue(internal::R::styleable::CircledImageView_clip_dimen)) {
        mSquareDimen = a->getInt(internal::R::styleable::CircledImageView_clip_dimen,
                SQUARE_DIMEN_NONE);
        mSquareDimenSet = true;
    }

    mCircleRadiusPercent = a->getFraction(
            internal::R::styleable::CircledImageView_background_radius_percent, 1, 1, 0.f);

    mCircleRadiusPressedPercent = a->getFraction(
            internal::R::styleable::CircledImageView_background_radius_pressed_percent, 1, 1,
            mCircleRadiusPercent);

    float shadowWidth = a->getDimension(
            internal::R::styleable::CircledImageView_background_shadow_width, 0);

    // a.recycle() — the CDROID TypedArray is a unique_ptr.

    // mPaint = new Paint(); mPaint.setAntiAlias(true);
    mPaint.setAntiAlias(true);
    mShadowPainter = new OvalShadowPainter(shadowWidth, 0, getCircleRadius(),
            mCircleBorderWidth);

    mIndeterminateDrawable = new ProgressDrawable();
    // The drawable callback is held by weak reference upstream; CDROID View IS a
    // Drawable::Callback, so `this` keeps the invalidation path.
    mIndeterminateDrawable->setCallback(this);

    setWillNotDraw(false);

    mAnimationListener = [this](ValueAnimator& animation)->void{
        int color = GET_VARIANT(animation.getAnimatedValue(), int);
        if (color != this->mCurrentColor) {
            this->mCurrentColor = color;
            this->invalidate();
        }
    };

    setColorForCurrentState();
}

CircledImageView::~CircledImageView(){
    // Java GC reclaims the animator/drawables/painter; C++ must free them.
    delete mColorAnimator;
    delete mIndeterminateDrawable;
    delete mDrawable;
    delete mShadowPainter;
}

/** Sets the circle to be hidden. */
void CircledImageView::setCircleHidden(bool circleHidden) {
    if (circleHidden != mCircleHidden) {
        mCircleHidden = circleHidden;
        invalidate();
    }
}

bool CircledImageView::onSetAlpha(int alpha) {
    return true;
}

void CircledImageView::onDraw(Canvas& canvas) {
    int paddingLeft = getPaddingLeft();
    int paddingTop = getPaddingTop();

    float circleRadius = mPressed ? getCircleRadiusPressed() : getCircleRadius();

    // Maybe draw the shadow
    mShadowPainter->draw(canvas, getAlpha());
    if (mCircleBorderWidth > 0) {
        // First let's find the center of the view. (RectF stores x,y,w,h.)
        mOval.set(paddingLeft, paddingTop, getWidth() - getPaddingRight() - paddingLeft,
                getHeight() - getPaddingBottom() - paddingTop);
        // Having the center, lets make the border meet the circle.
        mOval.set(mOval.centerX() - circleRadius, mOval.centerY() - circleRadius,
                circleRadius + circleRadius, circleRadius + circleRadius);
        mPaint.setColor(mCircleBorderColor);
        // Paint.setAlpha is a helper that just sets the alpha portion of the color;
        // Paint.setColor will clear any previously set alpha value.
        mPaint.setAlpha((int)std::round(Color::alpha(mPaint.getColor()) * getAlpha()));
        mPaint.setStyle(Paint::STROKE);
        mPaint.setStrokeWidth(mCircleBorderWidth);
        // mPaint.setStrokeCap(mCircleBorderCap) — applied as the cairo line cap below.

        if (mProgressIndeterminate) {
            // mOval.roundOut(mIndeterminateBounds)
            mIndeterminateBounds.set((int)std::floor(mOval.left), (int)std::floor(mOval.top),
                    (int)std::ceil(mOval.width), (int)std::ceil(mOval.height));
            mIndeterminateDrawable->setBounds(mIndeterminateBounds);
            mIndeterminateDrawable->setRingColor(mCircleBorderColor);
            mIndeterminateDrawable->setRingWidth(mCircleBorderWidth);
            mIndeterminateDrawable->draw(canvas);
        } else {
            // canvas.drawArc(mOval, -90, 360 * mProgress, false, mPaint)
            canvas.set_color((mPaint.getColor() & 0x00FFFFFF) | (mPaint.getAlpha() << 24));
            canvas.set_line_width(mCircleBorderWidth);
            canvas.set_line_cap(static_cast<Cairo::Context::LineCap>(mCircleBorderCap));
            const float startAngle = -90.f;
            const float sweepAngle = 360.f * mProgress;
            canvas.arc(mOval.centerX(), mOval.centerY(), mOval.width / 2.f,
                    MathUtils::toRadians(startAngle),
                    MathUtils::toRadians(startAngle + sweepAngle));
            canvas.stroke();
        }
    }
    if (!mCircleHidden) {
        mOval.set(paddingLeft, paddingTop, getWidth() - getPaddingRight() - paddingLeft,
                getHeight() - getPaddingBottom() - paddingTop);
        mPaint.setColor(mCurrentColor);
        mPaint.setAlpha((int)std::round(Color::alpha(mPaint.getColor()) * getAlpha()));

        mPaint.setStyle(Paint::FILL);
        float centerX = mOval.centerX();
        float centerY = mOval.centerY();

        // canvas.drawCircle(centerX, centerY, circleRadius, mPaint)
        canvas.set_color((mPaint.getColor() & 0x00FFFFFF) | (mPaint.getAlpha() << 24));
        canvas.arc(centerX, centerY, circleRadius, 0, M_PI * 2);
        canvas.fill();
    }

    if (mDrawable != nullptr) {
        mDrawable->setAlpha((int)std::round(getAlpha() * 255));

        if (mImageTintSet) {   // Java: mImageTint != null
            mDrawable->setTint(mImageTint);
        }
        mDrawable->draw(canvas);
    }

    View::onDraw(canvas);
}

void CircledImageView::setColorForCurrentState() {
    int newColor = mCircleColor->getColorForState(getDrawableState(),
            mCircleColor->getDefaultColor());
    if (mColorChangeAnimationDurationMs > 0) {
        if (mColorAnimator != nullptr) {
            mColorAnimator->cancel();
        } else {
            mColorAnimator = new ValueAnimator();
        }
        mColorAnimator->setIntValues({mCurrentColor, newColor});
        mColorAnimator->setEvaluator(PropertyValuesHolder::ArgbEvaluator);
        mColorAnimator->setDuration(mColorChangeAnimationDurationMs);
        mColorAnimator->addUpdateListener(mAnimationListener);
        mColorAnimator->start();
    } else {
        if (newColor != mCurrentColor) {
            mCurrentColor = newColor;
            invalidate();
        }
    }
}

void CircledImageView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    const float radius = getCircleRadius() + mCircleBorderWidth
            + mShadowPainter->mShadowWidth * mShadowPainter->mShadowVisibility;
    float desiredWidth = radius * 2;
    float desiredHeight = radius * 2;

    int widthMode = MeasureSpec::getMode(widthMeasureSpec);
    int widthSize = MeasureSpec::getSize(widthMeasureSpec);
    int heightMode = MeasureSpec::getMode(heightMeasureSpec);
    int heightSize = MeasureSpec::getSize(heightMeasureSpec);

    int width;
    int height;

    if (widthMode == MeasureSpec::EXACTLY) {
        width = widthSize;
    } else if (widthMode == MeasureSpec::AT_MOST) {
        width = (int)std::min(desiredWidth, (float)widthSize);
    } else {
        width = (int)desiredWidth;
    }

    if (heightMode == MeasureSpec::EXACTLY) {
        height = heightSize;
    } else if (heightMode == MeasureSpec::AT_MOST) {
        height = (int)std::min(desiredHeight, (float)heightSize);
    } else {
        height = (int)desiredHeight;
    }

    if (mSquareDimenSet) {   // Java: mSquareDimen != null
        switch (mSquareDimen) {
            case SQUARE_DIMEN_HEIGHT:
                width = height;
                break;
            case SQUARE_DIMEN_WIDTH:
                height = width;
                break;
        }
    }

    View::onMeasure(MeasureSpec::makeMeasureSpec(width, MeasureSpec::EXACTLY),
            MeasureSpec::makeMeasureSpec(height, MeasureSpec::EXACTLY));
}

void CircledImageView::onLayout(bool changed, int left, int top, int width, int height) {
    if (mDrawable != nullptr) {
        // Retrieve the sizes of the drawable and the view.
        const int nativeDrawableWidth = mDrawable->getIntrinsicWidth();
        const int nativeDrawableHeight = mDrawable->getIntrinsicHeight();
        const int viewWidth = getMeasuredWidth();
        const int viewHeight = getMeasuredHeight();
        const float imageCirclePercentage =
                mImageCirclePercentage > 0 ? mImageCirclePercentage : 1;

        const float scaleFactor = std::min(1.f, std::min(
                (float)nativeDrawableWidth != 0
                        ? imageCirclePercentage * viewWidth / nativeDrawableWidth
                        : 1,
                (float)nativeDrawableHeight != 0
                        ? imageCirclePercentage * viewHeight / nativeDrawableHeight
                        : 1));

        // Scale the drawable down to fit the view, if needed.
        const int drawableWidth = (int)std::round(scaleFactor * nativeDrawableWidth);
        const int drawableHeight = (int)std::round(scaleFactor * nativeDrawableHeight);

        // Center the drawable within the view.
        const int drawableLeft = (viewWidth - drawableWidth) / 2
                + (int)std::round(mImageHorizontalOffcenterPercentage * drawableWidth);
        const int drawableTop = (viewHeight - drawableHeight) / 2;

        // Java setBounds(l, t, l + w, t + h); CDROID Drawable bounds are (x, y, w, h).
        mDrawable->setBounds(drawableLeft, drawableTop, drawableWidth, drawableHeight);
    }

    View::onLayout(changed, left, top, width, height);
}

/** Sets the image given a resource. */
void CircledImageView::setImageResource(int resId) {
    setImageDrawable(resId == 0 ? nullptr : getContext()->getDrawable(resId));
}

/** Sets the size of the image based on a percentage in [0, 1]. */
void CircledImageView::setImageCirclePercentage(float percentage) {
    float clamped = std::max(0.f, std::min(1.f, percentage));
    if (clamped != mImageCirclePercentage) {
        mImageCirclePercentage = clamped;
        invalidate();
    }
}

/** Sets the horizontal offset given a percentage in [0, 1]. */
void CircledImageView::setImageHorizontalOffcenterPercentage(float percentage) {
    if (percentage != mImageHorizontalOffcenterPercentage) {
        mImageHorizontalOffcenterPercentage = percentage;
        invalidate();
    }
}

/** Sets the tint. */
void CircledImageView::setImageTint(int tint) {
    if (!mImageTintSet || tint != mImageTint) {
        mImageTint = tint;
        mImageTintSet = true;
        invalidate();
    }
}

/** Returns the circle radius. */
float CircledImageView::getCircleRadius() const {
    float radius = mCircleRadius;
    if (mCircleRadius <= 0 && mCircleRadiusPercent > 0) {
        radius = std::max((float)getMeasuredHeight(), (float)getMeasuredWidth()) * mCircleRadiusPercent;
    }

    return radius - mRadiusInset;
}

/** Sets the circle radius. */
void CircledImageView::setCircleRadius(float circleRadius) {
    if (circleRadius != mCircleRadius) {
        mCircleRadius = circleRadius;
        mShadowPainter->setInnerCircleRadius(mPressed ? getCircleRadiusPressed() : getCircleRadius());
        invalidate();
    }
}

/** Gets the circle radius percent. */
float CircledImageView::getCircleRadiusPercent() const {
    return mCircleRadiusPercent;
}

/** Sets the radius of the circle to be a percentage of the largest dimension of the view. */
void CircledImageView::setCircleRadiusPercent(float circleRadiusPercent) {
    if (circleRadiusPercent != mCircleRadiusPercent) {
        mCircleRadiusPercent = circleRadiusPercent;
        mShadowPainter->setInnerCircleRadius(mPressed ? getCircleRadiusPressed() : getCircleRadius());
        invalidate();
    }
}

/** Gets the circle radius when pressed. */
float CircledImageView::getCircleRadiusPressed() const {
    float radius = mCircleRadiusPressed;

    if (mCircleRadiusPressed <= 0 && mCircleRadiusPressedPercent > 0) {
        radius = std::max((float)getMeasuredHeight(), (float)getMeasuredWidth())
                * mCircleRadiusPressedPercent;
    }

    return radius - mRadiusInset;
}

/** Sets the circle radius when pressed. */
void CircledImageView::setCircleRadiusPressed(float circleRadiusPressed) {
    if (circleRadiusPressed != mCircleRadiusPressed) {
        mCircleRadiusPressed = circleRadiusPressed;
        invalidate();
    }
}

/** Gets the circle radius when pressed as a percent. */
float CircledImageView::getCircleRadiusPressedPercent() const {
    return mCircleRadiusPressedPercent;
}

/** Sets the radius of the circle to be a percentage of the largest dimension of the view when pressed. */
void CircledImageView::setCircleRadiusPressedPercent(float circleRadiusPressedPercent) {
    if (circleRadiusPressedPercent != mCircleRadiusPressedPercent) {
        mCircleRadiusPressedPercent = circleRadiusPressedPercent;
        mShadowPainter->setInnerCircleRadius(mPressed ? getCircleRadiusPressed() : getCircleRadius());
        invalidate();
    }
}

void CircledImageView::drawableStateChanged() {
    View::drawableStateChanged();
    setColorForCurrentState();
}

/** Sets the circle color. */
void CircledImageView::setCircleColor(int circleColor) {
    setCircleColorStateList(ColorStateList::valueOf(circleColor));
}

/** Gets the circle color. */
RefPtr<ColorStateList> CircledImageView::getCircleColorStateList() const {
    return mCircleColor;
}

/** Sets the circle color. */
void CircledImageView::setCircleColorStateList(const RefPtr<ColorStateList>& circleColor) {
    // Objects.equals(circleColor, mCircleColor): ColorStateList does not override
    // equals() upstream, so this is an identity comparison — shared_ptr equality.
    if (circleColor != mCircleColor) {
        mCircleColor = circleColor;
        setColorForCurrentState();
        invalidate();
    }
}

/** Gets the default circle color. */
int CircledImageView::getDefaultCircleColor() const {
    return mCircleColor->getDefaultColor();
}

/**
 * Show the circle border as an indeterminate progress spinner. The views circle border width
 * and color must be set for this to have an effect.
 */
void CircledImageView::showIndeterminateProgress(bool show) {
    mProgressIndeterminate = show;
    if (mIndeterminateDrawable != nullptr) {
        if (show && mVisible && mWindowVisible) {
            mIndeterminateDrawable->startAnimation();
        } else {
            mIndeterminateDrawable->stopAnimation();
        }
    }
}

void CircledImageView::onVisibilityChanged(View& changedView, int visibility) {
    View::onVisibilityChanged(changedView, visibility);
    mVisible = (visibility == View::VISIBLE);
    showIndeterminateProgress(mProgressIndeterminate);
}

void CircledImageView::onWindowVisibilityChanged(int visibility) {
    View::onWindowVisibilityChanged(visibility);
    mWindowVisible = (visibility == View::VISIBLE);
    showIndeterminateProgress(mProgressIndeterminate);
}

/** Sets the progress. */
void CircledImageView::setProgress(float progress) {
    if (progress != mProgress) {
        mProgress = progress;
        invalidate();
    }
}

/**
 * Set how much of the shadow should be shown.
 * @param shadowVisibility Value between 0 and 1.
 */
void CircledImageView::setShadowVisibility(float shadowVisibility) {
    if (shadowVisibility != mShadowPainter->mShadowVisibility) {
        mShadowPainter->setShadowVisibility(shadowVisibility);
        invalidate();
    }
}

float CircledImageView::getInitialCircleRadius() const {
    return mInitialCircleRadius;
}

void CircledImageView::setCircleBorderColor(int circleBorderColor) {
    mCircleBorderColor = circleBorderColor;
}

/**
 * Set the border around the circle.
 * @param circleBorderWidth Width of the border around the circle.
 */
void CircledImageView::setCircleBorderWidth(float circleBorderWidth) {
    if (circleBorderWidth != mCircleBorderWidth) {
        mCircleBorderWidth = circleBorderWidth;
        mShadowPainter->setInnerCircleBorderWidth(circleBorderWidth);
        invalidate();
    }
}

/**
 * Set the stroke cap for the border around the circle.
 * @param circleBorderCap Stroke cap for the border around the circle.
 */
void CircledImageView::setCircleBorderCap(int circleBorderCap) {
    if (circleBorderCap != mCircleBorderCap) {
        mCircleBorderCap = circleBorderCap;
        invalidate();
    }
}

void CircledImageView::setPressed(bool pressed) {
    View::setPressed(pressed);
    if (pressed != mPressed) {
        mPressed = pressed;
        mShadowPainter->setInnerCircleRadius(mPressed ? getCircleRadiusPressed() : getCircleRadius());
        invalidate();
    }
}

void CircledImageView::setPadding(int left, int top, int right, int bottom) {
    if (left != getPaddingLeft() || top != getPaddingTop()
            || right != getPaddingRight() || bottom != getPaddingBottom()) {
        mShadowPainter->setBounds(left, top, getWidth() - right, getHeight() - bottom);
    }
    View::setPadding(left, top, right, bottom);
}

void CircledImageView::onSizeChanged(int newWidth, int newHeight, int oldWidth, int oldHeight) {
    if (newWidth != oldWidth || newHeight != oldHeight) {
        mShadowPainter->setBounds(getPaddingLeft(), getPaddingTop(),
                newWidth - getPaddingRight(), newHeight - getPaddingBottom());
    }
}

Drawable* CircledImageView::getImageDrawable() {
    return mDrawable;
}

/** Sets the image drawable. */
void CircledImageView::setImageDrawable(Drawable* drawable) {
    if (drawable != mDrawable) {
        Drawable* existingDrawable = mDrawable;
        mDrawable = drawable;
        Drawable* passedToFree = nullptr;   // freed after the skipLayout comparison below
        if (mDrawable != nullptr && mDrawable->getConstantState() != nullptr) {
            // The provided Drawable may be used elsewhere, so make a mutable clone before
            // setTint() or setAlpha() is called on it.
            Drawable* clone = mDrawable->getConstantState()
                    ->newDrawable(&getContext()->getResources());
            passedToFree = mDrawable;   // Java GC reclaims the passed instance
            mDrawable = clone->mutate();
        }

        const bool skipLayout = drawable != nullptr && existingDrawable != nullptr
                && existingDrawable->getIntrinsicHeight() == drawable->getIntrinsicHeight()
                && existingDrawable->getIntrinsicWidth() == drawable->getIntrinsicWidth();

        if (skipLayout) {
            mDrawable->setBounds(existingDrawable->getBounds());
        } else {
            requestLayout();
        }

        invalidate();

        // Java drops the replaced reference (GC reclaims it); C++ owns both it
        // and the superseded drawable.
        delete passedToFree;
        delete existingDrawable;
    }
}

/** @return the milliseconds duration of the transition animation when the color changes. */
int64_t CircledImageView::getColorChangeAnimationDuration() const {
    return mColorChangeAnimationDurationMs;
}

void CircledImageView::setColorChangeAnimationDuration(int64_t durationMillis) {
    mColorChangeAnimationDurationMs = durationMillis;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// CircledImageView::OvalShadowPainter
///////////////////////////////////////////////////////////////////////////////////////////////////

CircledImageView::OvalShadowPainter::OvalShadowPainter(float shadowWidth, float shadowVisibility,
        float innerCircleRadius, float innerCircleBorderWidth)
    : mShadowWidth(shadowWidth), mShadowVisibility(shadowVisibility),
      mInnerCircleRadius(innerCircleRadius), mInnerCircleBorderWidth(innerCircleBorderWidth) {
    mShadowPaint.setColor(Color::BLACK);
    mShadowPaint.setStyle(Paint::FILL);
    mShadowPaint.setAntiAlias(true);
    updateRadialGradient();
}

void CircledImageView::OvalShadowPainter::draw(Canvas& canvas, float alpha) {
    if (mShadowWidth > 0 && mShadowVisibility > 0) {
        mShadowPaint.setAlpha((int)std::round(mShadowPaint.getAlpha() * alpha));
        // Skia modulates the shader output by the paint alpha; cairo fills a
        // pattern source at full alpha, so clip to the circle and paint the
        // gradient through paint_with_alpha instead.
        if (mShadowPaint.getShader()) {
            canvas.save();
            canvas.arc(mBounds.centerX(), mBounds.centerY(), mShadowRadius, 0, M_PI * 2);
            canvas.clip();
            canvas.set_source(mShadowPaint.getShader());
            canvas.paint_with_alpha(mShadowPaint.getAlpha() / 255.f);
            canvas.restore();
        } else {
            canvas.set_color(mShadowPaint.getColor());
            canvas.arc(mBounds.centerX(), mBounds.centerY(), mShadowRadius, 0, M_PI * 2);
            canvas.fill();
        }
    }
}

void CircledImageView::OvalShadowPainter::setBounds(int left, int top, int right, int bottom) {
    // android RectF.set(l, t, r, b); CDROID RectF stores (x, y, w, h).
    mBounds.set(left, top, right - left, bottom - top);
    updateRadialGradient();
}

void CircledImageView::OvalShadowPainter::setInnerCircleRadius(float newInnerCircleRadius) {
    mInnerCircleRadius = newInnerCircleRadius;
    updateRadialGradient();
}

void CircledImageView::OvalShadowPainter::setInnerCircleBorderWidth(float newInnerCircleBorderWidth) {
    mInnerCircleBorderWidth = newInnerCircleBorderWidth;
    updateRadialGradient();
}

void CircledImageView::OvalShadowPainter::setShadowVisibility(float newShadowVisibility) {
    mShadowVisibility = newShadowVisibility;
    updateRadialGradient();
}

void CircledImageView::OvalShadowPainter::updateRadialGradient() {
    // Make the shadow start beyond the circled and possibly the border.
    mShadowRadius = mInnerCircleRadius + mInnerCircleBorderWidth + mShadowWidth * mShadowVisibility;
    // This may happen if the innerCircleRadius has not been correctly computed yet while
    // the view has already been inflated, but not yet measured. In this case, if the view
    // specifies the radius as a percentage of the screen width, then that evaluates to 0
    // and will be corrected after measuring, through onSizeChanged().
    if (mShadowRadius > 0) {
        // new RadialGradient(cx, cy, r, {BLACK, TRANSPARENT}, {0.6f, 1f}, MIRROR)
        Cairo::RefPtr<Cairo::RadialGradient> shader = Cairo::RadialGradient::create(
                mBounds.centerX(), mBounds.centerY(), 0,
                mBounds.centerX(), mBounds.centerY(), mShadowRadius);
        for (size_t i = 0; i < sizeof(SHADER_STOPS) / sizeof(SHADER_STOPS[0]); i++) {
            const uint32_t c = SHADER_COLORS[i];
            shader->add_color_stop_rgba(SHADER_STOPS[i],
                    Color::red(c) / 255.0, Color::green(c) / 255.0,
                    Color::blue(c) / 255.0, Color::alpha(c) / 255.0);
        }
        shader->set_extend(Cairo::Pattern::Extend::REFLECT);   // Shader.TileMode.MIRROR
        mShadowPaint.setShader(shader);
    }
}

}/*endof namespace*/
