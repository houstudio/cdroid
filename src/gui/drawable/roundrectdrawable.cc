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
#include <drawable/roundrectdrawable.h>
#include <drawable/gradientdrawable.h>
#include <drawable/roundrectdrawablewithshadow.h>
namespace cdroid{
RoundRectDrawable::RoundRectDrawable(const RefPtr<ColorStateList>& backgroundColor, float radius) {
    mRadius = radius;
    //mPaint = new Paint(Paint.ANTI_ALIAS_FLAG | Paint.DITHER_FLAG);
    setBackground(backgroundColor);
}

RoundRectDrawable::~RoundRectDrawable(){
}

void RoundRectDrawable::setBackground(const RefPtr<ColorStateList>& color) {
    mBackground = (color == nullptr) ?  ColorStateList::valueOf(Color::TRANSPARENT) : color;
    mStateColor = mBackground->getColorForState(getState(), mBackground->getDefaultColor());
}

void RoundRectDrawable::setPadding(float padding, bool insetForPadding, bool insetForRadius) {
    if (padding == mPadding && mInsetForPadding == insetForPadding
            && mInsetForRadius == insetForRadius) {
        return;
    }
    mPadding = padding;
    mInsetForPadding = insetForPadding;
    mInsetForRadius = insetForRadius;
    updateBounds({});
    invalidateSelf();
}

float RoundRectDrawable::getPadding() const{
    return mPadding;
}

void RoundRectDrawable::draw(Canvas& canvas) {
    canvas.set_color(mStateColor);
    GradientDrawable::drawRoundedRect(canvas,mBoundsF,mRadius,mRadius,mRadius,mRadius);
    canvas.fill();
}

void RoundRectDrawable::updateBounds(const Rect& bounds) {
    Rect b = bounds;
    if (bounds.empty()) {
        b = getBounds();
    }
    mBoundsF.set(b.left, b.top, b.width, b.height);
    mBoundsI = b;
    if (mInsetForPadding) {
        const float vInset = RoundRectDrawableWithShadow::calculateVerticalPadding(mPadding, mRadius, mInsetForRadius);
        const float hInset = RoundRectDrawableWithShadow::calculateHorizontalPadding(mPadding, mRadius, mInsetForRadius);
        mBoundsI.inset((int) std::ceil(hInset), (int) std::ceil(vInset));
        // to make sure they have same bounds.
        mBoundsF.set(mBoundsI.left,mBoundsI.top,mBoundsI.width,mBoundsI.height);
    }
}

void RoundRectDrawable::onBoundsChange(const Rect& bounds) {
    Drawable::onBoundsChange(bounds);
    updateBounds(bounds);
}

void RoundRectDrawable::getOutline(Outline& outline) {
    outline.setRoundRect(mBoundsI, mRadius);
}

void RoundRectDrawable::setRadius(float radius) {
    if (radius == mRadius) {
        return;
    }
    mRadius = radius;
    updateBounds({});
    invalidateSelf();
}

void RoundRectDrawable::setAlpha(int alpha) {
    //mPaint.setAlpha(alpha);
    mAlpha = alpha&0xFF;
}

int RoundRectDrawable::getAlpha()const{
    return mAlpha;
}

void RoundRectDrawable::setColorFilter(const cdroid::RefPtr<ColorFilter>& cf) {
    //mPaint.setColorFilter(cf);
}

int RoundRectDrawable::getOpacity() {
    return PixelFormat::TRANSLUCENT;
}

float RoundRectDrawable::getRadius() const{
    return mRadius;
}

void RoundRectDrawable::setColor(const cdroid::RefPtr<ColorStateList>& color) {
    setBackground(color);
    invalidateSelf();
}

const RefPtr<ColorStateList> RoundRectDrawable::getColor() const{
    return mBackground;
}

void RoundRectDrawable::setTintList(const cdroid::RefPtr<ColorStateList>& tint) {
    mTint = tint;
    mTintFilter = createTintFilter(mTint, mTintMode);
    invalidateSelf();
}

void RoundRectDrawable::setTintMode(int tintMode) {
    mTintMode = tintMode;
    mTintFilter = createTintFilter(mTint, mTintMode);
    invalidateSelf();
}

bool RoundRectDrawable::onStateChange(const std::vector<int>& stateSet) {
    const int newColor = mBackground->getColorForState(stateSet, mBackground->getDefaultColor());
    const bool colorChanged =(mStateColor!=newColor);
    if (colorChanged) {
        mStateColor = newColor;
    }
    if (mTint != nullptr && mTintMode <0) {
        mTintFilter = createTintFilter(mTint, mTintMode);
        return true;
    }
    return colorChanged;
}

bool RoundRectDrawable::isStateful() const {
    return (mTint != nullptr && mTint->isStateful())
            || (mBackground != nullptr && mBackground->isStateful()) || Drawable::isStateful();
}

cdroid::RefPtr<PorterDuffColorFilter> RoundRectDrawable::createTintFilter(const RefPtr<ColorStateList>& tint, int tintMode) {
    if (tint == nullptr || tintMode <0) {
        return nullptr;
    }
    const int color = tint->getColorForState(getState(), Color::TRANSPARENT);
    return std::make_shared<PorterDuffColorFilter>(color, tintMode);
}
}
