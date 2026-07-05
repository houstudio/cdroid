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
#ifndef __COLOR_FILTERS_H__
#define __COLOR_FILTERS_H__
#include <drawable/colormatrix.h>
#include <core/canvas.h>
#include <core/porterduff.h>
namespace cdroid{

class ColorFilter{
public:
    virtual void apply(Canvas&canvas,const Rect&)=0;
    virtual ~ColorFilter()=default;
};

class ColorMatrixColorFilter:public ColorFilter{
protected:
    ColorMatrix mCM;
public:    
    ColorMatrixColorFilter(const float(&v)[20]);
    void apply(Canvas&canvas,const Rect&)override;
};

class PorterDuffColorFilter:public ColorFilter{
protected:
    int mColor;
    int mMode;
public:
    PorterDuffColorFilter(int color,int mode);
    void apply(Canvas&canvas,const Rect&)override;
    void setColor(int c);
    void setMode(int m);
    int getColor()const;
    int getMode()const;
};

/* API 29+ BlendModeColorFilter. cairo operators are W3C == Android BlendMode
 * semantics, so apply just paints the tint color with BlendMode::toOperator —
 * exact for all 28 modes (MODULATE approximated). Usable via Drawable/ImageView
 * setColorFilter(), which the begin/endTintGroup path already drives. */
class BlendModeColorFilter:public ColorFilter{
protected:
    int mColor;
    int mBlendMode;
public:
    BlendModeColorFilter(int color,int blendMode);
    void apply(Canvas&canvas,const Rect&)override;
    int getColor()const;
    int getBlendMode()const;
};

/* R' = R * colorMultiply.R + colorAdd.R
 * G' = G * colorMultiply.G + colorAdd.G
 * B' = B * colorMultiply.B + colorAdd.B
 * The result is pinned to the <code>[0..255]</code> range for each channel*/
class LightingColorFilter:public ColorFilter{
protected:
    int mMul;
    int mAdd;
public:
    LightingColorFilter(int mul,int add);
    int getColorMultiply()const;
    void setColorMultiply(int mul);
    int getColorAdd()const;
    void setColorAdd(int add);
    void apply(Canvas&canvas,const Rect&)override;
};
}/*endof namespace*/

#endif
