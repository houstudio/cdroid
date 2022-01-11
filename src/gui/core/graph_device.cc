#include <canvas.h>
#include <graph_device.h>
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
    mInst=this;
    compose_event=0;
    mFpsStartTime=mFpsPrevTime=0;
    mFpsNumFrames=0;
    GFXInit();
    GFXGetScreenSize((UINT*)&width,(UINT*)&height);

    format=fmt<0?GPF_ARGB:fmt;
    GFXCreateSurface(&primarySurface,width,height,format,1);
    GFXLockSurface(primarySurface,(void**)&buffer,&pitch);
    last_compose_time=SystemClock::uptimeMillis();
    LOGD("primarySurface=%p size=%dx%d",primarySurface,width,height);
    RefPtr<Surface>surf=ImageSurface::create(buffer,Surface::Format::ARGB32,width,height,pitch);
    primaryContext=new Canvas(surf);
    mInvalidateRgn=Region::create();
}

GraphDevice::~GraphDevice(){
    delete primaryContext;
    GFXDestroySurface(primarySurface);
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
    w=width;
    h=height;
}

int GraphDevice::getScreenWidth(){
    return width;
}

int GraphDevice::getScreenHeight(){
    return height;
}

void GraphDevice::flip(){
    compose_event++;
}

bool GraphDevice::needCompose(){
    return compose_event;
}

Canvas*GraphDevice::getPrimaryContext(){
    return primaryContext;
}

void GraphDevice::composeSurfaces(){
    int rects=0;
    long t2,t1=SystemClock::uptimeMillis();
    trackFPS();

    std::vector<RefPtr<Canvas>>wSurfaces;
    std::vector<Rect>wBounds;
    std::vector<Window*>wins;
    WindowManager::getInstance().enumWindows([&wSurfaces,&wBounds,&wins](Window*w)->bool{
        if(w->getVisibility()!=View::VISIBLE||w->mVisibleRgn->empty()==false){
            Rect rcw=w->getBound();
            wSurfaces.push_back(w->mAttachInfo->mCanvas);
            wBounds.push_back(rcw);
            wins.push_back(w);
        }return true;
    });
    primaryContext->set_operator(Cairo::Context::Operator::SOURCE);
    for(int i=0;i<wSurfaces.size();i++){
        Rect rcw=wBounds[i];
        HANDLE hdlSurface=wSurfaces[i]->mHandle;
        std::vector<Rectangle>clipRects;
        primaryContext->set_source(wSurfaces[i]->get_target(),rcw.left,rcw.top);
        RefPtr<Region>rgn=wins[i]->mVisibleRgn;
        wSurfaces[i]->copy_clip_rectangle_list(clipRects);
        rects+=clipRects.size();
        mInvalidateRgn->subtract((const RectangleInt&)rcw);
        if(hdlSurface){
            for(auto r:clipRects){
                Rect rc;
                rc.set(r.x,r.y,r.width,r.height);
                GFXBlit(primarySurface,rcw.left+rc.left,rcw.top+rc.top,hdlSurface,(const GFXRect*)&rc);
            }
            continue;
        }
        for(auto r:clipRects){
            primaryContext->rectangle(rcw.left+r.x,rcw.top+r.y,r.width,r.height);
            LOGV("win %p invalidrect(%d,%d,%d,%d) ",wins[i],(int)r.x,(int)r.y,(int)r.width,(int)r.height);
        }
        primaryContext->fill();
    } 
    for(int i=0;i<mInvalidateRgn->get_num_rectangles();i++){
        RectangleInt r=mInvalidateRgn->get_rectangle(i);
        GFXFillRect(primarySurface,(const GFXRect*)&r,0);
        LOGV("%d:(%d,%d,%d,%d)",i,r.x,r.y,r.width,r.height);
    }
    mInvalidateRgn->do_xor(mInvalidateRgn);
    GFXFlip(primarySurface); 
    t2=SystemClock::uptimeMillis();
    LOGV("%d surfaces %d rects used %d ms",wSurfaces.size(),rects,t2-t1);
    last_compose_time=SystemClock::uptimeMillis();
    compose_event=0;
}
}//end namespace
