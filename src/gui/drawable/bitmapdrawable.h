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
#ifndef __BITMAP_DRAWABLE_H__
#define __BITMAP_DRAWABLE_H__
#include <drawable/drawable.h>
#include <content/typedarray.h>
#include <cairomm/surface.h>
#include <cairomm/refptr.h>
namespace cdroid{

enum TileMode{
    DISABLED=-1,
    CLAMP =0,
    REPEAT=1,
    MIRROR=2
};
class BitmapDrawable:public Drawable{
private:
    class BitmapState:public std::enable_shared_from_this<BitmapState>,public ConstantState{
    public:
        float mBaseAlpha;
        int mAlpha;
        int mGravity;
        int mTransparency;
        bool mAutoMirrored;
        bool mAntiAlias;
        bool mFilterBitmap;
        bool mDither;
        bool mMipMap;
        int mChangingConfigurations;
        std::vector<int>mThemeAttrs;
        cdroid::RefPtr<ColorStateList> mTint;
        int mTintMode;
        int mTileModeX;
        int mTileModeY;
        int mSrcDensityOverride;
        int mTargetDensity;
        /*Stand-in for Bitmap.mDensity: Cairo::ImageSurface carries no density
          metadata, so the density of the bucket the bitmap was decoded from
          lives here (0 = unknown → intrinsic sizes stay raw pixels, the
          pre-fix behavior; resource-loaded bitmaps get a real value).
          computeBitmapSize scales raw→target through it, like
          Bitmap.getScaledWidth(mTargetDensity).*/
        int mBitmapDensity;
        Cairo::RefPtr<Cairo::ImageSurface>mBitmap;
        /* Two-slot memo of the baked tinted copy of mBitmap (source space,
         * like AOSP's paint-side color filter). Value-keyed (color, mode) so a
         * pressed/normal tint cycle hits the other slot instead of re-baking
         * on every flip (updateTintFilter builds a fresh filter object per
         * color change, so an identity key always misses); exotic filters
         * (ColorMatrix) fall back to pointer + generation. Slots hold RefPtrs,
         * so a recycled heap address can never false-hit, and the copy-ctor
         * carries them: N mutate() clones share ONE bake. Sources over
         * TINT_CACHE_MAX_PIXELS are never memoized (re-baked per frame) — on
         * 64/128MB targets RAM beats CPU. MULTIPLY-style filters are not
         * idempotent, so the copy — never tint mBitmap itself. */
        struct TintMemo {
            Cairo::RefPtr<Cairo::ImageSurface> cache;
            Cairo::RefPtr<Cairo::ImageSurface> from;
            Cairo::RefPtr<ColorFilter> with;
            int key1 = 0, key2 = 0;   // value key: (color, mode/mul/add/blendMode)
            int typeId = 0;           // which filter type the value key came from
            int generation = 0;       // in-place-mutation guard (pointer-key path)
            bool byValue = false;
        };
        TintMemo mTintMemo[2];
        int mTintSlot = 0;
        BitmapState();
        BitmapState(Cairo::RefPtr<Cairo::ImageSurface>bitmap);
        BitmapState(const BitmapState&bitmapState);
        ~BitmapState()override;
        BitmapDrawable* newDrawable()override;
        Drawable* newDrawable(Resources* res)override;
        int getChangingConfigurations()const override;
    };
    /* Source-space tinted bitmap, memoized in the state (see the members
     * there); replaces the per-draw tint group for this drawable. */
    static Cairo::RefPtr<Cairo::ImageSurface> tintedBitmap(BitmapState& state,
            const Cairo::RefPtr<ColorFilter>& filter);
    int mBitmapWidth;
    int mBitmapHeight;
    Insets mOpticalInsets;
    std::shared_ptr<BitmapState>mBitmapState;
    cdroid::RefPtr<PorterDuffColorFilter>mTintFilter;
    bool needMirroring();
    void computeBitmapSize();
    void updateDstRectAndInsetsIfDirty();
    BitmapDrawable(std::shared_ptr<BitmapState>state, Resources* res);
    void updateStateFromTypedArray(const TypedArray& a, int srcDensityOverride);
protected:
    bool mMutated;
    Rect mDstRect;
    bool mDstRectAndInsetsDirty;
    void onBoundsChange(const Rect&r)override;
    bool onStateChange(const std::vector<int>&)override;
public:
    BitmapDrawable();
    BitmapDrawable(Cairo::RefPtr<Cairo::ImageSurface>img);
    BitmapDrawable(Context*ctx,const std::string&resname);
    ~BitmapDrawable();
    Cairo::RefPtr<Cairo::ImageSurface> getBitmap()const;
    void setBitmap(Cairo::RefPtr<Cairo::ImageSurface>bmp);
    void setAlpha(int a)override;
    int getAlpha()const override;
    int getGravity()const;
    void setGravity(int gravity);
    void setMipMap(bool);
    bool hasMipMap()const;
    void setAntiAlias(bool aa);
    bool hasAntiAlias()const;
    void setFilterBitmap(bool filter)override;
    bool isFilterBitmap()const override;
    void setDither(bool)override;
    int getIntrinsicWidth() override;
    int getIntrinsicHeight() override;
    int getOpacity()const override;
    int getTileModeX()const;
    int getTileModeY()const;
    void setTileModeX(int);
    void setTileModeY(int);
    void setTileModeXY(int,int);
    void setTargetDensity(int density);
    void setSourceDensity(int density);
    void setAutoMirrored(bool mirrored)override;
    bool isAutoMirrored()const override;
    void setTintList(const cdroid::RefPtr<ColorStateList>&lst)override;
    void setTintMode(int mode)override;
    int getTintMode()const;
    BitmapDrawable*mutate()override;
    void clearMutated()override;
    bool isStateful() const override;
    std::shared_ptr<ConstantState>getConstantState()override;
    void draw(Canvas&canvas)override;
    Insets getOpticalInsets()override;
    void getOutline(Outline&)override;
    void inflate(Resources&r,XmlPullParser&parser,const AttributeSet&atts,const Resources::Theme* theme)override;
    bool canApplyTheme()override;
    void applyTheme(const Resources::Theme& t)override;
};

}
#endif/*__BITMAP_DRAWABLE_H__*/
