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
#ifndef __ROUNDRECT_DRAWABLE_H__
#define __ROUNDRECT_DRAWABLE_H__
#include <drawable/drawable.h>
namespace cdroid{
class RoundRectDrawable :public Drawable {
private:
    float mRadius;
    //Paint mPaint;
    RectF mBoundsF;
    Rect mBoundsI;
    float mPadding;
    bool mInsetForPadding = false;
    bool mInsetForRadius = true;

    cdroid::RefPtr<ColorStateList> mBackground;
    cdroid::RefPtr<PorterDuffColorFilter> mTintFilter;
    cdroid::RefPtr<ColorStateList> mTint;
    int mAlpha;
    int mStateColor;
    int mTintMode;// = PorterDuff.Mode.SRC_IN;
private:
    void setBackground(const RefPtr<ColorStateList>& color);
    void updateBounds(const Rect& bounds);
    cdroid::RefPtr<PorterDuffColorFilter> createTintFilter(const RefPtr<ColorStateList>& tint, int tintMode);
protected:
    void onBoundsChange(const Rect& bounds) override;
    bool onStateChange(const std::vector<int>& stateSet) override;
public:
    RoundRectDrawable(const RefPtr<ColorStateList>& backgroundColor, float radius);

    void setPadding(float padding, bool insetForPadding, bool insetForRadius);

    float getPadding() const;

    void draw(Canvas& canvas) override;

    void getOutline(Outline& outline) override;

    void setRadius(float radius);

    void setAlpha(int alpha) override;
    int getAlpha()const override;
    void setColorFilter(const cdroid::RefPtr<ColorFilter>& cf) override;

    int getOpacity() override;

    float getRadius()const;

    void setColor(const cdroid::RefPtr<ColorStateList>& color);

    const cdroid::RefPtr<ColorStateList> getColor() const;

    void setTintList(const cdroid::RefPtr<ColorStateList>& tint) override;
    void setTintMode(int tintMode) override;

    bool isStateful() const override;
};
}
#endif/*__ROUNDRECT_DRAWABLE_H__*/
