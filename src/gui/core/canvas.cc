#include <cdtypes.h>
#include <canvas.h>
#include <cairo.h>
#include <cairo-cdroid.h>
#include <cairomm/cdroid_surface.h>
#include <cdlog.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wordbreak.h>
#include <linebreak.h>
#include <textutils.h>
#include <gravity.h>
#include <windowmanager.h>

using namespace std;
using namespace Cairo;
namespace cdroid{

Canvas::Canvas(GraphDevice*_dev,HANDLE surface)
   :Canvas(_dev,CDroidSurface::create(surface,false)){
    //nglGetSurfaceInfo(surface,(UINT*)&screenRect.width,(UINT*)&screenRect.height,nullptr);
}

Canvas::Canvas(GraphDevice*_dev,const RefPtr<Surface>& target)
   :Context(target){
    dev=_dev;
    mLeft=mTop=0;
    mInvalidRgn=Region::create();
}

Canvas*Canvas::subContext(int x,int y,int w,int h){
    auto suf=Surface::create(get_target(),x,y,w,h);
    auto ctx=new Canvas(dev,suf);
    ctx->mLeft=x;
    ctx->mTop=y;
    return ctx;
}

void Canvas::clip2dirty(){
    std::vector<Cairo::Rectangle>clipsf;   //float regions
    std::vector<Cairo::RectangleInt>clipsi;//integer regions
    copy_clip_rectangle_list(clipsf);
    for(auto c:clipsf){
        RectangleInt r={(int)c.x,(int)c.y,(int)c.width,(int)c.height};
        clipsi.push_back(r);
    }
    RefPtr<Region>rgn=Region::create(clipsi);
    mInvalidRgn->do_union(rgn);
    mInvalidRgn->intersect(mVisibleRgn);
}

void Canvas::invalidate(const Rect&r){
   mInvalidRgn->do_union((RectangleInt&)r); 
}
int Canvas::blit2Device(HANDLE surface){
    HANDLE cdsurface=cairo_cdroid_surface_get_surface(get_target()->cobj());

    int num=mInvalidRgn->get_num_rectangles();
    for(int i=0;i<num;i++){
        RectangleInt r=mInvalidRgn->get_rectangle(i);
        LOGV("blit %p (%d,%d-%d,%d)-->%d,%d",this,r.x,r.y,r.width,r.height,mLeft,mTop);
        GFXBlit(surface,mLeft+r.x,mTop+r.y,cdsurface,(GFXRect*)&r);
    }
    mInvalidRgn->subtract(mInvalidRgn);
    return num;
}

Canvas::~Canvas(){
}

void Canvas::set_position(int x,int y){
    mLeft=x;
    mTop=y;
    dev->flip();
}
void Canvas::set_layer(int l,RefPtr<Region>rgn){
    mLayer=l;
    mVisibleRgn=rgn;
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
    rectangle((double)r.x,(double)r.y,(double)r.width,(double)r.height);
}

static inline float sdot(float a,float b,float c,float d){
    return a * b + c * d;
}

void Canvas::rotate(float degrees,float px,float py){
    const float radians=M_PI*2.f*degrees/360.f;
    const float fsin=sin(radians);
    const float fcos=cos(radians);
#if 0//Anti clockwise
    Matrix mtx(fcos,-fsin, fsin,fcos,
            sdot(-fsin,py,1-fcos,px),   sdot(fsin,px,1-fcos,py));
#else//Clockwise
    Matrix mtx(fcos,fsin, -fsin,fcos,
            sdot(fsin,py,1-fcos,px),   sdot(-fsin,px,1-fcos,py));
#endif
    transform(mtx);
}

void Canvas::roundrect(int x,int y,int width,int height,int radius){
    if(0==radius)
        Context::rectangle(x,y,width,height);
    else{
        double from_degre = M_PI/180;
        arc(x+width-radius, y+height-radius, radius, 0*from_degre, 90*from_degre);
        arc(x+radius, y+height-radius, radius, 90*from_degre, 180*from_degre);
        arc(x+radius, y+radius, radius, 180*from_degre, 270*from_degre);
        arc(x+width-radius, y+radius, radius, 270*from_degre, 360*from_degre);
        close_path();
    }
}

void Canvas::get_text_size(const std::string&text,int*width,int *height){
    TextExtents te;
    get_text_extents(text,te);
    if(width)*width=te.width;
    if(height)*height=te.height;
}

void Canvas::draw_text(const RECT&rect,const std::string&text,int text_alignment){
     char*brks=(char*)malloc(text.length()) ;//WORDBREAK_BREAK, #WORDBREAK_NOBREAK,#WORDBREAK_INSIDEACHAR
     set_wordbreaks_utf8((const utf8_t*)text.c_str(),text.length(),"",brks);
     const char*ptxt=text.c_str();
     const char*pword=ptxt;
     TextExtents extents,te;
     FontExtents ftext;
     std::vector<std::string>lines;
     std::string line;
     double x,y;
     double total_width=0, total_height=0;
     get_font_extents(ftext);
     //LOGV("ascent=%.3f descent=%.3f height=%.3f xy_advance=%.3f/%.3f",ftext.ascent,ftext.descent,ftext.height,ftext.max_x_advance,ftext.max_y_advance);
     for(int i=0; i<text.length();i++){
         switch(brks[i]){
         case WORDBREAK_BREAK:{
                 std::string word(pword,ptxt+i-pword+1);
                 get_text_extents(word,extents);
                 if( ((total_width+extents.width > rect.width) || (text[i]=='\n')) && (text_alignment&DT_MULTILINE) ){
                     lines.push_back(line);
                     total_height+=ftext.height;
                     line="";total_width=0;
                     size_t ps=word.find('\n');
                     if(ps!=std::string::npos)word.erase(ps,1);
                 }
                 total_width+=extents.x_advance;
                 line.append(word);
                 pword=ptxt+i+1;
             }
             break;
        case WORDBREAK_NOBREAK:    break;
        case WORDBREAK_INSIDEACHAR:break;
        default:break;
        }
    }
    free(brks);
    total_height+=extents.height;
    lines.push_back(line);
    total_height=lines.size()*ftext.height;
    
    if((text_alignment&DT_MULTILINE)==0){
        get_text_extents(lines[0],te);
        y=rect.y;
        switch(text_alignment&0xF0){
        case DT_TOP:y=rect.y+ftext.ascent-ftext.descent;break;
        case DT_VCENTER:y=rect.y+rect.height/2+(ftext.ascent-ftext.descent)/2;break;
        case DT_BOTTOM:y=rect.y+rect.height;break;
        }
        switch(text_alignment&0x0F){
        case DT_LEFT:x=rect.x;break;
        case DT_CENTER:x=rect.x+(rect.width-te.x_advance)/2;break;
        case DT_RIGHT:x=rect.x+rect.width-te.x_advance;break;
        }
        move_to(x,y);
        show_text(text);
    }else {
        y=rect.y;
        switch(text_alignment&0xF0){
        case DT_TOP:y=rect.y+ftext.descent;break;
        case DT_VCENTER:y=rect.y+(rect.height-total_height)/2+ftext.descent;break;
        case DT_BOTTOM:y=rect.y+rect.height-total_height+ftext.descent;break;
        }
        for(auto line:lines){
            get_text_extents(line,te);
            switch(text_alignment&0x0F){
            case DT_LEFT:x=rect.x;break;
            case DT_CENTER:x=rect.x+(rect.width-te.x_advance)/2;break;
            case DT_RIGHT:x=rect.x+rect.width-te.x_advance;break;
            }
            move_to(x,y-te.y_bearing);
            y+=ftext.height;
            show_text(line);
        }
    }
}

void Canvas::draw_text(int x,int y,const std::string& text){
    FontExtents fe;
    get_font_extents(fe);
    move_to(x,y-fe.descent);//-fe.descent);
    show_text(text);
}

void Canvas::draw_image(const RefPtr<ImageSurface>&img,const RECT&dst,const RECT*srcRect){
    Rect src=srcRect==nullptr?Rect::Make(0,0,img->get_width(),img->get_height()):*srcRect;
    
    const float sx=src.x    , sy=src.y;
    const float sw=src.width, sh=src.height;
    float dx =dst.x     , dy = dst.y;
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
    set_source(img, dx - sx, dy - sy);
    paint_with_alpha(1.0);
    restore();
}

void Canvas::draw_ninepatch(const RefPtr<ImageSurface>img,const RECT& rect,const std::vector<NinePatchBlock>&horz,
        const std::vector<NinePatchBlock>& vert){
    int dw=rect.width;
    int dh=rect.height;
    int sw=img->get_width();
    int sh=img->get_height();
    float horz_stretch=0;
    float vert_stretch=0;

    float horz_mul=0,vert_mul =0;
    int dy0=0, dy1=0;
    float vstretch=0;
    for_each(horz.begin(),horz.end(),[&](const NinePatchBlock&v){if(v.stretchable)horz_stretch+=v.len;});
    for_each(vert.begin(),vert.end(),[&](const NinePatchBlock&v){if(v.stretchable)vert_stretch+=v.len;});

    if (horz_stretch > 0) horz_mul = (float)(dw - (sw - 2 - horz_stretch)) / horz_stretch;
    if (vert_stretch > 0) vert_mul = (float)(dh - (sh - 2 - vert_stretch)) / vert_stretch;
    for(int i=0;i<(int)vert.size();i++){
        int sy0=vert[i].pos;
        if(i+1==(int)vert.size()){
            dy1=dh;
        }else if(vert[i].stretchable){
            vstretch=(float)vert[i].len*vert_mul;
            float s=floor(vstretch);
            vstretch-=s;
            dy1+=(int)s;
        }else{
            dy1+=vert[i].len;
        }
        int dx0=0,dx1=0;
        float hstretch=0;
        for(int j=0;j<(int)horz.size();j++){
            int sx0=horz[j].pos;
            if(j+1==(int)horz.size()){
                dx1=dw;
            }else if(horz[j].stretchable){
                hstretch+=(float)horz[j].len*horz_mul;
                float s=floor(hstretch);
                hstretch-=s;
                dx1+=(int)s;
            }else{
                dx1+=horz[j].len;
            }
            RECT rd={dx0,dy0,dx1-dx0,dy1-dy0};
            RECT rs={sx0+1, sy0+1,horz[j].len,vert[i].len};
            rd.offset(rect.x,rect.y);
            draw_image(img,rd,&rs);
            dx0=dx1;
        }
        dy0=dy1;
    }    
}

void Canvas::dump2png(const char*fname){
#ifdef CAIRO_HAS_PNG_FUNCTIONS
    get_target()->write_to_png(fname);
#else
    LOGW("PNG NOT support!");
#endif
}

void DumpRegion(const std::string&label,RefPtr<Region>rgn){
    for(int i=0;i<rgn->get_num_rectangles();i++){
       RectangleInt rr=rgn->get_rectangle(i);
       LOGV("%s[%d]=%d,%d-%d,%d)",label.c_str(),i,rr.x,rr.y,rr.width,rr.height);
    }
}

}//namespace
