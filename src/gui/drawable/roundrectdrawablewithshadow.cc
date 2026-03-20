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
 *********************************************************************************/
#include <drawable/gradientdrawable.h>
#include <drawable/roundrectdrawablewithshadow.h>
namespace cdroid{

RoundRectDrawableWithShadow::RoundRectDrawableWithShadow(Context*context,const RefPtr<ColorStateList>& backgroundColor, float radius,
        float shadowSize, float maxShadowSize) {
    mShadowStartColor = 0x37000000;//resources.getColor(R.color.cardview_shadow_start_color);
    mShadowEndColor   = 0x03000000;//resources.getColor(R.color.cardview_shadow_end_color);
    mInsetShadow = 1;//resources.getDimensionPixelSize(R.dimen.cardview_compat_inset_shadow);
    //mPaint = new Paint(Paint.ANTI_ALIAS_FLAG | Paint.DITHER_FLAG);
    setBackground(backgroundColor);
    mCornerRadius = (int) (radius + 0.5f);
    setShadowSize(shadowSize, maxShadowSize);
    mCornerShadowPath = nullptr;
}

void RoundRectDrawableWithShadow::setBackground(const RefPtr<ColorStateList>& color) {
    mBackground = (color == nullptr) ?  ColorStateList::valueOf(Color::TRANSPARENT) : color;
    mStateColor = mBackground->getColorForState(getState(), mBackground->getDefaultColor());
}

/**
 * Casts the value to an even integer.
 */
static int toEven(float value) {
    int i = (int) (value + 0.5f);
    if (i % 2 == 1) {
        return i - 1;
    }
    return i;
}

void RoundRectDrawableWithShadow::setAddPaddingForCorners(bool addPaddingForCorners) {
    mAddPaddingForCorners = addPaddingForCorners;
    invalidateSelf();
}

void RoundRectDrawableWithShadow::setAlpha(int alpha) {
    //mPaint.setAlpha(alpha);
    //mCornerShadowPaint.setAlpha(alpha);
    //mEdgeShadowPaint.setAlpha(alpha);
    mAlpha = alpha;
}

int RoundRectDrawableWithShadow::getAlpha()const{
    return mAlpha;
}

void RoundRectDrawableWithShadow::onBoundsChange(const Rect& bounds) {
    Drawable::onBoundsChange(bounds);
    mDirty = true;
}

void RoundRectDrawableWithShadow::setShadowSize(float shadowSize, float maxShadowSize) {
    if (shadowSize < 0.f) {
        FATAL("Invalid shadow size %2.f Must be >= 0",shadowSize);
    }
    if (maxShadowSize < 0.f) {
        FATAL("Invalid max shadow size %2.f Must be >= 0",maxShadowSize);
    }
    shadowSize = toEven(shadowSize);
    maxShadowSize = toEven(maxShadowSize);
    if (shadowSize > maxShadowSize) {
        shadowSize = maxShadowSize;
        if (!mPrintedShadowClipWarning) {
            mPrintedShadowClipWarning = true;
        }
    }
    if (mRawShadowSize == shadowSize && mRawMaxShadowSize == maxShadowSize) {
        return;
    }
    mRawShadowSize = shadowSize;
    mRawMaxShadowSize = maxShadowSize;
    mShadowSize = (int) (shadowSize * SHADOW_MULTIPLIER + mInsetShadow + 0.5f);
    mDirty = true;
    invalidateSelf();
}

bool RoundRectDrawableWithShadow::getPadding(Rect& padding) {
    const int vOffset = (int) std::ceil(calculateVerticalPadding(mRawMaxShadowSize, mCornerRadius, mAddPaddingForCorners));
    const int hOffset = (int) std::ceil(calculateHorizontalPadding(mRawMaxShadowSize, mCornerRadius, mAddPaddingForCorners));
    padding.set(hOffset, vOffset, hOffset, vOffset);
    return true;
}

static constexpr double COS_45 = 0.7071067811865475244008443621048490392848359376884740;//Math.cos(Math.toRadians(45));
float RoundRectDrawableWithShadow::calculateVerticalPadding(float maxShadowSize, float cornerRadius, bool addPaddingForCorners) {
    if (addPaddingForCorners) {
        return (float) (maxShadowSize * SHADOW_MULTIPLIER + (1.0 - COS_45) * cornerRadius);
    } else {
        return maxShadowSize * SHADOW_MULTIPLIER;
    }
}

float RoundRectDrawableWithShadow::calculateHorizontalPadding(float maxShadowSize, float cornerRadius, bool addPaddingForCorners) {
    if (addPaddingForCorners) {
        return (float) (maxShadowSize + (1.0 - COS_45) * cornerRadius);
    } else {
        return maxShadowSize;
    }
}

bool RoundRectDrawableWithShadow::onStateChange(const std::vector<int>& stateSet) {
    const int newColor = mBackground->getColorForState(stateSet, mBackground->getDefaultColor());
    if (mStateColor == newColor) {
        return false;
    }
    mStateColor = newColor;
    mDirty = true;
    invalidateSelf();
    return true;
}

bool RoundRectDrawableWithShadow::isStateful() const{
    return (mBackground != nullptr && mBackground->isStateful()) || Drawable::isStateful();
}

void RoundRectDrawableWithShadow::setColorFilter(const cdroid::RefPtr<ColorFilter>& cf) {
    //mPaint.setColorFilter(cf);
}

int RoundRectDrawableWithShadow::getOpacity() {
    return PixelFormat::TRANSLUCENT;
}

void RoundRectDrawableWithShadow::setCornerRadius(float radius) {
    if (radius < 0.f) {
        FATAL("Invalid radius %2.f Must be >= 0",radius);
    }
    radius = (int) (radius + 0.5f);
    if (mCornerRadius == radius) {
        return;
    }
    mCornerRadius = radius;
    mDirty = true;
    invalidateSelf();
}

void RoundRectDrawableWithShadow::draw(Canvas& canvas) {
    if (mDirty) {
        buildComponents(getBounds());
        mDirty = false;
    }
    canvas.translate(0, mRawShadowSize / 2);
    drawShadow(canvas);
    canvas.translate(0, -mRawShadowSize / 2);
    //sRoundRectHelper->drawRoundRect(canvas, mCardBounds, mCornerRadius, mPaint);
    canvas.set_color(mStateColor);
    GradientDrawable::drawRoundedRect(canvas,mCardBounds,mCornerRadius,mCornerRadius,mCornerRadius,mCornerRadius);
    canvas.fill();
}

void RoundRectDrawableWithShadow::drawShadow(Canvas& canvas) {
    const float edgeShadowTop = -mCornerRadius - mShadowSize;
    const float inset = mCornerRadius + mInsetShadow + mRawShadowSize / 2;
    const bool drawHorizontalEdges = mCardBounds.width - 2 * inset > 0;
    const bool drawVerticalEdges = mCardBounds.height - 2 * inset > 0;
    // LT
    canvas.save();
    canvas.translate(mCardBounds.left + inset, mCardBounds.top + inset);
    //canvas.drawPath(mCornerShadowPath, mCornerShadowPaint);
    mCornerShadowPath->append_to_context(&canvas);
    canvas.set_source(mCornerShadowPattern);
    canvas.fill();
    if (drawHorizontalEdges) {
        canvas.set_antialias(Cairo::Antialias::ANTIALIAS_NONE);
        canvas.rectangle(0, edgeShadowTop, mCardBounds.width - 2 * inset, -mCornerRadius - edgeShadowTop);
        canvas.set_source(mEdgeShadowPattern);
        canvas.fill();
    }
    canvas.restore();
    // RB
    canvas.save();
    canvas.translate(mCardBounds.right() - inset, mCardBounds.bottom() - inset);
    canvas.rotate_degrees(180.f);
    //canvas.drawPath(mCornerShadowPath, mCornerShadowPaint);
    mCornerShadowPath->append_to_context(&canvas);
    canvas.set_source(mCornerShadowPattern);
    canvas.fill();
    if (drawHorizontalEdges) {
        canvas.set_antialias(Cairo::Antialias::ANTIALIAS_NONE);
        canvas.rectangle(0, edgeShadowTop, mCardBounds.width - 2 * inset, -mCornerRadius + mShadowSize -edgeShadowTop);
        canvas.set_source(mEdgeShadowPattern);
        canvas.fill();
    }
    canvas.restore();
    // LB
    canvas.save();
    canvas.translate(mCardBounds.left + inset, mCardBounds.bottom() - inset);
    canvas.rotate_degrees(270.f);
    //canvas.drawPath(mCornerShadowPath, mCornerShadowPaint);
    mCornerShadowPath->append_to_context(&canvas);
    canvas.set_source(mCornerShadowPattern);
    canvas.fill();
    if (drawVerticalEdges) {
        canvas.set_antialias(Cairo::Antialias::ANTIALIAS_NONE);
        canvas.rectangle(0, edgeShadowTop, mCardBounds.height - 2 * inset, -mCornerRadius-edgeShadowTop);
        canvas.set_source(mEdgeShadowPattern);
        canvas.fill();
    }
    canvas.restore();
    // RT
    canvas.save();
    canvas.translate(mCardBounds.right() - inset, mCardBounds.top + inset);
    canvas.rotate_degrees(90.f);
    //canvas.drawPath(mCornerShadowPath, mCornerShadowPaint);
    mCornerShadowPath->append_to_context(&canvas);
    canvas.set_source(mCornerShadowPattern);
    canvas.fill();
    if (drawVerticalEdges) {
        canvas.set_antialias(Cairo::Antialias::ANTIALIAS_NONE);
        canvas.rectangle(0, edgeShadowTop, mCardBounds.height - 2 * inset, -mCornerRadius-edgeShadowTop);
        canvas.set_source(mEdgeShadowPattern);
        canvas.fill();
    }
    canvas.restore();
}

void RoundRectDrawableWithShadow::buildShadowCorners() {
    RectF innerBounds = {-mCornerRadius, -mCornerRadius, 2.f*mCornerRadius, 2.f*mCornerRadius};
    RectF outerBounds = innerBounds;
    outerBounds.inset(-mShadowSize, -mShadowSize);

    if (mCornerShadowPath == nullptr) {
        mCornerShadowPath = new Path();
    } else {
        mCornerShadowPath->reset();
    }
    mCornerShadowPath->set_fill_rule(Cairo::Context::FillRule::EVEN_ODD);//setFillType(Path.FillType.EVEN_ODD);
    mCornerShadowPath->move_to(-mCornerRadius, 0);
    mCornerShadowPath->rel_line_to(-mShadowSize, 0);/*rLineTo*/
    // outer arc
    mCornerShadowPath->arc_to(outerBounds, 180.f, 90.f, false);
    // inner arc
    mCornerShadowPath->arc_to(innerBounds, 270.f, -90.f, false);
    mCornerShadowPath->close_path();
    const float startRatio = mCornerRadius / (mCornerRadius + mShadowSize);
    const Color cs(mShadowStartColor);
    const Color ce(mShadowEndColor);
    mCornerShadowPattern = Cairo::RadialGradient::create(0,0,0,0, 0, mCornerRadius + mShadowSize);
    mCornerShadowPattern->add_color_stop_rgba(0.0f,cs.red(),cs.green(),cs.blue(),cs.alpha());
    mCornerShadowPattern->add_color_stop_rgba(startRatio,cs.red(),cs.green(),cs.blue(),cs.alpha());
    mCornerShadowPattern->add_color_stop_rgba(1.0f,ce.red(),ce.green(),ce.blue(),ce.alpha());

    // we offset the content shadowSize/2 pixels up to make it more realistic.
    // this is why edge shadow shader has some extra space
    // When drawing bottom edge shadow, we use that extra space.
    mEdgeShadowPattern = Cairo::LinearGradient::create(0, -mCornerRadius + mShadowSize, 0, -mCornerRadius - mShadowSize);
    mEdgeShadowPattern->add_color_stop_rgba(0.0f,cs.red(),cs.green(),cs.blue(),cs.alpha());
    mEdgeShadowPattern->add_color_stop_rgba(0.5f,cs.red(),cs.green(),cs.blue(),cs.alpha());
    mEdgeShadowPattern->add_color_stop_rgba(1.0f,ce.red(),ce.green(),ce.blue(),ce.alpha());
}

void RoundRectDrawableWithShadow::buildComponents(const Rect& bounds) {
    // Card is offset SHADOW_MULTIPLIER * maxShadowSize to account for the shadow shift.
    // We could have different top-bottom offsets to avoid extra gap above but in that case
    // center aligning Views inside the CardView would be problematic.
    const float verticalOffset = mRawMaxShadowSize * SHADOW_MULTIPLIER;
    mCardBounds.set(bounds.left + mRawMaxShadowSize, bounds.top + verticalOffset,
            bounds.width - 2*mRawMaxShadowSize, bounds.height - 2*verticalOffset);
    buildShadowCorners();
}

float RoundRectDrawableWithShadow::getCornerRadius() const{
    return mCornerRadius;
}

void RoundRectDrawableWithShadow::getMaxShadowAndCornerPadding(Rect& into) {
    getPadding(into);
}

void RoundRectDrawableWithShadow::setShadowSize(float size) {
    setShadowSize(size, mRawMaxShadowSize);
}

void RoundRectDrawableWithShadow::setMaxShadowSize(float size) {
    setShadowSize(mRawShadowSize, size);
}

float RoundRectDrawableWithShadow::getShadowSize() const{
    return mRawShadowSize;
}

float RoundRectDrawableWithShadow::getMaxShadowSize() const{
    return mRawMaxShadowSize;
}

float RoundRectDrawableWithShadow::getMinWidth() const{
    const float content = 2
            * std::max(mRawMaxShadowSize, mCornerRadius + mInsetShadow + mRawMaxShadowSize / 2);
    return content + (mRawMaxShadowSize + mInsetShadow) * 2;
}

float RoundRectDrawableWithShadow::getMinHeight() const{
    const float content = 2 * std::max(mRawMaxShadowSize, mCornerRadius + mInsetShadow
                    + mRawMaxShadowSize * SHADOW_MULTIPLIER / 2);
    return content + (mRawMaxShadowSize * SHADOW_MULTIPLIER + mInsetShadow) * 2;
}

void RoundRectDrawableWithShadow::setColor(const cdroid::RefPtr<ColorStateList>& color) {
    setBackground(color);
    invalidateSelf();
}

const cdroid::RefPtr<ColorStateList> RoundRectDrawableWithShadow::getColor() const{
    return mBackground;
}
}
