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
#include <drawable/colorfilters.h>
#include <porting/cdlog.h>
#include <core/color.h>
namespace cdroid{

ColorMatrixColorFilter::ColorMatrixColorFilter(const float(&v)[20]){
    mCM.set(v);
}

void ColorMatrixColorFilter::apply(Canvas&canvas,const Rect&rect){
    /* TODO: implement per-pixel 4x5 matrix. The group is already the active target
     * (see Drawable::begin/endTintGroup): get it via canvas.get_group_target(), cast
     * to ImageSurface, run mCM over each ARGB pixel, mark_dirty(). ColorMatrix even
     * has a ready transform(ImageSurface) helper. Deferred per project scope. */
    LOGW("ColorMatrixColorFilter::apply not yet implemented");
}

PorterDuffColorFilter::PorterDuffColorFilter(int color,int mode){
    mColor= color;
    mMode = mode;
}

void PorterDuffColorFilter::apply(Canvas&canvas,const Rect&rect){
    /* New contract (Drawable::endTintGroup): a group holding the drawable's content
     * is the active target (the DST). Composite the tint color (SRC) onto it using the
     * PorterDuff/blend operator. Option A: cairo's native operator — exact for the 12
     * classic Porter-Duff modes + ADD; W3C-semantic (close, not bit-exact vs Skia) for
     * MULTIPLY/SCREEN/OVERLAY/DARKEN/LIGHTEN (alpha-handling differs; see memory note). */
    canvas.set_operator((Cairo::Context::Operator)PorterDuff::toOperator(mMode));
    canvas.set_color(mColor);
    canvas.paint();
}

void PorterDuffColorFilter::setColor(int c){
    mColor = c;
}

int PorterDuffColorFilter::getColor()const{
    return mColor;
}

void PorterDuffColorFilter::setMode(int m){
    mMode = m;
}

int PorterDuffColorFilter::getMode()const{
    return mMode;
}

BlendModeColorFilter::BlendModeColorFilter(int color,int blendMode):mColor(color),mBlendMode(blendMode){
}

void BlendModeColorFilter::apply(Canvas&canvas,const Rect&){
    /* Same in-group contract as PorterDuffColorFilter: a group holding the drawable's
     * content is the active target (DST). Paint the tint color (SRC) over it with the
     * BlendMode's cairo operator — which is W3C-exact for BlendMode. */
    canvas.set_operator((Cairo::Context::Operator)BlendMode::toOperator(mBlendMode));
    canvas.set_color(mColor);
    canvas.paint();
}

int BlendModeColorFilter::getColor()const{
    return mColor;
}

int BlendModeColorFilter::getBlendMode()const{
    return mBlendMode;
}

LightingColorFilter::LightingColorFilter(int mul,int add){
    mMul = mul;
    mAdd = add;
}

int LightingColorFilter::getColorMultiply()const{
    return mMul;
}

void LightingColorFilter::setColorMultiply(int mul){
    mMul = mul;
}

int LightingColorFilter::getColorAdd()const{
    return mAdd;
}

void LightingColorFilter::setColorAdd(int add){
    mAdd = add;
}

void LightingColorFilter::apply(Canvas&canvas,const Rect&rect){
    /* TODO: implement per-channel mul+add (a subset of ColorMatrixColorFilter). On the
     * active group surface (canvas.get_group_target()) do R'=R*mul.R+add.R per channel.
     * Deferred alongside ColorMatrixColorFilter; the previous operator-based body did
     * not match the new in-group contract. */
    LOGW("LightingColorFilter::apply not yet implemented");
}

}

