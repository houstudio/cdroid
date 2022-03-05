#ifndef __GRAPH_DEVICE_H__
#define __GRAPH_DEVICE_H__
#include <core/rect.h>
#include <cairomm/context.h>
#include <mutex>
#include <condition_variable>
#include <map>

using namespace Cairo;
namespace cdroid{
class GraphDevice{
private:
    int width;
    int height;
    int format;
    int mComposing;
    int compose_event;
    uint64_t last_compose_time;
    uint64_t mFpsStartTime;
    uint64_t mFpsPrevTime;
    uint64_t mFpsNumFrames;
    std::mutex mMutex;
    std::condition_variable mCV;
    class Canvas*primaryContext;//
    HANDLE primarySurface;
    RefPtr<Region>mInvalidateRgn;
    static GraphDevice*mInst;
    GraphDevice(int format=-1);
    void trackFPS();
    void doCompose();
public:
    static GraphDevice&getInstance();
    ~GraphDevice();
    void getScreenSize(int &w,int&h);
    int getScreenWidth();
    int getScreenHeight();
    void flip();
    void notify();
    void lock();
    void unlock();
    void composeSurfaces();
    bool needCompose();
    Canvas*getPrimaryContext();
    void invalidate(const Rect&);
    HANDLE getPrimarySurface(){return primarySurface;}
};
}
#endif

