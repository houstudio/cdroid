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
#include <cdtypes.h>
#include <canvas.h>
#include <cairo.h>
#include <cdlog.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <utils/textutils.h>
#include <view/gravity.h>
#include <windowmanager.h>
#include <cdgraph.h>
#include <cdlog.h>
using namespace std;
using namespace Cairo;
namespace cdroid{

Canvas::Canvas(const RefPtr<Surface>& target)
   :Context(target){
    mHandle=nullptr;
    //mInvalidRgn=Region::create();
}

Canvas::Canvas(unsigned int width,unsigned int height):Context(nullptr,true){
    uint8_t*buffer;
    uint32_t pitch;
    GFXCreateSurface(0,&mHandle,width,height,GPF_ARGB,false);
    GFXLockSurface(mHandle,(void**)&buffer,&pitch);
    RefPtr<Surface>surf=ImageSurface::create(buffer,Surface::Format::ARGB32,width,height,pitch);
    m_cobject=cairo_create(surf->cobj());
}

Canvas::~Canvas(){
    if(mHandle)
        GFXDestroySurface(mHandle);
}

void*Canvas::getHandler()const{
    return mHandle;
}

void Canvas::set_color(uint32_t color){
    set_color((color>>16)&0xFF,(color>>8)&0xFF,color&0xFF,(color>>24));
}

void Canvas::set_color(uint8_t r,uint8_t g,uint8_t b,uint8_t a){
    set_source_rgba(r/255.f,g/255.f,b/255.f,a/255.f);
}

void Canvas::rectangle(int x,int y,int w,int h){
    Context::rectangle((double)x,(double)y,(double)w,(double)h);
}

void Canvas::rectangle(const Rect &r){
    rectangle((double)r.left,(double)r.top,(double)r.width,(double)r.height);
}

void Canvas::get_text_size(const std::string&text,int*width,int *height){
    TextExtents te;
    get_text_extents(text,te);
    if(width)*width=te.width;
    if(height)*height=te.height;
}

void Canvas::draw_text(const Rect& rect,const std::string& text,int text_alignment){
    TextExtents te;
    FontExtents ftext;
    double x,y;
    get_font_extents(ftext);
    get_text_extents(text,te);
    switch(text_alignment & Gravity::VERTICAL_GRAVITY_MASK){
    default:
    case Gravity::TOP    : y = rect.top + ftext.ascent-ftext.descent;break;
    case Gravity::CENTER_VERTICAL: y = rect.top+rect.height/2+(ftext.ascent-ftext.descent)/2;break;
    case Gravity::BOTTOM : y = rect.top+rect.height; break;
    }
    switch(text_alignment & Gravity::HORIZONTAL_GRAVITY_MASK){
    default:
    case Gravity::LEFT  : x = rect.left ; break;
    case Gravity::CENTER_HORIZONTAL:x = rect.left+(rect.width-te.x_advance)/2;break;
    case Gravity::RIGHT : x = rect.left+rect.width-te.x_advance;break;
    }
    move_to(x - te.x_bearing,y);
    show_text(text);
}

void Canvas::draw_image(const RefPtr<ImageSurface>& img,const Rect& dst,const Rect* srcRect){
    Rect src= (srcRect==nullptr) ? Rect::Make(0,0,img->get_width(),img->get_height()) : *srcRect;
    
    const float sx = float(src.left)  , sy = float(src.top);
    const float sw = float(src.width) , sh = float(src.height);
    float dx = float(dst.left)  , dy = float(dst.top);
    float dw = float(dst.width) , dh = float(dst.height);
    float fx = dw / sw   , fy = dh / sh;

    save();

    rectangle(dx, dy, dw, dh);
    if (dw != sw || dh != sh) {
       scale(fx, fy);
       dx /= fx;       dy /= fy;
    }

    clip();
    set_source(img,dx - sx, dy - sy);
    paint_with_alpha(1.0);
    restore();
}

void Canvas::dump2png(const std::string& fname){
#ifdef CAIRO_HAS_PNG_FUNCTIONS
    get_target()->write_to_png(fname);
#else
    LOGW("PNG NOT support!");
#endif
}

void DumpRegion(const std::string&label,const Cairo::RefPtr<Cairo::Region>& rgn){
    RectangleInt re=rgn->get_extents();
    LOGV("%s:%d retcs",label.c_str(),rgn->get_num_rectangles());
    for(int i=0;i<rgn->get_num_rectangles();i++){
       RectangleInt rr=rgn->get_rectangle(i);
       LOGV("%s[%d]=%d,%d-%d,%d)",label.c_str(),i,rr.x,rr.y,rr.width,rr.height);
    }
    LOGV("extents:(%d,%d,%d,%d)",re.x,re.y,re.width,re.height);
}

}//namespace
