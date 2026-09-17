/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <canvas.h>
#include <graphdevice.h>
#include <cairo.h>
#include <porting/cdtypes.h>
#include <porting/cdgraph.h>
#include <porting/cdlog.h>
#include <vector>
#include <cairomm/context.h>
#include <cairomm/region.h>
#include <cairomm/fontface.h>
#include <image-decoders/imagedecoder.h>
#include <windowmanager.h>
#include <widget/cdwindow.h>
#include <systemclock.h>
#include <thread>
#if defined(__linux__)||defined(__unix__)
#include <sys/resource.h>
#endif
#include <malloc.h>
#include <fstream>
#include <mutex>
#include <algorithm>
#include <animation/animator.h>
#include <core/handler.h>
using namespace Cairo;

namespace cdroid{

namespace {
// Bounding box of two screen-space rects (cdroid Rect stores left/top/width/height).
Rect unionRect(const Rect& a, const Rect& b) {
    const int l = std::min(a.left, b.left);
    const int t = std::min(a.top, b.top);
    const int r = std::max(a.left + a.width, b.left + b.width);
    const int bo = std::max(a.top + a.height, b.top + b.height);
    return Rect::MakeLTRB(l, t, r, bo);
}
} // namespace

GraphDevice&GraphDevice::getInstance(){
    static GraphDevice* mInstance = nullptr;
    static std::once_flag flag;
    std::call_once(flag, []() {
        mInstance = new GraphDevice();
    });
    return *mInstance;
}

GraphDevice::GraphDevice(){
    mFormat  = GPF_ARGB;
    mRotation= 0;
    mShowFPS = false;
    LOGD("GraphDevice %p",this);
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
    return *this;
}

int GraphDevice::init(){
    GFXHANDLE logoSurface;
    uint8_t*buffer,*logoBuffer;
    uint32_t pitch;
    mPendingCompose = 0;
    mFpsNumFrames = 0;
    mFpsStartTime = mFpsPrevTime = 0;
    Cairo::RefPtr<Cairo::ImageSurface> img= nullptr;
    if(!mLogo.empty()){
        img = ImageDecoder::loadImage(nullptr,mLogo);
    }
    GFXInit();

    GFXGetDisplaySize(0,(uint32_t*)&mScreenWidth,(uint32_t*)&mScreenHeight);
    GFXCreateSurface(0,&mPrimarySurface,mScreenWidth,mScreenHeight,mFormat,1);
    GFXLockSurface(mPrimarySurface,(void**)&buffer,&pitch);
    LOGI("PrimarySurface=%p size=%dx%d buffer=%p rotation=%d",mPrimarySurface,mScreenWidth,mScreenHeight,buffer,mRotation*90);
    RefPtr<Surface>surf = ImageSurface::create(buffer,Surface::Format::ARGB32,mScreenWidth,mScreenHeight,pitch);
    mPrimaryContext = new Canvas(surf);
    mRectBanner.set(0,0,400,40);

    if(img){
        GFXCreateSurface(0,&logoSurface,mScreenWidth,mScreenHeight,mFormat,0);
        GFXLockSurface(logoSurface,(void**)&logoBuffer,&pitch);
        RefPtr<Surface>logoSurf = ImageSurface::create(logoBuffer,Surface::Format::ARGB32,mScreenWidth,mScreenHeight,pitch);
        RefPtr<Cairo::Context>logoContext=Cairo::Context::create(logoSurf);
        showLogo(logoContext.get(),img);
        GFXBlit(mPrimarySurface,0,0,logoSurface,nullptr);
        GFXDestroySurface(logoSurface);
    }else{
        showLogo(mPrimaryContext,img);
    }

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
#if COMPOSE_ASYNC
    mCV.notify_all();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
#endif
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
    context->save();
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
    context->move_to(rc.width-te.x_advance-32,rc.height+te.y_bearing-8);
    context->show_text(copyRight);
    context->fill();
    context->restore();
}

GFXHANDLE GraphDevice::getPrimarySurface()const{
    return mPrimarySurface;
}

void GraphDevice::trackFPS(Canvas& canvas) {
    // Tracks frames per second drawn. First value in a series of draws may be bogus
    // because it down not account for the intervening idle time
    const int64_t nowTime = SystemClock::currentTimeMillis();
#define FPS_CHECKTIME 1000
    if (mFpsStartTime ==0) {
        mFpsStartTime = mFpsPrevTime = nowTime;
        mFpsNumFrames = 0;
    } else {
        ++mFpsNumFrames;
        const long frameTime = long(nowTime - mFpsPrevTime);
        const long totalTime = long(nowTime - mFpsStartTime);
        mFpsPrevTime = nowTime;
        if (totalTime >=FPS_CHECKTIME) {
            char buffer[64];
            const float fps = (float) mFpsNumFrames * FPS_CHECKTIME / totalTime;
#if HAVE_MALLINFO2
            struct mallinfo2 mi = mallinfo2();
            sprintf(buffer,"%.2ffps,%ldK",fps,long(mi.uordblks>>10));
#elif HAVE_MALLINFO
            struct mallinfo mi = mallinfo();
            sprintf(buffer,"%.2ffps,%ldK",fps,long(mi.uordblks>>10));
#else
            sprintf(buffer,"%.2ffps",fps);
#endif
            mFpsStartTime = nowTime;
            mFpsNumFrames = 0;
       	    mFPSText = buffer;
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
    return mPendingCompose && mComposing;
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
        mCV.wait(lock);
        composeSurfaces();
        mComposing = 0;
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

void GraphDevice::computeVisibleRegion(std::vector<Window*>&windows,std::vector<Cairo::RefPtr<Cairo::Region>>&regions){
    /*visibleregion is windowbased*/
    for (auto w=windows.begin() ;w!= windows.end();w++){
        const Rect rcw = (*w)->getBound();
        RefPtr<Cairo::Region>newrgn = Cairo::Region::create((RectangleInt&)rcw);
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
    const int ox = dx;
    const int oy = dy;
    // Initialize rd from source rect to ensure fields are defined.
    rd = rs;
    switch(rotation){
    case Display::ROTATION_0 :
        // No change
        break;
    case Display::ROTATION_90:
        dx = oy;
        dy = mScreenHeight - ox - rs.width;
        rd.width  = rs.height;
        rd.height = rs.width;
        rd.left = rs.top;
        rd.top = rcw.width - rs.left - rs.width;
        break;
    case Display::ROTATION_180:
        dx = mScreenWidth - ox - rs.width;
        dy = mScreenHeight - oy - rs.height;
        rd.left = rcw.width - rs.left - rs.width;
        rd.top = rcw.height - rs.top - rs.height;
        break;
    case Display::ROTATION_270:
        dy = ox;
        dx = mScreenWidth - oy - rs.height;
        rd.width = rs.height;
        rd.height= rs.width;
        rd.left = rcw.height - rs.top - rs.height;
        rd.top = rs.left;
        break;
    }
}

void GraphDevice::composeSurfaces(){
    const int rotation = WindowManager::getInstance().getDefaultDisplay().getRotation();
    std::vector<Rect> wBounds;
    std::vector<Window*> wins;
    std::vector<Cairo::RefPtr<Canvas>> wSurfaces;
    std::vector<Cairo::RefPtr<Cairo::Region>> winVisibleRgns;
    WindowManager::getInstance().enumWindows([&wSurfaces,&wBounds,&wins](Window*w){
        if( (w->getVisibility()==View::VISIBLE) && w->mAttachInfo && w->mAttachInfo->mCanvas){
            wSurfaces.push_back(w->mAttachInfo->mCanvas);
            wBounds.push_back(w->getBound());
            wins.push_back(w);
            return true;
        }
        return false;
    });
    computeVisibleRegion(wins,winVisibleRgns);
    int commitedRects = 0;
    mPrimaryContext->set_operator(Cairo::Context::Operator::SOURCE);
    for(int i = 0;i < wSurfaces.size();i++){
        Rect rcw = wBounds[i];
        // Compose-time visual translation (the SLIDE activity transition's only output —
        // CDROID's SurfaceControl::setPosition): blit the surface at the translated position
        // while the window's real frame stays at rest, so a11y/input/WMS see the resting
        // geometry mid-animation (AOSP semantics).
        rcw.left += wins[i]->mSurfaceDx;
        rcw.top  += wins[i]->mSurfaceDy;
        GFXHANDLE hdlSurface = wSurfaces[i]->mHandle;
        Cairo::RefPtr<Cairo::Region> rgn = wins[i]->mPendingRgn;
        if(rgn->empty())continue; 
        rgn->intersect(wins[i]->mVisibleRgn);/*it is already empty*/
        LOGV_IF(!rgn->empty(),"surface[%d] has %d rects to compose",i,rgn->get_num_rectangles());
        if (!rgn->empty() && wins[i]->getAlpha() < 1.0f) {
            // Whole-surface fade (window ActivityTransition FADE): the X11-style backends
            // have no per-surface opacity, and the blit path bypasses View-level alpha —
            // apply it HERE, compositing through cairo with paint_with_alpha (OVER, so the
            // windows below show through). NB: ignores display rotation (fades on rotated
            // displays fall back to this un-rotated blit).
            const float walpha = wins[i]->getAlpha();
            mPrimaryContext->save();
            mPrimaryContext->reset_clip();
            for(int j = 0; j < rgn->get_num_rectangles(); j++){
                const RectangleInt rc = rgn->get_rectangle(j);
                mPrimaryContext->rectangle(rcw.left + rc.x, rcw.top + rc.y, rc.width, rc.height);
            }
            mPrimaryContext->clip();
            mPrimaryContext->set_operator(Cairo::Context::Operator::OVER);
            mPrimaryContext->set_source(wSurfaces[i]->get_target(), rcw.left, rcw.top);
            mPrimaryContext->paint_with_alpha(walpha);
            mPrimaryContext->restore();
            commitedRects += rgn->get_num_rectangles();
            rgn->subtract(rgn);
            continue;
        }
#if defined(__x86_64__) ||defined(__x86_64) ||defined(__amd64__)||defined(__amd64)
        for(int j = 0; j < rgn->get_num_rectangles(); j++){
            const RectangleInt rc = rgn->get_rectangle(j);
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
#else
        int dx = rcw.left;
        int dy = rcw.top;
        RectangleInt rs={0,0,rcw.width,rcw.height};
        RectangleInt rd={0,0,rcw.width,rcw.height};
        rotateRectInWindow(rcw,(const Rect&)rs,(Rect&)rd,dx,dy,rotation);
        GFXBlit(mPrimarySurface ,dx,dy,hdlSurface,(const GFXRect*)&rd);
#endif
        commitedRects +=rgn->get_num_rectangles();
        if(mShowFPS && (i == wSurfaces.size()-1) && mPrimaryContext){
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
    // Compositor ghosts: snapshots of REMOVED windows playing their exit
    // animation (AOSP: WMS keeps animating the surface after the view detach).
    // Painted after every window — popups/activity windows sit at the top of
    // their stacks when they exit, so no z-interleaving is needed. NB: like the
    // window fade path above, this blit is un-rotated (rotated-display exit
    // fades/slides fall back to the resting orientation).
    if (mPrimaryContext != nullptr) {
        for (GhostLayer* g : mGhosts) {
            if (!g->snapshot) continue;
            mPrimaryContext->save();
            mPrimaryContext->reset_clip();
            mPrimaryContext->set_operator(Cairo::Context::Operator::OVER);
            mPrimaryContext->set_source(g->snapshot,
                    g->bounds.left + g->dx, g->bounds.top + g->dy);
            if (g->alpha < 1.f) {
                mPrimaryContext->paint_with_alpha(g->alpha);
            } else {
                mPrimaryContext->paint();
            }
            mPrimaryContext->restore();
            commitedRects++;
        }
    }
    if(commitedRects)GFXFlip(mPrimarySurface);
    mLastComposeTime = SystemClock::uptimeMillis();
    mPendingCompose = 0;
}

GraphDevice::GhostLayer* GraphDevice::addGhost(
        const Cairo::RefPtr<Cairo::ImageSurface>& snap, const Rect& bounds) {
    LOGD("ghost add %dx%d at (%d,%d)", bounds.width, bounds.height, bounds.left, bounds.top);
    GhostLayer* g = new GhostLayer();
    g->snapshot = snap;
    g->bounds = bounds;
    g->lastRect = bounds;   // the first compose damages from the resting bounds
    mGhosts.push_back(g);
    return g;
}

void GraphDevice::removeGhost(GhostLayer* g) {
    LOGD("ghost remove alpha=%.2f dx=%d dy=%d", g->alpha, g->dx, g->dy);
    const auto it = std::find(mGhosts.begin(), mGhosts.end(), g);
    if (it == mGhosts.end()) return;  // idempotent: animator cancel() re-fires end
    mGhosts.erase(it);
    // Clear the ghost's last frame: repaint its swept region from the windows
    // below (the frame still sits on the primary), then compose once without it.
    Rect cur = g->bounds;
    cur.offset(g->dx, g->dy);
    WindowManager::getInstance().damageRegion(unionRect(g->lastRect, cur));
    if (g->animator != nullptr) {
        Animator* a = g->animator;
        g->animator = nullptr;
        a->cancel();   // may re-enter removeGhost — the erase above made it a no-op
        // Never free the animator mid-dispatch: this runs FROM its end callback
        // (same discipline as Window::finishClose's posted deletes).
        Handler* h = new Handler();
        h->post([h, a]() { delete a; delete h; });
    }
    delete g;
    composeSurfaces();
}

void GraphDevice::clearGhosts() {
    // Process teardown: no compose, no posts — cancel and free (the looper may
    // already be quitting). cancel() re-fires the end listener -> removeGhost,
    // which no-ops on the already-emptied list.
    std::vector<GhostLayer*> ghosts;
    ghosts.swap(mGhosts);
    for (GhostLayer* g : ghosts) {
        if (g->animator != nullptr) {
            Animator* a = g->animator;
            g->animator = nullptr;
            a->cancel();   // after cancel() returns, the dispatch stack is unwound
            delete a;      // — ~Window's cancel-then-delete discipline
        }
        delete g;
    }
}

void GraphDevice::composeGhosts() {
    if (mGhosts.empty()) return;
    // A ghost frame: repaint the swept region (previous ∪ current placement)
    // from the windows below — the last painted frame still sits on the
    // primary — then compose with the ghosts on top. The exit animator drives
    // this from the Choreographer, independent of any window traversal.
    for (GhostLayer* g : mGhosts) {
        Rect cur = g->bounds;
        cur.offset(g->dx, g->dy);
        WindowManager::getInstance().damageRegion(unionRect(g->lastRect, cur));
        g->lastRect = cur;
    }
    composeSurfaces();
}

}//end namespace
