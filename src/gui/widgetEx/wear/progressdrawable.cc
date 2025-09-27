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
#include <animation/interpolators.h>
#include <animation/objectanimator.h>
#include <widgetEx/wear/progressdrawable.h>

namespace cdroid{
namespace {
    class PDLEVEL:public Property{
    public:
        PDLEVEL():Property("level",INT_TYPE){}
        AnimateValue get(void*drawable)const override{
            return ((ProgressDrawable*)drawable)->getLevel();
        }
        void set(void*drawable,const AnimateValue& value)const override {
            ((ProgressDrawable*)drawable)->setLevel(GET_VARIANT(value,int));
            ((ProgressDrawable*)drawable)->invalidateSelf();
        }
    };
    class PDLEVEL LEVEL;
}

ProgressDrawable::ProgressDrawable() {
    mInnerCircleBounds.setEmpty();
    mAnimator = ObjectAnimator::ofInt(this, &LEVEL,{0, MAX_LEVEL});
    mAnimator->setRepeatCount(ValueAnimator::INFINITE);
    mAnimator->setRepeatMode(ValueAnimator::RESTART);
    mAnimator->setDuration(ANIMATION_DURATION);
    mAnimator->setInterpolator(LinearInterpolator::gLinearInterpolator.get());
}

ProgressDrawable::~ProgressDrawable(){
    delete mAnimator;
}

float ProgressDrawable::lerpInv(float a, float b, float value) {
    return a != b ? ((value - a) / (b - a)) : 0.0f;
}

void ProgressDrawable::setRingColor(int color) {
    mCircleBorderColor = color;
}

void ProgressDrawable::setRingWidth(float width) {
    mCircleBorderWidth = width;
}

void ProgressDrawable::startAnimation() {
    if (!mAnimator->isStarted()) {
        mAnimator->start();
    }
}

void ProgressDrawable::stopAnimation() {
    mAnimator->cancel();
}

void ProgressDrawable::draw(Canvas& canvas) {
    canvas.save();
    mInnerCircleBounds.set(mBounds.left,mBounds.top,mBounds.width,mBounds.height);
    mInnerCircleBounds.inset(mCircleBorderWidth / 2.0f, mCircleBorderWidth / 2.0f);

    canvas.set_line_width(mCircleBorderWidth);
    canvas.set_color(mCircleBorderColor);

    const int level = getLevel();
    const int currentSegment = level / LEVELS_PER_SEGMENT;
    const int offset = currentSegment * LEVELS_PER_SEGMENT;
    const float progress = (level - offset) / (float) LEVELS_PER_SEGMENT;

    const bool growing = progress < GROW_SHRINK_RATIO;
    const float correctionAngle = CORRECTION_ANGLE * progress;

    float sweepAngle;
    auto sInterpolator = BezierSCurveInterpolator::gBezierSCurveInterpolator.get();
    if (growing) {
        sweepAngle = MAX_SWEEP
                * sInterpolator->getInterpolation(lerpInv(0.0f, GROW_SHRINK_RATIO, progress));
    } else {
        sweepAngle = MAX_SWEEP * (1.0f - sInterpolator->getInterpolation(
                        lerpInv(GROW_SHRINK_RATIO, 1.0f, progress)));
    }

    sweepAngle = std::max(1.0f, sweepAngle);

    //canvas.rotate( level * (1.0f / MAX_LEVEL) * 2 * FULL_CIRCLE + STARTING_ANGLE + correctionAngle,
    //        mInnerCircleBounds.centerX(), mInnerCircleBounds.centerY());
    //canvas.drawArc(mInnerCircleBounds, growing ? 0 : MAX_SWEEP - sweepAngle, sweepAngle, false,mPaint);
    canvas.restore();
}

void ProgressDrawable::setAlpha(int i) {
    // Not supported.
}

void ProgressDrawable::setColorFilter(ColorFilter* colorFilter) {
    // Not supported.
}

int ProgressDrawable::getOpacity() {
    return PixelFormat::OPAQUE;
}

bool ProgressDrawable::onLevelChange(int level) {
    return true; // Changing the level of this drawable does change its appearance.
}
}/*endof namesapce*/
