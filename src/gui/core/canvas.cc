#include <cdtypes.h>
#include <canvas.h>
#include <cairo.h>
#include <cdlog.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <textutils.h>
#include <view/gravity.h>
#include <windowmanager.h>
#include <cdgraph.h>
using namespace std;
using namespace Cairo;
namespace cdroid{

Canvas::Canvas(const RefPtr<Surface>& target)
   :Context(target){
    mHandle=nullptr;
    //mInvalidRgn=Region::create();
}

Canvas::Canvas(unsigned int width,unsigned int height):Context(nullptr,true){
    BYTE*buffer;
    INT format;
    UINT pitch;
    GFXCreateSurface(0,&mHandle,width,height,GPF_ARGB,false);
    GFXLockSurface(mHandle,(void**)&buffer,&pitch);
    RefPtr<Surface>surf=ImageSurface::create(buffer,Surface::Format::ARGB32,width,height,pitch);
    m_cobject=cairo_create(surf->cobj());
}

Canvas::~Canvas(){
    if(mHandle)
        GFXDestroySurface(mHandle);
}

void Canvas::set_color(UINT color){
    set_color((color>>16)&0xFF,(color>>8)&0xFF,color&0xFF,(color>>24));
}

void Canvas::set_color(BYTE r,BYTE g,BYTE b,BYTE a){
    set_source_rgba(r/255.f,g/255.f,b/255.f,a/255.f);
}

void Canvas::rectangle(int x,int y,int w,int h){
    Context::rectangle((double)x,(double)y,(double)w,(double)h);
}

void Canvas::rectangle(const RECT &r){
    rectangle((double)r.left,(double)r.top,(double)r.width,(double)r.height);
}

void Canvas::get_text_size(const std::string&text,int*width,int *height){
    TextExtents te;
    get_text_extents(text,te);
    if(width)*width=te.width;
    if(height)*height=te.height;
}

void Canvas::draw_text(const Rect&rect,const std::string&text,int text_alignment){
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

void Canvas::draw_image(const RefPtr<ImageSurface>&img,const RECT&dst,const RECT*srcRect){
    Rect src=srcRect==nullptr?Rect::Make(0,0,img->get_width(),img->get_height()):*srcRect;
    
    const float sx=src.left  , sy=src.top;
    const float sw=src.width , sh=src.height;
    float dx =dst.left     , dy = dst.top;
    float dw =dst.width , dh = dst.height;
    float fx = dw / sw  , fy = dh / sh;

    save();

    rectangle(dx, dy, dw, dh);
    if (dw != sw || dh != sh) {
       scale(fx, fy);
       dx /= fx;       dy /= fy;
       dw /= fx;       dh /= fy;
    }

    clip();
    set_source(img,dx - sx, dy - sy);
    paint_with_alpha(1.0);
    restore();
}

void Canvas::dump2png(const char*fname){
#ifdef CAIRO_HAS_PNG_FUNCTIONS
    get_target()->write_to_png(fname);
#else
    LOGW("PNG NOT support!");
#endif
}

void DumpRegion(const std::string&label,RefPtr<Region>rgn){
    RectangleInt re=rgn->get_extents();
    for(int i=0;i<rgn->get_num_rectangles();i++){
       RectangleInt rr=rgn->get_rectangle(i);
       LOGV("%s[%d]=%d,%d-%d,%d)",label.c_str(),i,rr.x,rr.y,rr.width,rr.height);
    }
    LOGV("extents:(%d,%d,%d,%d)",re.x,re.y,re.width,re.height);
}

}//namespace
