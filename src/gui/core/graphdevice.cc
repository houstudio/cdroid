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
#include <fstream>
using namespace Cairo;

namespace cdroid{

GraphDevice*GraphDevice::mInst = nullptr;

GraphDevice&GraphDevice::GraphDevice::getInstance(){
    if(nullptr==mInst)
        mInst=new GraphDevice();
    return *mInst;
}

GraphDevice::GraphDevice(){
    mFormat = GPF_ARGB;
    mInst = this;
    mRotation=0;
    mShowFPS = false;
}

GraphDevice& GraphDevice::setFormat(int format){
    mFormat = format<0?GPF_ARGB:format;
    return *this;
}

GraphDevice& GraphDevice::setLogo(const std::string&logo){
    mLogo = logo;
    return *this;
}

GraphDevice & GraphDevice::setRotation(int rotation){
    mRotation = rotation;
    LOGD("");
    return *this;
}

int GraphDevice::init(){
    HANDLE logoSurface;
    uint8_t*buffer,*logoBuffer;
    uint32_t pitch;
    mPendingCompose = 0;
    mFpsNumFrames = 0;
    mFpsStartTime = mFpsPrevTime = 0;
    Cairo::RefPtr<Cairo::ImageSurface> img= nullptr;
    if(!mLogo.empty()){
        std::ifstream fs(mLogo.c_str());
	img = Cairo::ImageSurface::create_from_stream(fs);
    }
    GFXInit();

    GFXGetDisplaySize(0,(UINT*)&mScreenWidth,(UINT*)&mScreenHeight);
    GFXCreateSurface(0,&mPrimarySurface,mScreenWidth,mScreenHeight,mFormat,1);
    GFXLockSurface(mPrimarySurface,(void**)&buffer,&pitch);
    LOGI("PrimarySurface=%p size=%dx%d buffer=%p",mPrimarySurface,mScreenWidth,mScreenHeight,buffer);
    RefPtr<Surface>surf = ImageSurface::create(buffer,Surface::Format::ARGB32,mScreenWidth,mScreenHeight,pitch);
    mPrimaryContext = new Canvas(surf);
    mRectBanner.set(0,0,400,40);

    GFXCreateSurface(0,&logoSurface,mScreenWidth,mScreenHeight,mFormat,0);
    GFXLockSurface(logoSurface,(void**)&logoBuffer,&pitch);
    RefPtr<Surface>logoSurf = ImageSurface::create(logoBuffer,Surface::Format::ARGB32,mScreenWidth,mScreenHeight,pitch);
    RefPtr<Cairo::Context>logoContext=Cairo::Context::create(logoSurf);
    showLogo(logoContext.get(),img);
    GFXBlit(mPrimarySurface,0,0,logoSurface,nullptr);
    GFXDestroySurface(logoSurface);

    mLastComposeTime = SystemClock::uptimeMillis();
    mComposing = 0;
    mQuitFlag  = false;

#if COMPOSE_ASYNC
    std::thread t([this](){doCompose();});
    t.detach();
#endif
    return 0;
}

GraphDevice::~GraphDevice(){
    mQuitFlag = true;
    mCV.notify_all();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    delete mPrimaryContext;
    GFXDestroySurface(mPrimarySurface);
    LOGD("%p Destroied",this);
}

void GraphDevice::showLogo(Cairo::Context*context,Cairo::RefPtr<Cairo::ImageSurface> img){
    TextExtents te;
    const std::string copyRight("Powered by cdroid.");
    Cairo::Matrix matrix = Cairo::identity_matrix();
    Rect rc={0,0,mScreenWidth,mScreenHeight};
    switch(mRotation){
    case Display::ROTATION_0:break;
    case Display::ROTATION_90:
       matrix.rotate(-M_PI/2);
       matrix.translate(-mScreenHeight,0);
       context->transform(matrix);
       rc.set(0,0,mScreenHeight,mScreenWidth);
       break;
    case Display::ROTATION_180:
       matrix.translate(mScreenWidth,mScreenHeight);
       matrix.scale(-1,-1);
       context->transform(matrix);
       break;
    case Display::ROTATION_270:
       matrix.translate(mScreenWidth,0);
       matrix.rotate(M_PI/2);
       context->transform(matrix);
       rc.set(0,0,mScreenHeight,mScreenWidth);
       break;
    }
    if(img){
        context->set_source(img,0,0);
        context->paint();
    }
    context->set_font_size(20);
    context->get_text_extents(copyRight,te);
    auto pat= Cairo::LinearGradient::create(rc.width-te.x_advance*1.2,0,rc.width,0);
    pat->add_color_stop_rgb(0, .2, .2, 0); //yellow
    pat->add_color_stop_rgb(0.618, 1.0, 1.0, 1.);//orange
    pat->add_color_stop_rgb(1, .1, .1, .1); 
    context->set_source(pat);
    context->move_to(rc.width-te.x_advance-32,rc.height-10);
    context->show_text(copyRight);
    context->fill();
}

HANDLE GraphDevice::getPrimarySurface()const{
    return mPrimarySurface;
}

void GraphDevice::invalidate(const Rect&r){
    //mInvalidateRgn->do_union((const RectangleInt&)r);
    LOGV("(%d,%d,%d,%d)",r.left,r.top,r.width,r.height);
}

void GraphDevice::trackFPS(Canvas& canvas) {
    // Tracks frames per second drawn. First value in a series of draws may be bogus
    // because it down not account for the intervening idle time
    const long nowTime = SystemClock::currentTimeMillis();
    if (mFpsStartTime <=0) {
        mFpsStartTime = mFpsPrevTime = nowTime;
        mFpsNumFrames = 0;
    } else {
        ++mFpsNumFrames;
        const long frameTime = nowTime - mFpsPrevTime;
        const long totalTime = nowTime - mFpsStartTime;
        mFpsPrevTime = nowTime;
        if (totalTime > 1000) {
            const float fps = (float) mFpsNumFrames * 1000 / totalTime;
            mFpsStartTime = nowTime;
            mFpsNumFrames = 0;
       	    mFPSText = std::to_string(fps);
        }
    }
    canvas.save();
    canvas.set_source_rgb(.02,.02,.02);
    canvas.rectangle(0,0,mRectBanner.width,mRectBanner.height);
    canvas.fill();
    canvas.set_source_rgb(1,1,1);
    canvas.set_font_size(22);
    canvas.move_to(10,8);
    canvas.draw_text(mRectBanner,mFPSText,Gravity::CENTER);
    canvas.restore(); 
}

void GraphDevice::getScreenSize(int &w,int&h)const{
    w = mScreenWidth;
    h = mScreenHeight;
}

int GraphDevice::getScreenWidth()const{
    return mScreenWidth;
}

int GraphDevice::getScreenHeight()const{
    return mScreenHeight;
}

void GraphDevice::flip(){
    mPendingCompose++;
}

bool GraphDevice::needCompose()const{
    return mPendingCompose&&(mComposing==0);
}

Canvas*GraphDevice::getPrimaryContext(){
    return mPrimaryContext;
}

GraphDevice& GraphDevice::showFPS(bool value){
    mShowFPS = value;
    return *this;
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

void GraphDevice::rotateRectInWindow(const Rect&rcw,const Rect&rs,Rect&rd,int&dx,int&dy,int rotation){
    const int ox =dx;
    const int oy= dy;
    switch(rotation){
    case Display::ROTATION_0 : rd = rs;break;
    case Display::ROTATION_90:
        dx = oy;
        dy = mScreenHeight -ox - rs.width;
        rd.width  = rs.height;
        rd.height = rs.width;
        rd.left = rs.top;
        rd.top = rcw.width -rs.left -rs.width;
        break;
    case Display::ROTATION_180:
        dx = mScreenWidth - ox - rs.width;
        dy = mScreenHeight- oy - rs.height;
        rd.left = rcw.width  - rd.left - rs.width;
        rd.top = rcw.height - rd.top - rs.height;
        break;
    case Display::ROTATION_270:
        dy = ox;
        dx = mScreenWidth - oy - rs.height;	
        rd.width = rs.height;
        rd.height= rs.width;
        rd.left = rcw.height- rs.top -rs.height;
        rd.top = rs.left;
        break;
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
    computeVisibleRegion(wins,winVisibleRgns);
    bool fpsBlited = false;
    mPrimaryContext->set_operator(Cairo::Context::Operator::SOURCE);
    for(int i=0;i< wSurfaces.size();i++){
        Rect rcw = wBounds[i];
        RefPtr<Region> rgn = wins[i]->mPendingRgn;
        HANDLE hdlSurface  = wSurfaces[i]->mHandle;
        if(rgn->empty())continue; 
        rgn->intersect(wins[i]->mVisibleRgn);/*it is already empty*/
        LOGV_IF(!rgn->empty(),"surface[%d] has %d rects to compose",i,rgn->get_num_rectangles());
        for(int j=0;j<rgn->get_num_rectangles();j++){
            RectangleInt rc = rgn->get_rectangle(j);
            int dx = rcw.left+ rc.x;
            int dy = rcw.top + rc.y;
            const int ox = dx,oy = dy;
            RectangleInt rd = rc;
            const RectangleInt &rs= rc;
            rotateRectInWindow(rcw,(const Rect&)rs,(Rect&)rd,dx,dy,rotation);
            LOGV("blit surface[%d:%d](%d,%d,%d,%d)/(%d,%d,%d,%d) to (%d,%d)/(%d,%d) rotation=%d",i,j,
	         rc.x,rc.y,rc.width,rc.height,rd.x,rd.y,rd.width,rd.height,ox,oy,dx,dy,rotation);
            if(hdlSurface)GFXBlit(mPrimarySurface , dx , dy , hdlSurface,(const GFXRect*)&rd);
            else mPrimaryContext->rectangle(rcw.left + rc.x , rcw.top + rc.y , rc.width , rc.height);
        }
        if(mShowFPS && (i==wSurfaces.size()-1) && mPrimaryContext){
            Rect recFPS = mRectBanner;
            recFPS.offset(-rcw.left,-rcw.top);
            int dx =0,dy =0;
            wSurfaces[i]->reset_clip();
            mPrimaryContext->reset_clip();
            //trackFPS(*mPrimaryContext);
            trackFPS(*wSurfaces[i]);
            LOGV("FPS=%s fpsrect=(%d,%d,%d,%d)->(%d,%d)",mFPSText.c_str(),recFPS.left,recFPS.top,recFPS.width,recFPS.height,dx,dy);
            rotateRectInWindow(rcw,mRectBanner,recFPS,dx,dy,rotation);
            if( hdlSurface)GFXBlit(mPrimarySurface , dx,dy, hdlSurface,(const GFXRect*)&recFPS);
        }
        if(hdlSurface==nullptr){
            mPrimaryContext->set_source(wSurfaces[i]->get_target(),rcw.left,rcw.top);
            mPrimaryContext->fill();
        }
        rgn->subtract(rgn);
    }/*endif for wSurfaces.size*/
    GFXFlip(mPrimarySurface); 
    mLastComposeTime = SystemClock::uptimeMillis();
    if( (View::mViewCount!=mLastViewCount) && View::VIEW_DEBUG){
        LOGD("Total ViewCount %d",View::mViewCount);
        mLastViewCount = View::mViewCount;
    }
    mPendingCompose = 0;
}
}//end namespace
