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
    int mScreenWidth;
    int mScreenHeight;
    int mFormat;
    int mComposing;
    int mPendingCompose;
    bool mQuitFlag;
    uint64_t mLastComposeTime;
    uint64_t mFpsStartTime;
    uint64_t mFpsPrevTime;
    uint64_t mFpsNumFrames;
    Rect mRectBanner;
    std::mutex mMutex;
    std::condition_variable mCV;
    HANDLE mPrimarySurface;
    HANDLE mBannerSurface;
    class Canvas*mPrimaryContext;
    Canvas *mBannerContext;
    RefPtr<Region>mInvalidateRgn;
    static GraphDevice*mInst;
    GraphDevice(int format=-1);
    void trackFPS();
    void doCompose();
    void computeVisibleRegion(std::vector<class Window*>&windows,std::vector<RefPtr<Region>>&regions);
public:
    static GraphDevice&getInstance();
    ~GraphDevice();
    void getScreenSize(int &w,int&h);
    int getScreenWidth();
    int getScreenHeight();
    void flip();
    void requestCompose();
    void lock();
    void unlock();
    void composeSurfaces();
    bool needCompose();
    Canvas*getPrimaryContext();
    void invalidate(const Rect&);
    HANDLE getPrimarySurface()const;
};
}
#endif

