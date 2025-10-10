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

#ifndef __DRAWABLE_WRAPPER_H__
#define __DRAWABLE_WRAPPER_H__
#include <drawables/drawable.h>
namespace cdroid{

class DrawableWrapper:public Drawable,Drawable::Callback{
protected:
    class DrawableWrapperState:public std::enable_shared_from_this<DrawableWrapperState>,public ConstantState{
    public:
        int mDensity;
        int mSrcDensityOverride;
        int mChangingConfigurations;
        std::vector<int>mThemeAttrs;
        std::shared_ptr<ConstantState> mDrawableState;

        DrawableWrapperState();
        DrawableWrapperState(const DrawableWrapperState& orig);
        void setDensity(int targetDensity);
        virtual void onDensityChanged(int sourceDensity, int targetDensity);
        DrawableWrapper* newDrawable()override;
        int getChangingConfigurations()const override;
        virtual bool canConstantState()const;
    };
protected:
    bool mMutated;
    Drawable*mDrawable;
    std::shared_ptr<DrawableWrapperState>mState;
    void updateLocalState();
    void updateStateFromTypedArray(const AttributeSet&atts);
    void inflateChildDrawable(XmlPullParser& parser,const AttributeSet& attrs);
protected:
    virtual std::shared_ptr<DrawableWrapperState> mutateConstantState();
    DrawableWrapper(std::shared_ptr<DrawableWrapperState>state);
    bool onStateChange(const std::vector<int>& state)override;
    bool onLevelChange(int level)override;
    void onBoundsChange(const Rect& bounds)override;
public:
    DrawableWrapper(Drawable*d=nullptr);
    ~DrawableWrapper()override;
    int getIntrinsicWidth () override;
    int getIntrinsicHeight() override;
    void getOutline(Outline&) override;
    int getChangingConfigurations()const override;
    void getHotspotBounds(Rect& outRect)const override;
    void setDrawable(Drawable* dr);
    Drawable* getDrawable()const;
    DrawableWrapper*mutate()override;
    void clearMutated()override;
    std::shared_ptr<ConstantState>getConstantState()override;
    void invalidateDrawable(Drawable& who)override;
    void scheduleDrawable(Drawable&who,const Runnable& what, int64_t when)override;
    void unscheduleDrawable(Drawable& who,const Runnable& what)override;
    void draw(Canvas&canvas)override;
    bool getPadding(Rect& padding)override;
    Insets getOpticalInsets()override;
    bool setVisible(bool visible, bool restart)override;
    void setAlpha(int alpha)override;
    int getAlpha()const override;
    void setColorFilter(ColorFilter*)override;
    ColorFilter*getColorFilter()override;
    void setTintList(const ColorStateList*)override;
    void setTintMode(int)override;
    bool isStateful()const override;
    bool hasFocusStateSpecified()const override;
    void inflate(XmlPullParser&,const AttributeSet&atts)override;
};

}
#endif
