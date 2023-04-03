#ifndef __GRAPH_CONTEXT_H__
#define __GRAPH_CONTEXT_H__
#include <cdtypes.h>
#include <cdgraph.h>
#include <cairomm/context.h>
#include <cairomm/region.h>
#include <core/graphdevice.h>
#include <gui_features.h>

namespace cdroid{

typedef enum TextAlignment{
    DT_LEFT   =0x00,
    DT_CENTER =0x01,
    DT_RIGHT  =0x02,
    DT_TOP    =0x00,
    DT_VCENTER=0x10,
    DT_BOTTOM =0x20,
    DT_MULTILINE=0x100
}TEXTALIGNMENT;

#define RGBA(r,g,b,a) (((a)<<24)|((r)<<16)|((g)<<8)|(b))
#define RGB(r,g,b) RGBA(r,g,b,0xFF)

class Canvas:public Cairo::Context{
protected:
    void*mHandle;
    friend class Window;
    friend class GraphDevice;
    friend class WindowManager;
public:
    Canvas(const Cairo::RefPtr<Cairo::Surface>&target);
    Canvas(unsigned int width,unsigned int height);
    ~Canvas();
    void get_text_size(const std::string&txt,int*w,int*h); 
    void draw_text(const RECT&rect,const std::string&text,int text_alignment=DT_LEFT|DT_VCENTER);
    void set_color(BYTE r,BYTE g, BYTE b,BYTE a=255);
    void set_color(UINT color);
    void rectangle(int x,int y,int w,int h);
    void rectangle(const RECT &r);
    void draw_image(const Cairo::RefPtr<Cairo::ImageSurface>&img,const RECT&dst,const RECT*src);
    //void rotate(float degrees,float px,float py);
    void dump2png(const char*fname);
};
extern void DumpRegion(const std::string&label,Cairo::RefPtr<Cairo::Region>rgn);
}//namspace
#endif

