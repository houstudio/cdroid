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

class Uri;

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
    bool mBaselineAlignBottom;
    int mBaseline;
    int mAlpha;
    int mViewAlphaScale;
    void initImageView();
    void resolveUri();
    /*AOSP getDrawableFromUri: scheme-dispatched drawable load (android.resource
      / content / file / plain path). Uri is core/uri.h (abstract; const ref
      stands in for AOSP's pass-by-value).*/
    Drawable* getDrawableFromUri(const Uri& uri);
    int resolveAdjustedSize(int desiredSize, int maxSize,int measureSpec);
    void applyImageTint();
    void applyColorMod();
    bool isFilledByImage()const;
    /*AOSP ImageDrawableCallback (async apply); empty uri + resId 0 = none*/
    void imageDrawableCallback(Drawable*d,const std::string&uri,int resId);
protected:
    std::string mResource;
    /*resource id from setImageResource(int); 0 = none (AOSP mResource int)*/
    int mResourceId;
    /*image uri from setImageURI; empty = null (AOSP mUri)*/
    std::string mUri;
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
    cdroid::RefPtr<ColorFilter>mColorFilter;
    cdroid::RefPtr<ColorStateList>mDrawableTintList;
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
    std::string getAccessibilityClassName()const override;
    ImageView(Context*ctx);   // AOSP ImageView(Context)
    ImageView(Context*ctx,const AttributeSet*attrs);
    ImageView(Context*ctx,const AttributeSet* attrs,int defStyleAttr);
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
    void setImageResource(int resid);
    Runnable setImageResourceAsync(int resid);
    /*resid can be assets's resource or local filepath*/
    void setImageResource(const std::string&resid);
    Runnable setImageResourceAsync(const std::string&resid);
    /*uri empty = null (clear); schemes: android.resource/content/file, or a
      plain path (see getDrawableFromUri)*/
    void setImageURI(const std::string&uri);
    Runnable setImageURIAsync(const std::string&uri);
    void setImageDrawable(Drawable* drawable);
    void setImageBitmap(const Cairo::RefPtr<Cairo::ImageSurface>&bitmap);
    void setImageTintList(const cdroid::RefPtr<ColorStateList>&tint);
    const cdroid::RefPtr<ColorStateList> getImageTintList()const;
    void setImageTintMode(int mode);
    void setImageTintBlendMode(int blendMode); /* API29 stub: forwards to setImageTintMode */
    int getImageTintMode()const;
    void setColorFilter(int color,int mode);
    void setColorFilter(int color);
    void setColorFilter(const cdroid::RefPtr<ColorFilter>& cf);
    void clearColorFilter();
    const cdroid::RefPtr<ColorFilter> getColorFilter()const;
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
