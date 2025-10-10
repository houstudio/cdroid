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
#ifndef __ADAPTIVE_ICON_DRAWABLE_H__
#define __ADAPTIVE_ICON_DRAWABLE_H__
#include <drawables/drawable.h>
namespace cdroid{
class AdaptiveIconDrawable:public Drawable,Drawable::Callback {
private:
    static constexpr float SAFEZONE_SCALE = 66.f/72.f;
    static constexpr float EXTRA_INSET_PERCENTAGE = 1.f / 4.f;
    static constexpr float DEFAULT_VIEW_PORT_SCALE = 1.f / (1 + 2 * EXTRA_INSET_PERCENTAGE);

    static constexpr int BACKGROUND_ID = 0;
    static constexpr int FOREGROUND_ID = 1;
    static constexpr int MONOCHROME_ID = 2;
protected:
    class ChildDrawable;
    class LayerState;
public:
    static constexpr float MASK_SIZE = 100.f;
private:
    static Path sMask;
    Path mMask;
    Path mMaskScaleOnly;
    Cairo::Matrix mMaskMatrix;
    std::shared_ptr<Cairo::Region> mTransparentRegion;

    //Shader mLayersShader;
    std::shared_ptr<Cairo::ImageSurface> mLayersBitmap;

    Rect mHotspotBounds;
    int mAlpha;
    bool mMutated;

    bool mSuspendChildInvalidation;
    bool mChildRequestedInvalidation;
    Canvas* mCanvas;
    std::shared_ptr<LayerState> mLayerState;
private:
    ChildDrawable* createChildDrawable(Drawable* drawable);
    void addLayer(int index,ChildDrawable* layer);
    void updateLayerBounds(const Rect& bounds);
    void updateLayerBoundsInternal(const Rect& bounds);
    void updateMaskBoundsInternal(const Rect& b);
    void inflateLayers(XmlPullParser& parser,AttributeSet& attrs);
    void updateLayerFromTypedArray(ChildDrawable* layer,AttributeSet& attrs);
    void suspendChildInvalidation();
    void resumeChildInvalidation();
    int getMaxIntrinsicWidth();
    int getMaxIntrinsicHeight();
protected:
    void onBoundsChange(const Rect& bounds)override;
    bool onStateChange(const std::vector<int>&state) override;
    bool onLevelChange(int level) override;
public:
    AdaptiveIconDrawable();
    AdaptiveIconDrawable(LayerState* state);
    std::shared_ptr<LayerState> createConstantState(LayerState* state);
    AdaptiveIconDrawable(Drawable* backgroundDrawable,Drawable* foregroundDrawable);
    AdaptiveIconDrawable(Drawable* backgroundDrawable, Drawable* foregroundDrawable, Drawable* monochromeDrawable);
    void inflate(XmlPullParser& parser,AttributeSet& attrs);
    static float getExtraInsetFraction();
    static float getExtraInsetPercentage();

    Path getIconMask();

    Drawable* getForeground();
    Drawable* getBackground();
    Drawable* getMonochrome();

    void draw(Canvas& canvas) override;
    void invalidateSelf() override;

    void getOutline(Outline& outline) override;

    Cairo::RefPtr<Cairo::Region> getSafeZone();
    Cairo::RefPtr<Cairo::Region> getTransparentRegion() override;
    //void applyTheme() override;
    int getSourceDrawableResId();

    bool canApplyTheme() override;

    bool isProjected() const override;

    void invalidateDrawable(Drawable& who) override;

    void scheduleDrawable(Drawable& who,const Runnable& what, int64_t when)override;
    void unscheduleDrawable(Drawable& who,const Runnable& what) override;

    int getChangingConfigurations() const override;

    void setHotspot(float x, float y) override;
    void setHotspotBounds(int left, int top, int width, int height) override;
    void getHotspotBounds(Rect& outRect) const override;
    bool setVisible(bool visible, bool restart) override;
    void setDither(bool dither) override;
    void setAlpha(int alpha) override;

    int getAlpha() const override;

    void setColorFilter(ColorFilter* colorFilter) override;

    void setTintList(const ColorStateList* tint) override;

    //void setTintBlendMode(BlendMode blendMode)override;

    void setOpacity(int opacity);
    int getOpacity() override;

    void setAutoMirrored(bool mirrored) override;

    bool isAutoMirrored()const override;

    void jumpToCurrentState() override;

    bool isStateful() const override;

    bool hasFocusStateSpecified() const override;

    int getIntrinsicWidth() override;

    int getIntrinsicHeight() override;

    std::shared_ptr<ConstantState> getConstantState() override;

    Drawable* mutate() override;

    void clearMutated()override;
};

class AdaptiveIconDrawable::ChildDrawable {
public:
    int* mThemeAttrs;
    int mDensity = DisplayMetrics::DENSITY_DEFAULT;
    Drawable* mDrawable;

    ChildDrawable(int density);
    ChildDrawable(ChildDrawable* orig,AdaptiveIconDrawable* owner);
    bool canApplyTheme()const;
    void setDensity(int targetDensity);
};

class AdaptiveIconDrawable::LayerState:public Drawable::ConstantState {
    static constexpr int N_CHILDREN = 3;
    std::vector<ChildDrawable*> mChildren;

    // The density at which to render the drawable and its children.
    int mDensity;
    // The density to use when inflating/looking up the children drawables. A value of 0 means
    // use the system's density.
    int mSrcDensityOverride = 0;
    int mOpacityOverride = PixelFormat::UNKNOWN;
    int mChangingConfigurations;
    int mChildrenChangingConfigurations;
    int mSourceDrawableId = 0;//Resources.ID_NULL;
private:
    int* mThemeAttrs;
    int mOpacity;
    bool mCheckedOpacity;
    bool mCheckedStateful;
    bool mIsStateful;
    bool mAutoMirrored = false;
    friend AdaptiveIconDrawable;
public:
    LayerState(LayerState* orig, AdaptiveIconDrawable* owner);

    void setDensity(int targetDensity);

    bool canApplyTheme() const;
    Drawable* newDrawable()override;

    int getChangingConfigurations() const override;

    int getOpacity();

    bool isStateful();

    bool hasFocusStateSpecified()const;

    bool canConstantState();
    void invalidateCache();
};
}/*endof namespace*/
#endif /*__ADAPTIVE_ICON_DRAWABLE_H__*/
