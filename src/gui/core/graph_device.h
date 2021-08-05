#ifndef __GRAPH_DEVICE_H__
#define __GRAPH_DEVICE_H__
#include <core/rect.h>
#include <cairomm/context.h>
#include <map>

using namespace Cairo;
namespace cdroid{
class GraphDevice{
private:
    int width;
    int height;
    int format;
    uint64_t last_compose_time;
    class Canvas*primaryContext;//
    DWORD compose_event;
    HANDLE primarySurface;
    std::vector<class Canvas*>gSurfaces;
    static GraphDevice*mInst;
    GraphDevice(int format=-1);
public:
   static GraphDevice&getInstance();
   ~GraphDevice();
   void getScreenSize(int &w,int&h);
   int getScreenWidth();
   int getScreenHeight();
   void getTextExtens(const std::string&,int fontsize,TextExtents&te); 
   void getFontExtents(int fontsize,FontExtents&fe);
   void flip();
   void ComposeSurfaces();
   bool needCompose();
   Canvas*createContext(int w,int h);
   Canvas*createContext(const RECT&rect);
   Canvas*getPrimaryContext();
   void add(Canvas*ctx);
   void remove(Canvas*ctx);
   HANDLE getPrimarySurface(){return primarySurface;}
};
}
#endif

