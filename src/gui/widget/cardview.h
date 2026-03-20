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
#ifndef __CARD_VIEW_H__
#define __CARD_VIEW_H__
#include <widget/framelayout.h>
#include <widget/cardviewimpl.h>

namespace cdroid{
class CardView :public FrameLayout {
private:
    class CardViewDelegateInternal;
    //static final int[] COLOR_BACKGROUND_ATTR = {android.R.attr.colorBackground};
    static CardViewImpl* IMPL;

    bool mCompatPadding;
    bool mPreventCornerOverlap;

    int mUserSetMinWidth, mUserSetMinHeight;
    CardViewDelegate* mCardViewDelegate;

    Rect mContentPadding;
    Rect mShadowBounds;
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
public:
    CardView(int w,int h);
    CardView(Context* context,const AttributeSet& attrs);
    ~CardView()override;
    void setPadding(int left, int top, int right, int bottom)override;
    void setPaddingRelative(int start, int top, int end, int bottom)override;

    bool getUseCompatPadding() const;
    void setUseCompatPadding(bool useCompatPadding);

    void setContentPadding(int left, int top, int right, int bottom);

    void setMinimumWidth(int minWidth) override;
    void setMinimumHeight(int minHeight) override;

    void setCardBackgroundColor(int color);
    void setCardBackgroundColor(const RefPtr<ColorStateList>& color);
    const RefPtr<ColorStateList> getCardBackgroundColor() const;

    int getContentPaddingLeft() const;
    int getContentPaddingRight() const;
    int getContentPaddingTop() const;
    int getContentPaddingBottom() const;

    void setRadius(float radius);
    float getRadius() const;

    void setCardElevation(float elevation);
    float getCardElevation()const;
    void setElevation(float elevation)override;
    float getElevation()const override;

    void setMaxCardElevation(float maxElevation);
    float getMaxCardElevation() const;

    bool getPreventCornerOverlap() const;
    void setPreventCornerOverlap(bool preventCornerOverlap);
};
}/*endof namespace*/
#endif/*__CARD_VIEW_H__*/
