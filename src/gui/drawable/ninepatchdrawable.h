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
#ifndef __NINEPATCH_DRAWABLE_H__
#define __NINEPATCH_DRAWABLE_H__
#include <drawable/drawable.h>
#include <content/typedarray.h>

namespace cdroid{
class NinePatchRenderer;
class NinePatchDrawable:public Drawable{
private:
    class NinePatchState:public std::enable_shared_from_this<NinePatchState>,public ConstantState{
    public:
        // AOSP mThemeAttrs: ?attr ids captured at inflate, re-resolved by applyTheme.
        std::vector<int> mThemeAttrs;
        float mBaseAlpha;//= 1.0f;
        bool mDither;//=DEFAULT_DITHER;
        bool mAutoMirrored;//= false;
        Rect mPadding;
        Insets mOpticalInsets;
        int mTintMode;
        int mChangingConfigurations;
        // Density of the renderer's current pixel space (the asset's source
        // density at decode; becomes the target density once the decode-time
        // resample has run — AOSP keeps the equivalent in Bitmap.mDensity) and
        // the density numbers should render at (AOSP NinePatchState.mTargetDensity).
        int mSourceDensity;
        int mTargetDensity;
        cdroid::RefPtr<ColorStateList>mTint;
        Cairo::RefPtr<NinePatchRenderer>mNinePatch;
        NinePatchState();
        NinePatchState(const NinePatchState&state);
        void setBitmap(Cairo::RefPtr<Cairo::ImageSurface>bitmap,const Rect*padding=nullptr,
                       const std::vector<uint8_t>*ninePatchChunk=nullptr);
        NinePatchDrawable*newDrawable()override;
        void draw(Canvas&canvas,const Rect&rect,int alpha);
        int getChangingConfigurations()const override;
    };
    int mAlpha;
    int mBitmapWidth;
    int mBitmapHeight;
    int mTargetDensity;
    float mOutlineRadius;
    Insets mOpticalInsets;
    Rect mPadding;
    bool mMutated;
    bool mFilterBitmap;
    bool needsMirroring();
    void computeBitmapSize();
    std::shared_ptr<NinePatchState>mNinePatchState;
    cdroid::RefPtr<PorterDuffColorFilter>mTintFilter;
    NinePatchDrawable(std::shared_ptr<NinePatchState>state);
    void updateStateFromTypedArray(const TypedArray& a);
protected:
    bool onStateChange(const std::vector<int>& stateSet)override;
public:
    NinePatchDrawable();
    NinePatchDrawable(Cairo::RefPtr<Cairo::ImageSurface>bmp,const std::vector<uint8_t>*ninePatchChunk=nullptr);
    ~NinePatchDrawable();
    void setTargetDensity(int density);
    // Decode-seam density fixup (AOSP folds this into BitmapFactory.decodeResourceStream):
    // records the asset's source density so the decode site can resample the
    // bitmap + chunk into the display's pixel space once, up front.
    void setSourceDensity(int density);
    Insets getOpticalInsets()override;
    void setAlpha(int alpha)override;
    bool getPadding(Rect& padding) override;
    void getOutline(Outline&)override;
    int getAlpha()const override;
    void setTintList(const cdroid::RefPtr<ColorStateList>& tint)override;
    void setTintMode(int mode)override;
    void setAutoMirrored(bool mirrored)override;
    bool isAutoMirrored()const override;
    void setFilterBitmap(bool filter)override;
    bool isFilterBitmap()const override;
    int getIntrinsicWidth() override;
    int getIntrinsicHeight() override;
    NinePatchDrawable*mutate()override;
    bool isStateful()const override;
    bool hasFocusStateSpecified()const override;
    std::shared_ptr<ConstantState>getConstantState()override;
    void draw(Canvas&canvas)override;
    void inflate(Resources&r,XmlPullParser&parser,const AttributeSet&atts, const Resources::Theme* theme)override;
    bool canApplyTheme()override;
    void applyTheme(const Resources::Theme& t)override;
};
}
#endif
