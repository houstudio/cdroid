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
#ifndef __ROUNDED_DRAWABLE_H__
#define __ROUNDED_DRAWABLE_H__
#include <drawables/drawablewrapper.h>
namespace cdroid{
class RoundedDrawable:public Drawable {
private:
    int mBackgroundColor;
    int mRadius;// Radius applied to corners in pixels
    int mAlpha;
    Drawable* mDrawable;
    bool mIsClipEnabled;

    // Used to avoid creating new Rect objects every time draw() is called
    Rect mTmpBounds;
    RectF mTmpBoundsF;
protected:
    void onBoundsChange(const Rect& bounds)override;
public:
    RoundedDrawable();
    ~RoundedDrawable()override;
    void inflate(XmlPullParser& parser,const AttributeSet& attrs)override;

    void setDrawable(Drawable* drawable);
    Drawable* getDrawable();

    void setBackgroundColor(int color);
    int getBackgroundColor()const;

    /**
     * Sets whether the drawable inside should be clipped or resized to fit the rounded bounds. If
     * the drawable is animated, don't set clipping to {@code true} as clipping on animated
     * drawables is not supported.
     *
     * @param clipEnabled {@code true} if the drawable should be clipped, {@code false} if it
     *                    should be resized.
     * {@link androidx.wear.R.attr#clipEnabled}
     */
    void setClipEnabled(bool clipEnabled);
    bool isClipEnabled() const;

    void draw(Canvas& canvas) override;

    int getOpacity()override;

    void setAlpha(int alpha);
    int getAlpha()const override;

    void setColorFilter(ColorFilter* cf)override;

    /**
     * Sets the border radius to be applied when rendering the drawable in pixels.
     *
     * @param radius radius in pixels
     * {@link androidx.wear.R.attr#radius}
     */
    void setRadius(int radius);
    int getRadius() const;
};
}/*endof namespace*/
#endif/*__ROUNDED_DRAWABLE_H__*/
