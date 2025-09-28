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
#include <core/color.h>
#include <core/mathutils.h>
#include <animation/valueanimator.h>
#include <widgetEx/wear/circularprogressdrawable.h>

namespace cdroid{
CircularProgressDrawable::CircularProgressDrawable(Context* context) {
    //mResources = Preconditions.checkNotNull(context).getResources();
    mContext = context;
    mRotation =0;
    mRing = new Ring();
    mRing->setColors({(int)Color::BLACK});

    setStrokeWidth(STROKE_WIDTH);
    setupAnimators();
}

CircularProgressDrawable::~CircularProgressDrawable(){
    delete mRing;
    delete mAnimator;
}

void CircularProgressDrawable::setSizeParameters(float centerRadius, float strokeWidth, float arrowWidth,
        float arrowHeight) {
    const DisplayMetrics metrics = mContext->getDisplayMetrics();
    const float screenDensity = metrics.density;

    mRing->setStrokeWidth(strokeWidth * screenDensity);
    mRing->setCenterRadius(centerRadius * screenDensity);
    mRing->setColorIndex(0);
    mRing->setArrowDimensions(arrowWidth * screenDensity, arrowHeight * screenDensity);
}

void CircularProgressDrawable::setStyle(int size) {
    if (size == LARGE) {
        setSizeParameters(CENTER_RADIUS_LARGE, STROKE_WIDTH_LARGE, ARROW_WIDTH_LARGE,
                ARROW_HEIGHT_LARGE);
    } else {
        setSizeParameters(CENTER_RADIUS, STROKE_WIDTH, ARROW_WIDTH, ARROW_HEIGHT);
    }
    invalidateSelf();
}

float CircularProgressDrawable::getStrokeWidth() const{
    return mRing->getStrokeWidth();
}

void CircularProgressDrawable::setStrokeWidth(float strokeWidth) {
    mRing->setStrokeWidth(strokeWidth);
    invalidateSelf();
}

float CircularProgressDrawable::getCenterRadius() const{
    return mRing->getCenterRadius();
}

void CircularProgressDrawable::setCenterRadius(float centerRadius) {
    mRing->setCenterRadius(centerRadius);
    invalidateSelf();
}

void CircularProgressDrawable::setStrokeCap(int strokeCap) {
    mRing->setStrokeCap(strokeCap);
    invalidateSelf();
}

int CircularProgressDrawable::getStrokeCap() const{
    return mRing->getStrokeCap();
}

float CircularProgressDrawable::getArrowWidth() const{
    return mRing->getArrowWidth();
}

float CircularProgressDrawable::getArrowHeight() const{
    return mRing->getArrowHeight();
}

void CircularProgressDrawable::setArrowDimensions(float width, float height) {
    mRing->setArrowDimensions(width, height);
    invalidateSelf();
}

bool CircularProgressDrawable::getArrowEnabled() const{
    return mRing->getShowArrow();
}

void CircularProgressDrawable::setArrowEnabled(bool show) {
    mRing->setShowArrow(show);
    invalidateSelf();
}

float CircularProgressDrawable::getArrowScale() const{
    return mRing->getArrowScale();
}

void CircularProgressDrawable::setArrowScale(float scale) {
    mRing->setArrowScale(scale);
    invalidateSelf();
}

float CircularProgressDrawable::getStartTrim() const{
    return mRing->getStartTrim();
}

float CircularProgressDrawable::getEndTrim() const{
    return mRing->getEndTrim();
}

void CircularProgressDrawable::setStartEndTrim(float start, float end) {
    mRing->setStartTrim(start);
    mRing->setEndTrim(end);
    invalidateSelf();
}

float CircularProgressDrawable::getProgressRotation() const{
    return mRing->getRotation();
}

void CircularProgressDrawable::setProgressRotation(float rotation) {
    mRing->setRotation(rotation);
    invalidateSelf();
}

int CircularProgressDrawable::getBackgroundColor() const{
    return mRing->getBackgroundColor();
}

void CircularProgressDrawable::setBackgroundColor(int color) {
    mRing->setBackgroundColor(color);
    invalidateSelf();
}

std::vector<int> CircularProgressDrawable::getColorSchemeColors() const{
    return mRing->getColors();
}

void CircularProgressDrawable::setColorSchemeColors(const std::vector<int>&colors) {
    mRing->setColors(colors);
    mRing->setColorIndex(0);
    invalidateSelf();
}

void CircularProgressDrawable::draw(Canvas& canvas) {
    Rect bounds = getBounds();
    canvas.save();
    const float centerX = float(bounds.left+ bounds.width)/2.f;
    const float centerY = float(bounds.top + bounds.height)/2.f;
    //canvas.rotate(mRotation, centerX,centerY);
    canvas.translate(centerX ,centerY);
    canvas.rotate_degrees(mRotation);
    canvas.translate(-centerX,-centerY);
    mRing->draw(canvas, bounds);
    canvas.restore();
}

void CircularProgressDrawable::setAlpha(int alpha) {
    mRing->setAlpha(alpha);
    invalidateSelf();
}

int CircularProgressDrawable::getAlpha() const{
    return mRing->getAlpha();
}

void CircularProgressDrawable::setColorFilter(ColorFilter* colorFilter) {
    mRing->setColorFilter(colorFilter);
    invalidateSelf();
}

void CircularProgressDrawable::setRotation(float rotation) {
    mRotation = rotation;
}

float CircularProgressDrawable::getRotation() const{
    return mRotation;
}

int CircularProgressDrawable::getOpacity() {
    return PixelFormat::TRANSLUCENT;
}

bool CircularProgressDrawable::isRunning() {
    return mAnimator->isRunning();
}

void CircularProgressDrawable::start() {
    mAnimator->cancel();
    mRing->storeOriginals();
    // Already showing some part of the ring
    if (mRing->getEndTrim() != mRing->getStartTrim()) {
        mFinishing = true;
        mAnimator->setDuration(ANIMATION_DURATION / 2);
        mAnimator->start();
    } else {
        mRing->setColorIndex(0);
        mRing->resetOriginals();
        mAnimator->setDuration(ANIMATION_DURATION);
        mAnimator->start();
    }
}

void CircularProgressDrawable::stop() {
    mAnimator->cancel();
    setRotation(0);
    mRing->setShowArrow(false);
    mRing->setColorIndex(0);
    mRing->resetOriginals();
    invalidateSelf();
}

// Adapted from ArgbEvaluator.java
int CircularProgressDrawable::evaluateColorChange(float fraction, int startValue, int endValue) {
    int startA = (startValue >> 24) & 0xff;
    int startR = (startValue >> 16) & 0xff;
    int startG = (startValue >> 8) & 0xff;
    int startB = startValue & 0xff;

    int endA = (endValue >> 24) & 0xff;
    int endR = (endValue >> 16) & 0xff;
    int endG = (endValue >> 8) & 0xff;
    int endB = endValue & 0xff;

    return (startA + (int) (fraction * (endA - startA))) << 24
            | (startR + (int) (fraction * (endR - startR))) << 16
            | (startG + (int) (fraction * (endG - startG))) << 8
            | (startB + (int) (fraction * (endB - startB)));
}

void CircularProgressDrawable::updateRingColor(float interpolatedTime, Ring* ring) {
    if (interpolatedTime > COLOR_CHANGE_OFFSET) {
        ring->setColor(evaluateColorChange((interpolatedTime - COLOR_CHANGE_OFFSET)
                        / (1.f - COLOR_CHANGE_OFFSET), ring->getStartingColor(),
                ring->getNextColor()));
    } else {
        ring->setColor(ring->getStartingColor());
    }
}

void CircularProgressDrawable::applyFinishTranslation(float interpolatedTime, Ring* ring) {
    // shrink back down and complete a full rotation before
    // starting other circles
    // Rotation goes between [0..1].
    updateRingColor(interpolatedTime, ring);
    float targetRotation = (float) (std::floor(ring->getStartingRotation() / MAX_PROGRESS_ARC)
            + 1.f);
    const float startTrim = ring->getStartingStartTrim()
            + (ring->getStartingEndTrim() - MIN_PROGRESS_ARC - ring->getStartingStartTrim())
            * interpolatedTime;
    ring->setStartTrim(startTrim);
    ring->setEndTrim(ring->getStartingEndTrim());
    const float rotation = ring->getStartingRotation()
            + ((targetRotation - ring->getStartingRotation()) * interpolatedTime);
    ring->setRotation(rotation);
}

void CircularProgressDrawable::applyTransformation(float interpolatedTime, Ring* ring, bool lastFrame) {
    if (mFinishing) {
        applyFinishTranslation(interpolatedTime, ring);
        // Below condition is to work around a ValueAnimator issue where onAnimationRepeat is
        // called before last frame (1f).
    } else if (interpolatedTime != 1.0f || lastFrame) {
        const float startingRotation = ring->getStartingRotation();
        float startTrim, endTrim;
        auto MATERIAL_INTERPOLATOR = FastOutSlowInInterpolator::gFastOutSlowInInterpolator.get();

        if (interpolatedTime < SHRINK_OFFSET) { // Expansion occurs on first half of animation
            const float scaledTime = interpolatedTime / SHRINK_OFFSET;
            startTrim = ring->getStartingStartTrim();
            endTrim = startTrim + ((MAX_PROGRESS_ARC - MIN_PROGRESS_ARC)
                    * MATERIAL_INTERPOLATOR->getInterpolation(scaledTime) + MIN_PROGRESS_ARC);
        } else { // Shrinking occurs on second half of animation
            float scaledTime = (interpolatedTime - SHRINK_OFFSET) / (1.f - SHRINK_OFFSET);
            endTrim = ring->getStartingStartTrim() + (MAX_PROGRESS_ARC - MIN_PROGRESS_ARC);
            startTrim = endTrim - ((MAX_PROGRESS_ARC - MIN_PROGRESS_ARC)
                    * (1.0f - MATERIAL_INTERPOLATOR->getInterpolation(scaledTime))
                    + MIN_PROGRESS_ARC);
        }

        const float rotation = startingRotation + (RING_ROTATION * interpolatedTime);
        const float groupRotation = GROUP_FULL_ROTATION * (interpolatedTime + mRotationCount);

        ring->setStartTrim(startTrim);
        ring->setEndTrim(endTrim);
        ring->setRotation(rotation);
        setRotation(groupRotation);
    }
}

void CircularProgressDrawable::setupAnimators() {
    Ring* ring = mRing;
    ValueAnimator* animator = ValueAnimator::ofFloat({0.f, 1.f});
    ValueAnimator::AnimatorUpdateListener aul;
    aul = [this](ValueAnimator& animation)->void{
        float interpolatedTime = GET_VARIANT(animation.getAnimatedValue(),float);
        updateRingColor(interpolatedTime, mRing);
        applyTransformation(interpolatedTime, mRing, false);
        invalidateSelf();
    };
    animator->addUpdateListener(aul);
    animator->setRepeatCount(ValueAnimator::INFINITE);
    animator->setRepeatMode(ValueAnimator::RESTART);
    animator->setInterpolator(LinearInterpolator::gLinearInterpolator.get());//LINEAR_INTERPOLATOR);
    Animator::AnimatorListener al;
    al.onAnimationStart = [this](Animator& animator, bool/*isReverse*/){
        mRotationCount = 0;
    };
    al.onAnimationRepeat= [this](Animator& animator){
        applyTransformation(1.f, mRing, true);
        mRing->storeOriginals();
        mRing->goToNextColor();
        if (mFinishing) {
            // finished closing the last ring from the swipe gesture; go
            // into progress mode
            mFinishing = false;
            animator.cancel();
            animator.setDuration(ANIMATION_DURATION);
            animator.start();
            mRing->setShowArrow(false);
        } else {
            mRotationCount = mRotationCount + 1;
        }
    };
    animator->addListener(al);
    mAnimator = animator;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CircularProgressDrawable::Ring::Ring() {
    mStartTrim = 0.f;
    mEndTrim = 0.f;
    mRotation = 0.f;
    mStrokeWidth = 5.f;
    mArrowScale =1;
    mAlpha = 0xFF;
    mArrowWidth =0;
    mArrowHeight=0;
    mRingCap=static_cast<int>(Cairo::Context::LineCap::SQUARE);
    mRingCenterRadius =0.f;
    /*mPaint.setStrokeCap(Paint.Cap.SQUARE);
    mPaint.setAntiAlias(true);
    mPaint.setStyle(Style.STROKE);

    mArrowPaint.setStyle(Paint.Style.FILL);
    mArrowPaint.setAntiAlias(true);*/

    //mCirclePaint.setColor(Color::TRANSPARENT);
    mCircleColor = Color::TRANSPARENT;
}

void CircularProgressDrawable::Ring::setArrowDimensions(float width, float height) {
    mArrowWidth = (int) width;
    mArrowHeight = (int) height;
}

void CircularProgressDrawable::Ring::setStrokeCap(int strokeCap) {
    mRingCap = strokeCap;//mPaint.setStrokeCap(strokeCap);
}

int CircularProgressDrawable::Ring::getStrokeCap() const{
    return mRingCap;//mPaint.getStrokeCap();
}

float CircularProgressDrawable::Ring::getArrowWidth() const{
    return mArrowWidth;
}

float CircularProgressDrawable::Ring::getArrowHeight() const{
    return mArrowHeight;
}

void CircularProgressDrawable::Ring::draw(Canvas& c,const Rect& bounds) {
    RectF arcBounds = mTempBounds;
    float arcRadius = mRingCenterRadius + mStrokeWidth / 2.0f;
    if (mRingCenterRadius <= 0) {
        // If center radius is not set, fill the bounds
        arcRadius = std::min(bounds.width, bounds.height) / 2.0f - std::max(
                (mArrowWidth * mArrowScale) / 2.0f, mStrokeWidth / 2.0f);
    }
    arcBounds.set(bounds.centerX() - arcRadius,
            bounds.centerY() - arcRadius,
            arcRadius + arcRadius,
            arcRadius + arcRadius);

    const float startAngle = (mStartTrim + mRotation) * 360.f;
    const float endAngle = (mEndTrim + mRotation) * 360.f;
    float sweepAngle = endAngle - startAngle;

    //mPaint.setColor(mCurrentColor);mPaint.setAlpha(mAlpha);
    c.set_color(mCircleColor);

    // Draw the background first
    const float inset = mStrokeWidth / 2.0f; // Calculate inset to draw inside the arc
    arcBounds.inset(inset, inset); // Apply inset
    //c.drawCircle(arcBounds.centerX(), arcBounds.centerY(), arcBounds.width / 2.0f,mCirclePaint);
    c.arc(arcBounds.centerX(), arcBounds.centerY(),arcBounds.width / 2.0f,0,M_PI*2.0);
    c.fill();
    arcBounds.inset(-inset, -inset); // Revert the inset

    //c.drawArc(arcBounds, startAngle, sweepAngle, false, mPaint);
    c.set_color(mCurrentColor);
    c.set_line_width(mStrokeWidth);
    c.set_antialias(Cairo::ANTIALIAS_GRAY);
    c.set_line_cap(static_cast<Cairo::Context::LineCap>(mRingCap));
    c.arc(arcBounds.centerX(),arcBounds.centerY(),arcBounds.width/2,
            MathUtils::toRadians(startAngle),MathUtils::toRadians(endAngle));
    c.stroke();
    drawTriangle(c, startAngle, sweepAngle, arcBounds);
}

void CircularProgressDrawable::Ring::drawTriangle(Canvas& c, float startAngle, float sweepAngle,const RectF& bounds) {
    if (mShowArrow) {
        if (mArrow == nullptr) {
            mArrow = std::make_unique<cdroid::Path>();
            //mArrow.setFillType(android.graphics.Path.FillType.EVEN_ODD);
        } else {
            mArrow->reset();
        }
        float centerRadius = std::min(bounds.width, bounds.height) / 2.0f;
        float inset = mArrowWidth * mArrowScale / 2.0f;
        // Update the path each time. This works around an issue in SKIA
        // where concatenating a rotation matrix to a scale matrix
        // ignored a starting negative rotation. This appears to have
        // been fixed as of API 21.
        mArrow->move_to(0, 0);
        mArrow->line_to(mArrowWidth * mArrowScale, 0);
        mArrow->line_to((mArrowWidth * mArrowScale / 2), (mArrowHeight * mArrowScale));
        //mArrow->offset(centerRadius + bounds.centerX() - inset, bounds.centerY() + mStrokeWidth / 2.0f);
        mArrow->close_path();
        // draw a triangle
        //mArrowPaint.setColor(mCurrentColor);mArrowPaint.setAlpha(mAlpha);
        c.set_color(mCurrentColor);
        c.save();
        //c.rotate(startAngle + sweepAngle, bounds.centerX(), bounds.centerY());
        c.translate( bounds.centerX(), bounds.centerY());
        c.rotate_degrees(startAngle + sweepAngle);
        c.translate(-bounds.centerX(),-bounds.centerY());
        //c.drawPath(mArrow, mArrowPaint);
        mArrow->append_to_context(&c);
        c.fill();
        c.restore();
    }
}

void CircularProgressDrawable::Ring::setColors(const std::vector<int>&colors) {
    mColors = colors;
    // if colors are reset, make sure to reset the color index as well
    setColorIndex(0);
}

std::vector<int> CircularProgressDrawable::Ring::getColors() const{
    return mColors;
}

void CircularProgressDrawable::Ring::setColor(int color) {
    mCurrentColor = color;
}

void CircularProgressDrawable::Ring::setBackgroundColor(int color) {
    //mCirclePaint.setColor(color);
    mCircleColor = color;
}

int CircularProgressDrawable::Ring::getBackgroundColor() const{
    return mCircleColor;//mCirclePaint.getColor();
}

void CircularProgressDrawable::Ring::setColorIndex(int index) {
    mColorIndex = index;
    mCurrentColor = mColors[mColorIndex];
}

int CircularProgressDrawable::Ring::getNextColor() const{
    return mColors[getNextColorIndex()];
}

int CircularProgressDrawable::Ring::getNextColorIndex() const{
    return (mColorIndex + 1) % mColors.size();
}

void CircularProgressDrawable::Ring::goToNextColor() {
    setColorIndex(getNextColorIndex());
}

void CircularProgressDrawable::Ring::setColorFilter(ColorFilter* filter) {
    //mPaint.setColorFilter(filter);
}

void CircularProgressDrawable::Ring::setAlpha(int alpha) {
    mAlpha = alpha;
}

int CircularProgressDrawable::Ring::getAlpha() const{
    return mAlpha;
}

void CircularProgressDrawable::Ring::setStrokeWidth(float strokeWidth) {
    mStrokeWidth = strokeWidth;
    //mPaint.setStrokeWidth(strokeWidth);
}

float CircularProgressDrawable::Ring::getStrokeWidth() const{
    return mStrokeWidth;
}

void CircularProgressDrawable::Ring::setStartTrim(float startTrim) {
    mStartTrim = startTrim;
}

float CircularProgressDrawable::Ring::getStartTrim() const{
    return mStartTrim;
}

float CircularProgressDrawable::Ring::getStartingStartTrim() const{
    return mStartingStartTrim;
}

float CircularProgressDrawable::Ring::getStartingEndTrim() const{
    return mStartingEndTrim;
}

int CircularProgressDrawable::Ring::getStartingColor() const{
    return mColors[mColorIndex];
}

void CircularProgressDrawable::Ring::setEndTrim(float endTrim) {
    mEndTrim = endTrim;
}

float CircularProgressDrawable::Ring::getEndTrim() const{
    return mEndTrim;
}

void CircularProgressDrawable::Ring::setRotation(float rotation) {
    mRotation = rotation;
}

float CircularProgressDrawable::Ring::getRotation() const{
    return mRotation;
}

void CircularProgressDrawable::Ring::setCenterRadius(float centerRadius) {
    mRingCenterRadius = centerRadius;
}

float CircularProgressDrawable::Ring::getCenterRadius() const{
    return mRingCenterRadius;
}

void CircularProgressDrawable::Ring::setShowArrow(bool show) {
    if (mShowArrow != show) {
        mShowArrow = show;
    }
}

bool CircularProgressDrawable::Ring::getShowArrow() const{
    return mShowArrow;
}

void CircularProgressDrawable::Ring::setArrowScale(float scale) {
    if (scale != mArrowScale) {
        mArrowScale = scale;
    }
}

float CircularProgressDrawable::Ring::getArrowScale() const{
    return mArrowScale;
}

float CircularProgressDrawable::Ring::getStartingRotation() const{
    return mStartingRotation;
}

void CircularProgressDrawable::Ring::storeOriginals() {
    mStartingStartTrim = mStartTrim;
    mStartingEndTrim = mEndTrim;
    mStartingRotation = mRotation;
}

void CircularProgressDrawable::Ring::resetOriginals() {
    mStartingStartTrim = 0;
    mStartingEndTrim = 0;
    mStartingRotation = 0;
    setStartTrim(0);
    setEndTrim(0);
    setRotation(0);
}
}/*endofnamespace*/
