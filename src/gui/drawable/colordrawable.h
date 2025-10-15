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
#ifndef __COLOR_DRAWABLE_H__
#define __COLOR_DRAWABLE_H__
#include <drawable/drawable.h>

namespace cdroid{
class ColorDrawable:public Drawable{
private:
    class ColorState:public std::enable_shared_from_this<ColorState>,public ConstantState{
    public:
        uint32_t mBaseColor;// base color, independent of setAlpha()
        uint32_t mUseColor; // basecolor modulated by setAlpha()
        const ColorStateList*mTint;
        int mTintMode;
        ColorState();
        ColorState(const ColorState& state);
        ColorDrawable* newDrawable()override;
        int getChangingConfigurations()const override;
    };
private:
    bool mMutated;
    std::shared_ptr<ColorState>mColorState;
    PorterDuffColorFilter* mTintFilter;
    ColorDrawable(std::shared_ptr<ColorState> state);
protected:
    bool onStateChange(const std::vector<int>&stateSet)override;
public:
    ColorDrawable(int color);
    ~ColorDrawable();
    void setColor(int color);
    int getColor()const;
    int getAlpha()const override;
    void setAlpha(int a)override;
    int getOpacity()override;
    void getOutline(Outline&)override;
    void setTintList(const ColorStateList* tint)override;
    void setTintMode(int tintMode)override;
    bool isStateful()const override;
    int getChangingConfigurations()const override;
    void inflate(XmlPullParser&,const AttributeSet&)override;
    ColorDrawable*mutate()override;
    void clearMutated()override;
    bool hasFocusStateSpecified()const override;
    std::shared_ptr<ConstantState>getConstantState()override;
    void draw(Canvas&canvas)override;
};
}
#endif
