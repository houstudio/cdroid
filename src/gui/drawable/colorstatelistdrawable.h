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
/*
 * Ported to C++ for CDROID from android.graphics.drawable.ColorStateListDrawable (Android 12).
 * A Drawable that manages a ColorDrawable to make it stateful and backed by a ColorStateList.
 * Programmatic-only (Android has no XML tag / inflater entry for this class).
 */
#ifndef __COLOR_STATE_LIST_DRAWABLE_H__
#define __COLOR_STATE_LIST_DRAWABLE_H__
#include <drawable/drawable.h>
#include <drawable/colorstatelist.h>
#include <memory>

namespace cdroid{
class ColorDrawable;

class ColorStateListDrawable:public Drawable,public Drawable::Callback{
public:
    class ColorStateListDrawableState;
private:
    ColorDrawable* mColorDrawable = nullptr;
    std::shared_ptr<ColorStateListDrawableState> mState;
    bool mMutated = false;
    void initializeColorDrawable();
    ColorStateListDrawable(std::shared_ptr<ColorStateListDrawableState> state);
protected:
    void onBoundsChange(const Rect& bounds)override;
    bool onStateChange(const std::vector<int>& state)override;
public:
    ColorStateListDrawable();
    ColorStateListDrawable(const cdroid::RefPtr<ColorStateList>& colorStateList);
    ~ColorStateListDrawable()override;

    void draw(Canvas& canvas)override;
    int getAlpha()const override;
    bool isStateful()const override;
    bool hasFocusStateSpecified()const override;
    Drawable* getCurrent()override;
    bool canApplyTheme()override;
    void setAlpha(int alpha)override;
    void clearAlpha(); // androidx: clearAlpha is non-virtual on Drawable; CDROID base lacks it
    void setTintList(const cdroid::RefPtr<ColorStateList>& tint)override;
    void setTintBlendMode(int blendMode)override;
    const cdroid::RefPtr<ColorFilter> getColorFilter()const override;
    void setColorFilter(const cdroid::RefPtr<ColorFilter>& colorFilter)override;
    int getOpacity()const override;
    std::shared_ptr<ConstantState> getConstantState()override;
    cdroid::RefPtr<ColorStateList> getColorStateList();
    int getChangingConfigurations()const override;
    Drawable* mutate()override;
    void clearMutated()override;
    void setColorStateList(const cdroid::RefPtr<ColorStateList>& colorStateList);

    // Drawable::Callback — forward the wrapped ColorDrawable's calls as *this.
    void invalidateDrawable(Drawable& who)override;
    void scheduleDrawable(Drawable& who,const Runnable& what,int64_t when)override;
    void unscheduleDrawable(Drawable& who,const Runnable& what)override;
};

// androidx ColorStateListDrawable.ColorStateListDrawableState (248-293).
class ColorStateListDrawable::ColorStateListDrawableState:
        public std::enable_shared_from_this<ColorStateListDrawableState>,
        public ConstantState{
public:
    cdroid::RefPtr<ColorStateList> mColor;
    cdroid::RefPtr<ColorStateList> mTint;
    int mAlpha = -1;
    int mBlendMode = Drawable::DEFAULT_BLEND_MODE;
    int mChangingConfigurations = 0;

    ColorStateListDrawableState();
    ColorStateListDrawableState(const ColorStateListDrawableState& orig);
    Drawable* newDrawable()override;
    int getChangingConfigurations()const override;
    bool isStateful()const;
    bool hasFocusStateSpecified()const;
    bool canApplyTheme()const;
};

}/*namespace*/
#endif
