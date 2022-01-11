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
    uint64_t mFpsStartTime;
    uint64_t mFpsPrevTime;
    uint64_t mFpsNumFrames;
    class Canvas*primaryContext;//
    DWORD compose_event;
    HANDLE primarySurface;
    RefPtr<Region>mInvalidateRgn;
    static GraphDevice*mInst;
    GraphDevice(int format=-1);
    void trackFPS();
public:
   static GraphDevice&getInstance();
   ~GraphDevice();
   void getScreenSize(int &w,int&h);
   int getScreenWidth();
   int getScreenHeight();
   void flip();
   void composeSurfaces();
   bool needCompose();
   Canvas*getPrimaryContext();
   void invalidate(const Rect&);
   HANDLE getPrimarySurface(){return primarySurface;}
};
}
#endif

