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
#ifndef __UI_IMAGE_VIEW_H__
#define __UI_IMAGE_VIEW_H__
#include <view/view.h>

namespace cdroid{

enum ScaleType{
    MATRIX       =0,
    FIT_XY       =1,
    FIT_START    =2,
    FIT_CENTER   =3,
    FIT_END      =4,
    CENTER       =5,
    CENTER_CROP  =6,
    CENTER_INSIDE=7
};

class ImageView : public View {
private:
    bool mColorMod;
    bool mHasColorFilter;
    bool mHasDrawableTint;
    bool mHasDrawableTintMode;
    bool mBaselineAlignBottom;
    int mBaseline;
    int mAlpha;
    int mViewAlphaScale;
    void initImageView();
    void resolveUri();
    int resolveAdjustedSize(int desiredSize, int maxSize,int measureSpec);
    void applyImageTint();
    void applyColorMod();
    bool isFilledByImage()const;
    void imageDrawableCallback(Drawable*d,const std::string&uri,const std::string resid);
protected:
    std::string mResource;
    int mScaleType;
    int mLevel;
    int mMaxWidth;
    int mMaxHeight;
    int mDrawableWidth;
    int mDrawableHeight;
    int mRadii[4];
    bool mAdjustViewBounds;
    bool mMergeState;
    bool mHaveFrame;
    bool mCropToPadding;
    std::vector<int>mState;
    Drawable*mDrawable;
    ColorFilter*mColorFilter;
    const ColorStateList*mDrawableTintList;
    int mDrawableTintMode;
    BitmapDrawable*mRecycleableBitmapDrawable;
    Matrix mMatrix;
    Matrix mDrawMatrix;
    void updateDrawable(Drawable* d);
    void resizeFromDrawable();
    void configureBounds();
    bool setFrame(int l, int t, int w, int h)override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void drawableStateChanged()override;
    virtual void onDraw(Canvas& canvas) override;
    void onAttachedToWindow()override;
    void onDetachedFromWindow()override;
public:
    explicit ImageView(int w, int h);
    ImageView(Context*ctx,const AttributeSet&attrs);
    ~ImageView()override;
    bool verifyDrawable(Drawable* dr)const override;
    void jumpDrawablesToCurrentState()override;
    void invalidateDrawable(Drawable& dr)override;
    std::vector<int> onCreateDrawableState(int)override;
    void onRtlPropertiesChanged(int layoutDirection)override;
    void onVisibilityAggregated(bool isVisible)override;
    int getScaleType()const;
    void setScaleType(int st);
    void setImageMatrix(const Cairo::Matrix& matrix);
    Cairo::Matrix getImageMatrix()const;

    bool getCropToPadding()const;
    void setCropToPadding(bool cropToPadding);
    void setMaxWidth(int);
    void setMaxHeight(int);
    int getMaxWidth()const;
    int getMaxHeight()const;
    Drawable*getDrawable();
    void setBaseline(int baseline);
    int getBaseline()override;
    void setBaselineAlignBottom(bool aligned);
    bool getBaselineAlignBottom()const;
    void onPopulateAccessibilityEventInternal(AccessibilityEvent& event)override;
    bool getAdjustViewBounds()const;
    void setAdjustViewBounds(bool adjustViewBounds);
    /*resid can be assets's resource or local filepath*/
    void setImageResource(const std::string&resid);
    Runnable setImageResourceAsync(const std::string&resid);
    void setImageURI(const std::string&uri);
    Runnable setImageURIAsync(const std::string&uri);
    void setImageDrawable(Drawable* drawable);
    void setImageBitmap(Cairo::RefPtr<Cairo::ImageSurface>bitmap);
    void setImageTintList(const ColorStateList*tint);
    const ColorStateList* getImageTintList();
    void setImageTintMode(int mode);
    int getImageTintMode()const;
    void setColorFilter(int color,int mode);
    void setColorFilter(int color);
    void setColorFilter(ColorFilter* cf);
    void clearColorFilter();
    ColorFilter* getColorFilter();
    void setImageAlpha(int alpha);
    int getImageAlpha()const;
    bool isOpaque()const override;
    void setVisibility(int visibility)override;
    void setImageLevel(int level);
    void setSelected(bool selected)override;
    void setCornerRadii(int radius);
    void setCornerRadii(int topLeftRadius,int topRightRadius,int bottomRightRadius,int bottomLeftRadius);
    void setImageState(const std::vector<int>&state, bool merge);
    void drawableHotspotChanged(float x, float y)override;
    void animateTransform(const Cairo::Matrix* matrix);
};

}
#endif
