#ifndef __GRAPH_CONTEXT_H__
#define __GRAPH_CONTEXT_H__
#include <cdtypes.h>
#include <cdgraph.h>
#include <cmath>
#include <cairomm/context.h>
#include <cairomm/region.h>
#include <core/graphdevice.h>

namespace cdroid{

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
    void draw_text(const Rect&rect,const std::string&text,int text_alignment=0);
    void set_color(uint8_t r,uint8_t g, uint8_t b,uint8_t a=255);
    void set_color(UINT color);
    void rectangle(int x,int y,int w,int h);
    void rectangle(const Rect &r);
    void draw_image(const Cairo::RefPtr<Cairo::ImageSurface>&img,const Rect& dst,const Rect* src);
    //void rotate(float degrees,float px,float py);
    void dump2png(const char*fname);
};
extern void DumpRegion(const std::string&label,const Cairo::RefPtr<Cairo::Region>&rgn);
}//namspace
#endif

