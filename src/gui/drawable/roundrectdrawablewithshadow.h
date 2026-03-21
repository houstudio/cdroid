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
#ifndef __ROUNDRECT_DRAWABLE_WIDTH_SHADOW_H__
#define __ROUNDRECT_DRAWABLE_WIDTH_SHADOW_H__
#include <drawable/drawable.h>
namespace cdroid{

class RoundRectDrawableWithShadow:public Drawable {
    struct RoundRectHelper {
        virtual void drawRoundRect(Canvas& canvas,const RectF& bounds, float cornerRadius/*,Paint paint*/)=0;
    };
private:
    static constexpr float SHADOW_MULTIPLIER = 1.5f;

    int mInsetShadow; // extra shadow to avoid gaps between card and shadow

    /*
    * This helper is set by CardView implementations.
    * <p>
    * Prior to API 17, canvas.drawRoundRect is expensive; which is why we need this interface
    * to draw efficient rounded rectangles before 17.
    * */
    static RoundRectHelper* sRoundRectHelper;

    Cairo::RefPtr<Cairo::LinearGradient>mEdgeShadowPattern;
    Cairo::RefPtr<Cairo::RadialGradient>mCornerShadowPattern;

    RectF mCardBounds;

    float mCornerRadius;

    cdroid::Path* mCornerShadowPath;

    // actual value set by developer
    float mRawMaxShadowSize;
    // multiplied value to account for shadow offset
    float mShadowSize;

    // actual value set by developer
    float mRawShadowSize;

    cdroid::RefPtr<ColorStateList> mBackground;

    bool mDirty = true;
    bool mAddPaddingForCorners = true;
    bool mPrintedShadowClipWarning = false;

    int mShadowStartColor;
    int mShadowEndColor;
    int mStateColor;
    int mAlpha;
private:
    void setBackground(const cdroid::RefPtr<ColorStateList>& color);
    void setShadowSize(float shadowSize, float maxShadowSize);
    void drawShadow(Canvas& canvas);
    void buildShadowCorners();
    void buildComponents(const Rect& bounds);
protected:
    void onBoundsChange(const Rect& bounds) override;
    bool onStateChange(const std::vector<int>& stateSet) override;
public:
    RoundRectDrawableWithShadow(Context*,const cdroid::RefPtr<ColorStateList>& backgroundColor, float radius,
            float shadowSize, float maxShadowSize);
    ~RoundRectDrawableWithShadow()override;
    void setAddPaddingForCorners(bool addPaddingForCorners);
    void setAlpha(int alpha) override;
    int getAlpha()const override;

    bool getPadding(Rect& padding)override;

    static float calculateVerticalPadding(float maxShadowSize, float cornerRadius, bool addPaddingForCorners);
    static float calculateHorizontalPadding(float maxShadowSize, float cornerRadius, bool addPaddingForCorners);

    bool isStateful() const override;
    void setColorFilter(const cdroid::RefPtr<ColorFilter>& cf) override;
    int getOpacity() override;

    float getCornerRadius() const;
    void setCornerRadius(float radius);

    void draw(Canvas& canvas) override;

    void getMaxShadowAndCornerPadding(Rect& into);

    void setShadowSize(float size);
    void setMaxShadowSize(float size);

    float getShadowSize() const;
    float getMaxShadowSize() const;

    float getMinWidth() const;
    float getMinHeight() const;

    void setColor(const cdroid::RefPtr<ColorStateList>& color);

    const cdroid::RefPtr<ColorStateList> getColor() const;
};
}/*endof namespace*/
#endif/*__ROUNDRECT_DRAWABLE_WIDTH_SHADOW_H__*/
