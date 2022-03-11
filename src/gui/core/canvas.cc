#include <cdtypes.h>
#include <canvas.h>
#include <cairo.h>
#include <cdlog.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wordbreak.h>
#include <linebreak.h>
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
    GFXCreateSurface(&mHandle,width,height,GPF_ARGB,false);
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

static inline float sdot(float a,float b,float c,float d){
    return a * b + c * d;
}

void Canvas::rotate(float degrees,float px,float py){
    const float radians=M_PI*2.f*degrees/360.f;
    const float fsin=sin(radians);
    const float fcos=cos(radians);
#if 0//Anti clockwise
    Matrix mtx(fcos,-fsin, fsin,fcos, sdot(-fsin,py,1-fcos,px), sdot(fsin,px,1-fcos,py));
#else//Clockwise
    Matrix mtx(fcos,fsin, -fsin,fcos, sdot(fsin,py,1-fcos,px),  sdot(-fsin,px,1-fcos,py));
#endif
    transform(mtx);
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
        y=rect.top;
        switch(text_alignment&0xF0){
        case DT_TOP:y=rect.top + ftext.ascent-ftext.descent;break;
        case DT_VCENTER:y=rect.top+rect.height/2+(ftext.ascent-ftext.descent)/2;break;
        case DT_BOTTOM:y=rect.top+rect.height;break;
        }
        switch(text_alignment&0x0F){
        case DT_LEFT:x=rect.left;break;
        case DT_CENTER:x=rect.left+(rect.width-te.x_advance)/2;break;
        case DT_RIGHT:x=rect.left+rect.width-te.x_advance;break;
        }
        move_to(x,y);
        show_text(text);
    }else {
        y=rect.top;
        switch(text_alignment&0xF0){
        case DT_TOP:y=rect.top + ftext.descent;break;
        case DT_VCENTER:y=rect.top +(rect.height-total_height)/2+ftext.descent;break;
        case DT_BOTTOM:y=rect.top+rect.height-total_height+ftext.descent;break;
        }
        for(auto line:lines){
            get_text_extents(line,te);
            switch(text_alignment&0x0F){
            case DT_LEFT:x=rect.left ; break;
            case DT_CENTER:x=rect.left + (rect.width-te.x_advance)/2;break;
            case DT_RIGHT:x=rect.left + rect.width-te.x_advance;break;
            }
            move_to(x,y-te.y_bearing);
            y+=ftext.height;
            show_text(line);
        }
    }
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
