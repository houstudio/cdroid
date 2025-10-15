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
        const ColorStateList* mTint;
        int mTintMode;
        int mTileModeX;
        int mTileModeY;
        int mSrcDensityOverride;
        int mTargetDensity;
        Cairo::RefPtr<Cairo::ImageSurface>mBitmap;
        BitmapState();
        BitmapState(Cairo::RefPtr<Cairo::ImageSurface>bitmap);
        BitmapState(const BitmapState&bitmapState);
        ~BitmapState()override;
        BitmapDrawable* newDrawable()override;
        int getChangingConfigurations()const override;
    };
    int mBitmapWidth;
    int mBitmapHeight;
    Insets mOpticalInsets;
    std::shared_ptr<BitmapState>mBitmapState;
    PorterDuffColorFilter*mTintFilter;
    bool needMirroring();
    void computeBitmapSize();
    void updateDstRectAndInsetsIfDirty();
    BitmapDrawable(std::shared_ptr<BitmapState>state);
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
    int getOpacity()override;
    int getTileModeX()const;
    int getTileModeY()const;
    void setTileModeX(int);
    void setTileModeY(int);
    void setTileModeXY(int,int);
    void setAutoMirrored(bool mirrored)override;
    bool isAutoMirrored()const override;
    void setTintList(const ColorStateList*lst)override;
    void setTintMode(int mode)override;
    int getTintMode()const;
    BitmapDrawable*mutate()override;
    void clearMutated()override;
    bool isStateful() const override;
    std::shared_ptr<ConstantState>getConstantState()override;
    void draw(Canvas&canvas)override;
    Insets getOpticalInsets()override;
    void getOutline(Outline&)override;
    void inflate(XmlPullParser&parser,const AttributeSet&atts)override;
};

}
#endif
