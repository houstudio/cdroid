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
#ifndef __LAYER_DRAWABLE_H__
#define __LAYER_DRAWABLE_H__
#include <drawable/drawable.h>
namespace cdroid{

class LayerDrawable:public Drawable,public Drawable::Callback {
public:
    static constexpr int PADDING_MODE_NEST=0;
    static constexpr int PADDING_MODE_INNER=1;
    static constexpr int PADDING_MODE_STACK=2;
protected:
    class ChildDrawable{
    public:
        Drawable*mDrawable;
        int mInsetL, mInsetT, mInsetR, mInsetB,mInsetE,mInsetS;
        int mWidth,mHeight;
        int mGravity;
        int mDensity;
        int mId;
        std::vector<int>mThemeAttrs;
        ChildDrawable(int density);
        ChildDrawable(ChildDrawable* orig,LayerDrawable*owner);
        ~ChildDrawable();
        void setDensity(int targetDensity);
        void applyDensityScaling(int sourceDensity, int targetDensity);
    };
    class LayerState:public std::enable_shared_from_this<LayerState>,public ConstantState{ 
    public:
        int mDensity;
        int mPaddingTop,mPaddingBottom;
        int mPaddingLeft,mPaddingRight;
        int mPaddingStart,mPaddingEnd;
        int mOpacityOverride;
        int mChangingConfigurations,mChildrenChangingConfigurations;
        bool mCheckedOpacity;
        int  mOpacity;
        std::vector<int>mThemeAttrs;

        bool mCheckedStateful;
        bool mIsStateful;
        bool mAutoMirrored;
        int mPaddingMode;
        std::vector< ChildDrawable*>mChildren;
        LayerState();
        LayerState(const LayerState*state,LayerDrawable*owner);
        ~LayerState();
        LayerDrawable*newDrawable()override;
        int getChangingConfigurations()const override;
        int getOpacity();
        bool isStateful()const;
        bool hasFocusStateSpecified()const;
        bool canConstantState()const;
        void invalidateCache();
        void setDensity(int targetDensity);
        virtual void onDensityChanged(int sourceDensity, int targetDensity);
        void applyDensityScaling(int sourceDensity, int targetDensity);
    };
    bool mSuspendChildInvalidation;
    bool mChildRequestedInvalidation;
    void updateLayerBounds(const Rect& bounds);
    void updateLayerBoundsInternal(const Rect& bounds);
    void suspendChildInvalidation();
    void resumeChildInvalidation();
    static int resolveGravity(int gravity, int width, int height,
            int intrinsicWidth, int intrinsicHeight);
    std::shared_ptr<LayerState> mLayerState;
private:
    bool mMutated;
    Rect mHotspotBounds;
    std::vector<int>mPaddingL;
    std::vector<int>mPaddingT;
    std::vector<int>mPaddingR;
    std::vector<int>mPaddingB;
    bool refreshChildPadding(int i, ChildDrawable* r);
    void setLayerInsetInternal(int index, int l, int t, int r, int b, int s, int e);
    void computeNestedPadding(Rect& padding);
    void computeStackedPadding(Rect& padding);
    ChildDrawable* createLayer(Drawable* dr);
    Drawable* getFirstNonNullDrawable()const;
    void inflateLayers(XmlPullParser& parser,const AttributeSet& atts);
    void updateStateFromTypedArray(const AttributeSet&atts);
    void updateLayerFromTypedArray(ChildDrawable*layer,const AttributeSet&atts);
protected:
    virtual std::shared_ptr<LayerState> createConstantState(LayerState* state,const AttributeSet*);
    void onBoundsChange(const Rect& bounds)override;
    bool onLevelChange(int level)override;
    bool onStateChange(const std::vector<int>& state)override;

    void ensurePadding();
    void refreshPadding();

    int addLayer(ChildDrawable* layer);
    LayerDrawable(std::shared_ptr<LayerState>state);
    ChildDrawable* addLayer(Drawable* dr,const std::vector<int>&themeAttrs,int id,int left,int top,int right,int bottom);
public:
    LayerDrawable();
    LayerDrawable(const std::vector<Drawable*>&drawables);

    void setLayerSize(int index, int w, int h);
    int getLayerWidth(int index)const;
    void setLayerWidth(int index, int w);
    int getLayerHeight(int index)const;
    void setLayerHeight(int idx,int h);
    int getLayerGravity(int index)const;
    void setLayerGravity(int index, int gravity);
    void setLayerAttributes(int idx,const AttributeSet&);
    void setLayerInset(int index, int l, int t, int r, int b);
    void setLayerInsetRelative(int index, int s, int t, int e, int b);
    int getLayerInsetLeft(int index)const;
    void setLayerInsetLeft(int index, int l);
    int getLayerInsetRight(int index)const;
    void setLayerInsetRight(int index, int r);
    int getLayerInsetTop(int index)const;
    void setLayerInsetTop(int index, int t);
    int getLayerInsetBottom(int index)const;
    void setLayerInsetBottom(int index, int b);
    int getLayerInsetStart(int index)const;
    void setLayerInsetStart(int index, int s);
    int getLayerInsetEnd(int index)const;
    void setLayerInsetEnd(int index, int e);
    int getId(int index)const;
    void setId(int index, int id);
    int getNumberOfLayers()const;

    int getIntrinsicWidth() override;
    int getIntrinsicHeight() override;
    bool isStateful()const override;
    bool hasFocusStateSpecified()const override;
    void setAutoMirrored(bool mirrored)override;
    bool isAutoMirrored()const override;
    void jumpToCurrentState()override;

    int addLayer(Drawable* dr); 
    Drawable*findDrawableByLayerId(int id);
    virtual bool setDrawableByLayerId(int id, Drawable* drawable);
    int findIndexByLayerId(int id)const;
    Drawable* getDrawable(int index)const;
    void setDrawable(int index, Drawable* drawable);
    bool getPadding(Rect& padding)override;
    void setPadding(const AttributeSet&);
    void setPadding(int left, int top, int right, int bottom);
    void setPaddingRelative(int start,int top,int end,int bottom);
    int getLeftPadding()const;
    int getRightPadding()const;
    int getStartPadding()const;
    int getEndPadding()const;
    int getTopPadding()const;
    int getBottomPadding()const;
    virtual void setPaddingMode(int mode);
    int getPaddingMode()const;
    void setHotspot(float x,float y)override;
    void setHotspotBounds(int left,int top,int width,int height)override;
    void getHotspotBounds(Rect& outRect)const override;
    void setTintList(const ColorStateList* tint)override;
    void setTintMode(int tintMode)override;
    bool setVisible(bool visible,bool restart)override;
    void setDither(bool dither)override;
    void setAlpha(int alpha)override;
    int  getAlpha()const override;
    void setOpacity(int opacity);
    int  getOpacity()override;
    LayerDrawable*mutate()override;
    void clearMutated()override;
    bool onLayoutDirectionChanged(int layoutDirection)override;
    std::shared_ptr<ConstantState>getConstantState()override;
    void invalidateDrawable(Drawable& who)override;
    void scheduleDrawable(Drawable& who,const Runnable& what, int64_t when)override;
    void unscheduleDrawable(Drawable& who,const Runnable& what)override;
    void draw(Canvas&canvas)override;
    void inflate(XmlPullParser&parser,const AttributeSet&atts)override;
};
}
#endif
