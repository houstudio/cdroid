#ifndef __GRAPH_CONTEXT_H__
#define __GRAPH_CONTEXT_H__
#include <cdtypes.h>
#include <cdgraph.h>
#include <cairomm/context.h>
#include <cairomm/region.h>
#include <core/graph_device.h>
#include <gui/gui_features.h>
using namespace Cairo;

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

#define RGBA(r,g,b) (((a)<<24)|((r)<<16)|((g)<<8)|(b))
#define RGB(r,g,b,a) RGBA(r,g,b,0xFF)

class Canvas:public Cairo::Context{
protected:
    friend class GraphDevice;
    friend class WindowManager;
    GraphDevice*dev;
    int mLeft,mTop;//postion in parent/screen context
    int mWidth,mHeight;//
    int mLayer;
    RefPtr<Region>mInvalidRgn;
    RefPtr<Region>mVisibleRgn;
public:
    Canvas(GraphDevice*_dev,HANDLE surface);
    Canvas(GraphDevice*_dev,const RefPtr<Surface>& target);
    Canvas*subContext(int x,int y,int w,int h);
    int blit2Device(HANDLE surface);
    ~Canvas();
    void set_position(int x,int y);
    void set_layer(int l,RefPtr<Region>rgn);
    void draw_text(const RECT&rect,const std::string&text,int text_alignment=DT_LEFT|DT_VCENTER);
    void draw_text(int x,int y,const std::string& text);
    void get_text_size(const std::string&txt,int*w,int*h); 
    void set_color(BYTE r,BYTE g, BYTE b,BYTE a=255);
    void set_color(UINT color);
    void roundrect(int x,int y,int w,int h,int r=0);
    void rectangle(int x,int y,int w,int h);
    void rectangle(const RECT &r);
    void draw_image(const RefPtr<ImageSurface>&img,const RECT&dst,const RECT*src);
    void draw_ninepatch(const RefPtr<ImageSurface>img,const RECT& rect,const std::vector<NinePatchBlock>&horz,
            const std::vector<NinePatchBlock>&vert);
    void rotate(float degrees,float px,float py);
    void dump2png(const char*fname);
    void invalidate(const Rect&r);
    void clip2dirty();//copy clip region to dirty region for compose
};
extern void DumpRegion(const std::string&label,RefPtr<Region>rgn);
}//namspace
#endif

