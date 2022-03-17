#include <canvas.h>
#include <graphdevice.h>
#include <cairo.h>
#include <cdtypes.h>
#include <cdgraph.h>
#include <cdlog.h>
#include <vector>
#include <cairomm/context.h>
#include <cairomm/region.h>
#include <cairomm/fontface.h>
#include <windowmanager.h>
#include <pixman.h>
#include <systemclock.h>
#include <thread>

using namespace Cairo;

namespace cdroid{

GraphDevice*GraphDevice::mInst=nullptr;

GraphDevice&GraphDevice::GraphDevice::getInstance(){
    if(nullptr==mInst)
        mInst=new GraphDevice();
    return *mInst;
}

GraphDevice::GraphDevice(int fmt){
    BYTE*buffer;
    UINT pitch;
    mInst = this;
    mPendingCompose = 0;
    mFpsStartTime = mFpsPrevTime = 0;
    mFpsNumFrames = 0;
    GFXInit();
    GFXGetScreenSize((UINT*)&mScreenWidth,(UINT*)&mScreenHeight);

    mFormat = fmt<0?GPF_ARGB:fmt;
    GFXCreateSurface(&primarySurface,mScreenWidth,mScreenHeight,mFormat,1);
    GFXLockSurface(primarySurface,(void**)&buffer,&pitch);
    mLastComposeTime = SystemClock::uptimeMillis();
    LOGD("primarySurface=%p size=%dx%d",primarySurface,mScreenWidth,mScreenHeight);
    RefPtr<Surface>surf=ImageSurface::create(buffer,Surface::Format::ARGB32,mScreenWidth,mScreenHeight,pitch);
    primaryContext = new Canvas(surf);
    mInvalidateRgn = Region::create();
    mComposing = 0;
    mQuitFlag  = false;
    std::thread t([this](){doCompose();});
    t.detach();
}

GraphDevice::~GraphDevice(){
    mQuitFlag = true;
    mCV.notify_all();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    delete primaryContext;
    GFXDestroySurface(primarySurface);
    primarySurface = nullptr;
    primaryContext = nullptr;
    LOGD("%p Destroied",this);
}

void GraphDevice::invalidate(const Rect&r){
    mInvalidateRgn->do_union((const RectangleInt&)r);
}

void GraphDevice::trackFPS() {
    // Tracks frames per second drawn. First value in a series of draws may be bogus
    // because it down not account for the intervening idle time
    long nowTime = SystemClock::currentTimeMillis();
    if (mFpsStartTime <= 0) {
        mFpsStartTime = mFpsPrevTime = nowTime;
        mFpsNumFrames = 0;
    } else {
        ++mFpsNumFrames;
        long frameTime = nowTime - mFpsPrevTime;
        long totalTime = nowTime - mFpsStartTime;
        mFpsPrevTime = nowTime;
        if (totalTime > 1000) {
            float fps = (float) mFpsNumFrames * 1000 / totalTime;
            LOGV("\tFPS:%f",fps);
            mFpsStartTime = nowTime;
            mFpsNumFrames = 0;
        }
    }
}

void GraphDevice::getScreenSize(int &w,int&h){
    w = mScreenWidth;
    h = mScreenHeight;
}

int GraphDevice::getScreenWidth(){
    return mScreenWidth;
}

int GraphDevice::getScreenHeight(){
    return mScreenHeight;
}

void GraphDevice::flip(){
    mPendingCompose++;
}

bool GraphDevice::needCompose(){
    return mPendingCompose&&(mComposing==0);
}

Canvas*GraphDevice::getPrimaryContext(){
    return primaryContext;
}

void GraphDevice::doCompose(){
    LOGD("%d concurrent threads are supported",std::thread::hardware_concurrency());
    while(!mQuitFlag){
        std::unique_lock<std::mutex>lock(mMutex);
        mComposing = 0;
        mCV.wait(lock);
        mComposing++;
        composeSurfaces();
    }
    LOGD("ComposeThread exit");
}

void GraphDevice::requestCompose(){
    std::unique_lock<std::mutex> lock(mMutex);
    mCV.notify_all();
}

void GraphDevice::lock(){
    mMutex.lock();
}

void GraphDevice::unlock(){
    mMutex.unlock();
}

void GraphDevice::computeVisibleRegion(std::vector<Window*>&windows,std::vector<RefPtr<Region>>&regions){
    for (auto w=windows.begin() ;w!= windows.end();w++){
        Rect rcw=(*w)->getBound();
        RefPtr<Region>newrgn=Region::create((RectangleInt&)rcw);
        if((*w)->getVisibility()!=View::VISIBLE||(*w)->isAttachedToWindow()==false)continue;

        for(auto w1=w+1;w1!=windows.end();w1++){
            if((*w1)->getVisibility()!=View::VISIBLE)continue;
            Rect r=(*w1)->getBound();
            newrgn->subtract((const RectangleInt&)r);
        }
        newrgn->translate(-rcw.left,-rcw.top);
        regions.push_back(newrgn);
    }
}

void GraphDevice::composeSurfaces(){
    int rects=0;
    long t2,t1=SystemClock::uptimeMillis();
    trackFPS();

    std::vector<Rect>wBounds;
    std::vector<Window*>wins;
    std::vector<RefPtr<Canvas>>wSurfaces;
    std::vector<RefPtr<Region>>winVisibleRgns;
    WindowManager::getInstance().enumWindows([&wSurfaces,&wBounds,&wins](Window*w)->bool{
        if( (w->getVisibility()==View::VISIBLE) && w->mAttachInfo && w->mAttachInfo->mCanvas){
            wSurfaces.push_back(w->mAttachInfo->mCanvas);
            wBounds.push_back(w->getBound());
            wins.push_back(w);
            return true;
        }
        return false;
    });
    computeVisibleRegion(wins,winVisibleRgns);
    primaryContext->set_operator(Cairo::Context::Operator::SOURCE);
    for(int i=0;i<wSurfaces.size();i++){
        Rect rcw = wBounds[i];
        RefPtr<Region> rgn = winVisibleRgns[i];
        HANDLE hdlSurface = wSurfaces[i]->mHandle;
        if(rgn->empty())continue; 
        mInvalidateRgn->subtract((const RectangleInt&)rcw);
        rects+=rgn->get_num_rectangles();
        for(int j=0;j<rgn->get_num_rectangles();j++){
            RectangleInt rc=rgn->get_rectangle(j);
            Rect rcc={rc.x,rc.y,rc.width,rc.height};
            rcc.offset(rcw.left,rcw.top);
            rcc.intersect(0,0,mScreenWidth,mScreenHeight);
            if(rcc.empty())continue;

            if(hdlSurface)GFXBlit(primarySurface , rcw.left+rc.x , rcw.top+rc.y , hdlSurface,(const GFXRect*)&rc);
            else primaryContext->rectangle(rcw.left+rc.x , rcw.top+rc.y, rc.width , rc.height);
        }
        if(hdlSurface==nullptr){
            primaryContext->set_source(wSurfaces[i]->get_target(),rcw.left,rcw.top);
            primaryContext->fill();
        } 
    }
    const RectangleInt rectScreen={0,0,mScreenWidth,mScreenHeight};
    mInvalidateRgn->intersect(rectScreen);
    for(int i=0;i<mInvalidateRgn->get_num_rectangles();i++){
        RectangleInt r=mInvalidateRgn->get_rectangle(i);
        GFXFillRect(primarySurface,(const GFXRect*)&r,0);
        LOGV("%d:(%d,%d,%d,%d)",i,r.x,r.y,r.width,r.height);
    }
    mInvalidateRgn->do_xor(mInvalidateRgn);
    GFXFlip(primarySurface); 
    t2=SystemClock::uptimeMillis();
    mLastComposeTime = SystemClock::uptimeMillis();
    mPendingCompose = 0;
}
}//end namespace
