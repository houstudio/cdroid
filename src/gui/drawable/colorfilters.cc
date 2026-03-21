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
    Cairo::ImageSurface*img=dynamic_cast<Cairo::ImageSurface*>(canvas.get_target().get());
    uint8_t *data=img->get_data();
}

PorterDuffColorFilter::PorterDuffColorFilter(int color,int mode){
    mColor= color;
    mMode = mode;
}

void PorterDuffColorFilter::apply(Canvas&canvas,const Rect&rect){
    Cairo::RefPtr<Cairo::Pattern> pattern = canvas.get_source();
    canvas.set_color(mColor);
    canvas.mask(pattern);
    canvas.set_operator(Cairo::Context::Operator(10));//(Cairo::Context::Operator)PorterDuff::toOperator(mMode));
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
    // extract source pattern and rasterize it to an image surface if needed
    Cairo::RefPtr<Cairo::Pattern> pat = canvas.get_source();
    if (pat->get_type() != Cairo::Pattern::Type::SURFACE) {
        LOGE("LightingColorFilter only supports surface pattern as source");
        return;
    }

    // always work on an ImageSurface copy so we can access pixels
    auto img = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, rect.width, rect.height);
    {
        auto cr = Cairo::Context::create(img);
        cr->set_source(pat);
        cr->paint();
    }

    Color cmul(mMul);
    Color cadd(mAdd);

    // perform multiply/add by painting with appropriate operators
    auto cr2 = Cairo::Context::create(img);

    cr2->set_operator((Cairo::Context::Operator)CAIRO_OPERATOR_MULTIPLY);
    cr2->set_source_rgba(cmul.red(), cmul.green(), cmul.blue(), cmul.alpha());
    cr2->paint();

    cr2->set_operator((Cairo::Context::Operator)CAIRO_OPERATOR_ADD);
    cr2->set_source_rgba(cadd.red(), cadd.green(), cadd.blue(), cadd.alpha());
    cr2->paint();

    img->mark_dirty();

    // replace canvas source with filtered image
    auto new_pat = Cairo::SurfacePattern::create(img);
    canvas.set_source(new_pat);
    canvas.paint();
}

}

