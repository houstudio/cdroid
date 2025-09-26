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
#include <widgetEx/wear/roundeddrawable.h>
#include <drawables/gradientdrawable.h>
namespace cdroid{

RoundedDrawable::RoundedDrawable() {
    mTmpBounds.setEmpty();
    mTmpBoundsF.setEmpty();
    mAlpha = 0xFF;
    mBackgroundColor = Color::TRANSPARENT;
}

void RoundedDrawable::inflate(XmlPullParser& parser, const AttributeSet& attrs/*, Theme theme*/){
    Drawable::inflate(parser, attrs/*, theme*/);
    setRadius(attrs.getDimensionPixelSize("radius", 0));
    setClipEnabled(attrs.getBoolean("clipEnabled", false));
    setBackgroundColor(attrs.getColor("backgroundColor", Color::TRANSPARENT));
}

void RoundedDrawable::setDrawable(Drawable* drawable) {
    if (mDrawable==drawable) {
        return;
    }
    mDrawable = drawable;
    invalidateSelf();
}

Drawable* RoundedDrawable::getDrawable() {
    return mDrawable;
}

void RoundedDrawable::setBackgroundColor(int color) {
    mBackgroundColor = color;
    invalidateSelf();
}

int RoundedDrawable::getBackgroundColor() const{
    return mBackgroundColor;
}

void RoundedDrawable::setClipEnabled(bool clipEnabled) {
    if(mIsClipEnabled!=clipEnabled){
        mIsClipEnabled = clipEnabled;
        invalidateSelf();
    }
}

bool RoundedDrawable::isClipEnabled() const{
    return mIsClipEnabled;
}

void RoundedDrawable::onBoundsChange(const Rect& bounds) {
    mTmpBounds.width = bounds.width;
    mTmpBounds.height = bounds.height;
    mTmpBoundsF.width = bounds.width;
    mTmpBoundsF.height = bounds.height;
}

void RoundedDrawable::draw(Canvas& canvas) {
    Rect bounds = getBounds();
    if (mDrawable == nullptr || bounds.empty()) {
        return;
    }
    canvas.save();
    canvas.translate(bounds.left, bounds.top);
    // mTmpBoundsF is bounds translated to (0,0) and converted to RectF as drawRoundRect
    // requires.
    //canvas.drawRoundRect(mTmpBoundsF, (float) mRadius, (float) mRadius,mBackgroundPaint);
    GradientDrawable::drawRoundedRect(canvas,mTmpBoundsF,mRadius,mRadius,mRadius,mRadius);
    if (mIsClipEnabled) {
        // Clip to a rounded rectangle
        //canvas.drawRoundRect(mTmpBoundsF, (float) mRadius, (float) mRadius, mPaint);
        canvas.clip_preserve();
        if(mBackgroundColor!=Color::TRANSPARENT){
            canvas.set_color(mBackgroundColor);
            canvas.fill();
        }
        mDrawable->setBounds(mTmpBounds);
        mDrawable->draw(canvas);
    } else {
        // Scale to fit the rounded rectangle
        const int minEdge = std::min(bounds.width, bounds.height);
        const int padding = (int) std::ceil(std::min(mRadius, minEdge) * (1.f - 1.f / (float) std::sqrt(2.0)));
        mTmpBounds.inset(padding, padding);
        mDrawable->setBounds(mTmpBounds);
        mDrawable->draw(canvas);
        mTmpBounds.inset(-padding, -padding);
    }
    canvas.restore();
}

int RoundedDrawable::getOpacity() {
    return PixelFormat::TRANSLUCENT;
}

void RoundedDrawable::setAlpha(int alpha) {
    mAlpha = alpha;
}

int RoundedDrawable::getAlpha() const{
    return mAlpha;//mPaint.getAlpha();
}

void RoundedDrawable::setColorFilter(ColorFilter* cf) {
    //mPaint.setColorFilter(cf);
}

void RoundedDrawable::setRadius(int radius) {
    mRadius = radius;
}

int RoundedDrawable::getRadius() const{
    return mRadius;
}

}/*endof namespace*/
