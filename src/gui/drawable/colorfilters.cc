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
#include <cairomm/surface.h>
#include <cairo.h>
#include <cstdint>
namespace cdroid{

namespace {
// Map the active group surface to a writable ARGB32 image and run fn over its pixels, then unmap it
// (flushing the edits back to the group). The Drawable::begin/endTintGroup contract leaves the
// pushed group as the active target; after push_group() get_group_target() is a backend-specific
// surface (image for image backends, xlib/pixmap for xlib, ...) — NOT necessarily a Cairo::ImageSurface,
// so a dynamic_pointer_cast<ImageSurface> is unreliable. cairo_surface_map_to_image handles the
// readback for image AND xlib/pixmap targets; a recording surface (no pixels) is skipped with a
// warning. Pixels are premultiplied ARGB32.
template<typename Fn>
void withGroupPixels(Canvas& canvas, const char* who, Fn fn){
    Cairo::RefPtr<Cairo::Surface> g = canvas.get_group_target();
    cairo_surface_t* gt = g ? g->cobj() : nullptr;
    cairo_surface_t* img = gt ? cairo_surface_map_to_image(gt, nullptr) : nullptr;
    if(!img || cairo_surface_status(img) != CAIRO_STATUS_SUCCESS
            || cairo_image_surface_get_format(img) != CAIRO_FORMAT_ARGB32){
        if(img) cairo_surface_destroy(img);
        LOGW("%s: group target is not an ARGB32 image surface (filter skipped)", who);
        return;
    }
    fn(cairo_image_surface_get_data(img), cairo_image_surface_get_stride(img),
       cairo_image_surface_get_width(img), cairo_image_surface_get_height(img));
    cairo_surface_mark_dirty(img);
    cairo_surface_unmap_image(gt, img);
}
}

ColorMatrixColorFilter::ColorMatrixColorFilter(const float(&v)[20]){
    mCM.set(v);
}

void ColorMatrixColorFilter::apply(Canvas&canvas,const Rect&/*rect*/){
    // android.graphics.ColorMatrixColorFilter: 4x5 row-major matrix on unpremultiplied RGBA in the
    // 0..255 space (R' = m[0]*R+m[1]*G+m[2]*B+m[3]*A+m[4]; rows for G,B,A at +5 each), clamp 0..255.
    // The active group is premultiplied ARGB32, so unpremul first, apply the matrix, re-premul after.
    // mCM.mArray is read directly — ColorMatrix::transform(color) is NOT reused: its operator* reads a
    // phantom 5th matrix row (mArray[20], out of bounds) and then divides by it, so it is unreliable.
    const float* m = mCM.mArray;
    auto clamp255=[](float v)->int{ return v<0.f?0:(v>255.f?255:(int)v); };
    withGroupPixels(canvas, "ColorMatrixColorFilter",
        [&](unsigned char* data, int stride, int w, int h){
            for(int y=0;y<h;y++){
                uint32_t* row = reinterpret_cast<uint32_t*>(data + y*stride);
                for(int x=0;x<w;x++){
                    const uint32_t p = row[x];
                    const int a = (p>>24)&0xFF;
                    float fr,fg,fb,fa = (float)a;
                    if(a>0){ fr=((p>>16)&0xFF)*255.f/a; fg=((p>>8)&0xFF)*255.f/a; fb=(p&0xFF)*255.f/a; }
                    else { fr=fg=fb=0.f; } // premultiplied transparent: straight RGB is 0
                    const int iR = clamp255(m[0]*fr+m[1]*fg+m[2]*fb+m[3]*fa+m[4]);
                    const int iG = clamp255(m[5]*fr+m[6]*fg+m[7]*fb+m[8]*fa+m[9]);
                    const int iB = clamp255(m[10]*fr+m[11]*fg+m[12]*fb+m[13]*fa+m[14]);
                    const int iA = clamp255(m[15]*fr+m[16]*fg+m[17]*fb+m[18]*fa+m[19]);
                    row[x] = (iA<<24) | ((iR*iA/255)<<16) | ((iG*iA/255)<<8) | (iB*iA/255);
                }
            }
        });
}

PorterDuffColorFilter::PorterDuffColorFilter(int color,int mode){
    mColor= color;
    mMode = mode;
}

void PorterDuffColorFilter::apply(Canvas&canvas,const Rect&rect){
    /* New contract (Drawable::endTintGroup): a group holding the drawable's content
     * is the active target (DST). Composite the tint color (SRC) onto it using the
     * PorterDuff/blend operator. Option A: cairo's native operator — exact for the 12
     * classic Porter-Duff modes + ADD; W3C-semantic (close, not bit-exact vs Skia) for
     * MULTIPLY/SCREEN/OVERLAY/DARKEN/LIGHTEN (alpha-handling differs; see memory note). */
    canvas.set_operator((Cairo::Context::Operator)PorterDuff::toOperator(mMode));
    canvas.set_color(mColor);
    canvas.paint();
}

void PorterDuffColorFilter::setColor(int c){
    mColor= c;
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
     * content is an active target (DST). Paint the tint color (SRC) over it with the
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

void LightingColorFilter::apply(Canvas&canvas,const Rect&/*rect*/){
    // android.graphics.LightingColorFilter: out.c = clamp(in.c * mul.c/255 + add.c) per channel,
    // alpha unchanged. No single cairo operator expresses mul+add while preserving alpha, so do it
    // per-pixel. Cairo ARGB32 is premultiplied, hence Skia's premultiplied form:
    //   out_premul.c = in_premul.c * mul.c/255 + add.c * alpha/255   (add scaled by alpha keeps the
    //   result premultiplied and leaves transparent pixels untouched); out.a = in.a.
    const int mulR=(mMul>>16)&0xFF, mulG=(mMul>>8)&0xFF, mulB=mMul&0xFF;
    const int addR=(mAdd>>16)&0xFF, addG=(mAdd>>8)&0xFF, addB=mAdd&0xFF;
    withGroupPixels(canvas, "LightingColorFilter",
        [&](unsigned char* data, int stride, int w, int h){
            for(int y=0;y<h;y++){
                uint32_t* row = reinterpret_cast<uint32_t*>(data + y*stride);
                for(int x=0;x<w;x++){
                    const uint32_t p = row[x];
                    const int a = (p>>24)&0xFF;
                    if(a==0) continue; // premultiplied (0,0,0,0): leave transparent
                    const int pr=(p>>16)&0xFF, pg=(p>>8)&0xFF, pb=p&0xFF; // premultiplied RGB
                    int orr=(pr*mulR + addR*a)/255; if(orr>255) orr=255;
                    int ogg=(pg*mulG + addG*a)/255; if(ogg>255) ogg=255;
                    int obb=(pb*mulB + addB*a)/255; if(obb>255) obb=255;
                    row[x] = (a<<24)|(orr<<16)|(ogg<<8)|obb;
                }
            }
        });
}

}/*endof namespace*/
