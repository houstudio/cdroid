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
#include <sys/time.h>
#include <sys/resource.h>

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
    GFXGetDisplaySize(0,(UINT*)&mScreenWidth,(UINT*)&mScreenHeight);
    const int rotation =WindowManager::getInstance().getDefaultDisplay().getRotation();
    mFormat = fmt<0?GPF_ARGB:fmt;
    GFXCreateSurface(0,&mPrimarySurface,mScreenWidth,mScreenHeight,mFormat,1);
    GFXLockSurface(mPrimarySurface,(void**)&buffer,&pitch);
    LOGD("PrimarySurface=%p size=%dx%d rotation=%d",mPrimarySurface,mScreenWidth,mScreenHeight,rotation*90);
    RefPtr<Surface>surf=ImageSurface::create(buffer,Surface::Format::ARGB32,mScreenWidth,mScreenHeight,pitch);
    mPrimaryContext = new Canvas(surf);

    mRectBanner.set(0,0,400,40);
    RefPtr<Surface>bannerSurf=ImageSurface::create(Surface::Format::ARGB32,400,40);
    mBannerContext = new Canvas(bannerSurf);
    
    mLastComposeTime = SystemClock::uptimeMillis();

    //mInvalidateRgn = Region::create();
    mComposing = 0;
    mQuitFlag  = false;
    mShowFPS = false;

#if COMPOSE_ASYNC
    std::thread t([this](){doCompose();});
    t.detach();
#endif
}

GraphDevice::~GraphDevice(){
    mQuitFlag = true;
    mCV.notify_all();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    delete mPrimaryContext;
    delete mBannerContext;
    GFXDestroySurface(mPrimarySurface);
    //mBannerSufrace is destroied by Canvas,GFXDestroySurface(mBannerSurface);
    LOGD("%p Destroied",this);
}

HANDLE GraphDevice::getPrimarySurface()const{
    return mPrimarySurface;
}

void GraphDevice::invalidate(const Rect&r){
    //mInvalidateRgn->do_union((const RectangleInt&)r);
    LOGV("(%d,%d,%d,%d)",r.left,r.top,r.width,r.height);
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
            const float fps = (float) mFpsNumFrames * 1000 / totalTime;
            mFpsStartTime = nowTime;
            mFpsNumFrames = 0;
            std::string fpsText = std::to_string(fps);
            Cairo::Context::Operator oop = mBannerContext->get_operator();
            mBannerContext->set_operator(Cairo::Context::Operator::CLEAR);
            mBannerContext->set_source_rgb(.02,.02,.02);
            mBannerContext->rectangle(0,0,mRectBanner.width,mRectBanner.height);
            mBannerContext->fill();
            mBannerContext->set_operator(oop);
            mBannerContext->set_source_rgb(1,1,1);
            mBannerContext->set_font_size(22);
	    mBannerContext->move_to(10,8);
	    //mBannerContext->show_text(fpsText);
            mBannerContext->draw_text(mRectBanner,fpsText,Gravity::CENTER);
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
    return mPrimaryContext;
}

void GraphDevice::showFPS(bool value){
    mShowFPS=value;
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
    /*visibleregion is windowbased*/
    for (auto w=windows.begin() ;w!= windows.end();w++){
        const Rect rcw = (*w)->getBound();
        RefPtr<Region>newrgn = Region::create((RectangleInt&)rcw);
        if((*w)->getVisibility()!=View::VISIBLE||(*w)->isAttachedToWindow()==false)continue;

        for(auto w1=w+1;w1!=windows.end();w1++){
            if((*w1)->getVisibility()!=View::VISIBLE)continue;
            Rect r=(*w1)->getBound();
            newrgn->subtract((const RectangleInt&)r);
        }
        newrgn->translate(-rcw.left,-rcw.top);
	(*w)->mVisibleRgn = newrgn;
        regions.push_back(newrgn);
    }
}

void GraphDevice::composeSurfaces(){
    const int rotation = WindowManager::getInstance().getDefaultDisplay().getRotation();
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
    trackFPS();
    computeVisibleRegion(wins,winVisibleRgns);
    mPrimaryContext->set_operator(Cairo::Context::Operator::SOURCE);
    for(int i=0;i<wSurfaces.size();i++){
        Rect rcw = wBounds[i];
        RefPtr<Region> rgn = wins[i]->mVisibleRgn;
        HANDLE hdlSurface  = wSurfaces[i]->mHandle;
        if(rgn->empty())continue; 
        rgn->intersect(wins[i]->mPendingRgn);/*it is already empty*/
        LOGV_IF(!rgn->empty(),"surface[%d] has %d rects to compose",i,rgn->get_num_rectangles());
	DumpRegion("Region",rgn);
        for(int j=0;j<rgn->get_num_rectangles();j++){
            RectangleInt rc = rgn->get_rectangle(j);
            //Rect rcc = {rc.x,rc.y,rc.width,rc.height};
            //rcc.offset(rcw.left,rcw.top);
            //rcc.intersect(0,0,mScreenWidth,mScreenHeight);
            //if(rcc.empty())continue;
            int dx = rcw.left+rc.x;
            int dy = rcw.top+rc.y;
            const int ox = dx,oy=dy;
            RectangleInt rd = rc;
            RectangleInt &rs= rc; 
            switch(rotation){
            case Display::ROTATION_0 :break;
            case Display::ROTATION_90:
                dx = oy;
                dy = mScreenHeight -ox - rs.width;
                rd.width  = rs.height;
                rd.height = rs.width;
                rd.x = rs.y;
                rd.y = rcw.width -rs.x -rs.width;
                break;
            case Display::ROTATION_180:
                dx = mScreenWidth - ox - rs.width;
                dy = mScreenHeight- oy - rs.height;
                rd.x = rcw.width  - rd.x - rs.width;
                rd.y = rcw.height - rd.y - rs.height;
                break;
            case Display::ROTATION_270:
                dy = ox;
                dx = mScreenWidth - oy - rs.height;
                rd.width = rs.height;
                rd.height= rs.width;
                rd.x = rcw.height- rs.y -rs.height;
                rd.y = rs.x;
                break;
            }
            LOGV("blit surface[%d:%d](%d,%d,%d,%d)/(%d,%d,%d,%d) to (%d,%d)/(%d,%d) rotation=%d",i,j,
	         rc.x,rc.y,rc.width,rc.height,rd.x,rd.y,rd.width,rd.height,ox,oy,dx,dy,rotation);
            if(hdlSurface)GFXBlit(mPrimarySurface ,/* rcw.left+rc.x , rcw.top+rc.y*/dx,dy , hdlSurface,(const GFXRect*)&rd);//rc);
            else mPrimaryContext->rectangle(rcw.left+rc.x , rcw.top+rc.y , rc.width , rc.height);
        }
        if(hdlSurface==nullptr){
            mPrimaryContext->set_source(wSurfaces[i]->get_target(),rcw.left,rcw.top);
            mPrimaryContext->fill();
        }
        wins[i]->mPendingRgn->subtract(wins[i]->mPendingRgn);
    }
    if(mShowFPS && mPrimaryContext && mBannerContext){
	mPrimaryContext->set_operator(Cairo::Context::Operator::SOURCE);
	mPrimaryContext->set_source(mBannerContext->get_target(),0,0);
	mPrimaryContext->rectangle(mRectBanner.left,mRectBanner.top,mRectBanner.width,mRectBanner.height);
	mPrimaryContext->fill();
    }
    GFXFlip(mPrimarySurface); 
    mLastComposeTime = SystemClock::uptimeMillis();
    mPendingCompose = 0;
}
}//end namespace
